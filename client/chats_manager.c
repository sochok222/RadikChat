#include "chats_manager.h"

#include "client.h"

#include <conio.h>
#include <debug.h>
#include <request.h>
#include <stdio.h>
#include <synchapi.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "client_utils.h"
#include "console_input.h"
#include "console_output.h"
#include "contacts_manager.h"
#include "packet.h"

#include <process.h>
#include <processthreadsapi.h>
#include <time.h>

Contact *contacts = NULL;
Contact *current_contact = NULL;

static DWORD WINAPI chat_update_thread(void*);
typedef struct sChatUpdateThreadArg
{
    ChatHistory *chat_history;
    HANDLE start_from_mutex;
    int start_from;
} ChatUpdateThreadArg;

bool log_in(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_LEN+1];
    bool    result = false;

    Request *request;

    print_request("Enter nickname or q to quit:");
    read_in_buffer(nickname, NICKNAME_LEN);
    clear_request();
    if (strcmp(nickname, "q") == 0)
        return false;

    request = create_request();
    if (request == NULL) {
        DBG_FATAL("Failed to create request");
        return false;
    }

    PacketLogin *packet_login = NULL;
    create_login_packet(&packet_login);

    login_set_nickname(packet_login, nickname);
    associate_request(packet_login->tl_packet, request);
    tl_pack_login(packet_login);
    int total_sent = 0;
    while (total_sent < packet_login->tl_packet->size) {
        int sent = send(socket, packet_login->tl_packet->data + total_sent, packet_login->tl_packet->size - total_sent, 0);
        if (sent <= 0) {
            DBG_FATAL("Failed to send login packet");
            exit(1);
        }
        total_sent += sent;
    }
    // send_packet(socket, out, &socket_server_mutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);
    TLPacket *tl_respond = request->packet;

    PacketParseStatus parse_status;
    PacketServerRespond *server_respond = NULL;
    if ((parse_status = tl_unpack_server_respond(tl_respond, &server_respond)) != PKT_PARSE_OK) {
        DBG_ERROR("Failed to parse packet %s", get_parse_status_string(parse_status));
        // TODO handle error
        exit(1);
    }

    switch (server_respond->status) {
    case SERV_RESPOND_OK:
        result = true;
        break;
    default:
        DBG_ERROR("Server respond not ok");
        // print_status_error_message(in.status);
    }

    delete_packet_server_respond(server_respond);
    delete_tl_packet(tl_respond);
    return result;
}

void show_private_chats(void)
{
    DBG_FUNC();
    Contact *contact = contacts;
    static char input[128];
    int max, selected, i, ch, start = 0;

    while (true) {
        if (contact == NULL) {
            print_notification(formatDefault, "No contacts found");
            return;
        }

        print_request("press u to go up, d to go down and i to enter insert mode and q to quit");
        while (true) {
            print_contacts(contact, start);
            ch = getch();
            if (ch == 'd') {
                if (app_data.contact_count - start > get_main_area_height())
                    start++;
                else
                    print_notification(formatDefault, "Already at top");
                print_contacts(contact, start);
            } else if (ch == 'u') {
                if (start > 0)
                    start--;
                else
                    print_notification(formatDefault, "Already at bottom");
                print_contacts(contact, start);
            } else if (ch == 'i') {
                break;
            } else if (ch == 'q') {
                clear_screen();
                return;
            }
        }

        print_request("Select chat: ");
        read_in_buffer(input, 128);
        selected = atoi(input);

        if (selected > app_data.contact_count || selected < 0) {
            print_notification(formatDefault, "No such option");
            continue;
        }

        for (contact = contacts, i = 0; i < selected; i++)
            contact = contact->next;
        clear_request();
        open_chat(contact);
    }
}

void open_chat(const Contact *contact)
{
    DBG_FUNC();
    char input_buffer[100];
    char ch;
    HANDLE chat_update_thread_handle;
    ChatUpdateThreadArg chat_update_arg;

    print_contact_name("Chat with %s:", contact->nickname);
    print_chat_history(contact->chat_history, 0);

    chat_update_arg.chat_history = &contact->chat_history;
    chat_update_arg.start_from_mutex = CreateMutex(NULL, FALSE, NULL);
    chat_update_arg.start_from = 0;

    chat_update_thread_handle = CreateThread(NULL, 0, chat_update_thread, &chat_update_arg, 0, 0);

    // Process input and send messages
    while (true) {
        print_request("Type your message, /quit to quit and /scroll to enter scroll mode");
        read_in_buffer(input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0)
            continue;
        if (strcmp(input_buffer, "/quit") == 0) {
            TerminateThread(chat_update_thread_handle, 0);
            clear_screen();
            return;
        }
        if (strcmp(input_buffer, "/scroll") == 0) {
            while (true) {
                print_request("press u to go up, d to go down, i to write message");
                ch = read_char(false);
                if (ch == 'u') {
                    if (contact->chat_history.messages - chat_update_arg.start_from <= get_main_area_height()) {
                        print_notification(formatDefault, "Already at top");
                    } else {
                        WaitForSingleObject(chat_update_arg.start_from_mutex, INFINITE);
                        chat_update_arg.start_from++;
                        ReleaseMutex(chat_update_arg.start_from_mutex);
                        SetEvent(app_data.message_event);
                    }
                } else if (ch == 'd') {
                    if (chat_update_arg.start_from <= 0) {
                        chat_update_arg.start_from = 0;
                        print_notification(formatDefault, "Already at bottom");
                    } else {
                        WaitForSingleObject(chat_update_arg.start_from_mutex, INFINITE);
                        chat_update_arg.start_from--;
                        ReleaseMutex(chat_update_arg.start_from_mutex);
                        SetEvent(app_data.message_event);
                    }
                } else if (ch == 'i') {
                    break;
                }
            }
            input_buffer[0] = '\0';
            continue;
        }
        send_message(socket_server, contact, input_buffer);
    }
}

void send_message(SOCKET server_socket, const Contact *contact, const char *message)
{
    DBG_FUNC();
    SendMessageThreadArg *arg = malloc(sizeof(*arg)); // Freed in send_message_thread
    Message *newMessage = add_message(contact, message, true, MESSAGE_SEND_PENDING);
    SetEvent(app_data.message_event);

    arg->message = newMessage;
    arg->contact = contact;
    arg->socket = server_socket;

    _beginthread(send_message_thread, 0, arg);
}

void create_chat(SOCKET socket)
{
    // DBG_FUNC();
    // Request  *request;
    // char            nickname[NICKNAME_LEN+1];
    // TLPacket          in, out;
    //
    // print_request("Enter nickname or q to quit: ");
    // read_in_buffer(nickname, NICKNAME_LEN);
    // clear_request();
    //
    // if (strcmp(nickname, "q") == 0)
    //     return;
    //
    // if (find_contact(nickname) != NULL) {
    //     DBG_INFO("Chat already created");
    //     print_notification(formatDefault, "Chat already created");
    //     return;
    // }
    //
    // request = create_request();
    // if (request == NULL) {
    //     DBG_FATAL("Failed to create request");
    //     print_error("Can't create chat");
    //     return;
    // }
    // out = createPacket(TYPE_REQUEST, CMD_CREATE_CHAT, 0, request->id);
    // addPacketString(&out, nickname);
    // send_packet(socket, out, &socket_server_mutex);
    //
    // WaitForSingleObject(request->event, INFINITE);
    // WaitForSingleObject(request->mutex, INFINITE);
    //
    // in = packet_from_bytes(request->data);
    //
    // switch (in.status) {
    // case SERV_RESPOND_OK:
    //     DBG_INFO("Chat created");
    //     printSuccess("Chat created");
    //     create_contact(nickname);
    //     break;
    // default:
    //     print_status_error_message(in.status);
    // }
    //
    // delete_request(&request);
}

void delete_chat(const Contact *contact)
{
    DBG_FUNC();
}

int update_unread_messages(void)
{
    DBG_FUNC();
    int unread = 0;
    Contact *contact = contacts;

    while (contact != NULL) {
        unread += contact->unread;
        contact = contact->next;
    }

    return unread;
}

static DWORD WINAPI chat_update_thread(void *arg)
{
    DBG_FUNC();
    if (arg == NULL) {
        DBG_ERROR("No contact is selected\n");
        return 0;
    }
    ChatHistory *chat_history = ((ChatUpdateThreadArg*)arg)->chat_history;
    int start_from;

    static DWORD wait_res;
    while (true) {
        wait_res = WaitForSingleObject(app_data.message_event, INFINITE);
        start_from = ((ChatUpdateThreadArg*)arg)->start_from;
        if (wait_res == WAIT_OBJECT_0) {
            print_chat_history(*chat_history, start_from);
        }
    }
}