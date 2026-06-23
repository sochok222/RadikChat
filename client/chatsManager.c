#include "chatsManager.h"

#include "client.h"

#include <conio.h>
#include <debug.h>
#include <request.h>
#include <stdio.h>
#include <synchapi.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "clientUtils.h"
#include "consoleInput.h"
#include "consoleOutput.h"
#include "contactsManager.h"
#include "packet.h"

#include <process.h>
#include <processthreadsapi.h>
#include <time.h>

Contact *contacts = NULL;
Contact *currentContact = NULL;

static DWORD WINAPI chatUpdateThread(void*);
typedef struct sChatUpdateThreadArg
{
    ChatHistory *chatHistory;
    HANDLE startFromMutex;
    int startFrom;
} ChatUpdateThreadArg;

bool logIn(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_LEN+1];
    bool    result = false;

    Request *request;

    printRequest("Enter nickname or q to quit:");
    readInBuffer(nickname, NICKNAME_LEN);
    clearRequest();
    if (strcmp(nickname, "q") == 0)
        return false;

    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request");
        return false;
    }

    PacketLogin *packetLogin = NULL;
    createLoginPacket(&packetLogin);

    loginSetNickname(packetLogin, nickname);
    associateRequest(packetLogin->tlPacket, request);
    tlPackLogin(packetLogin);
    int totalSent = 0;
    while (totalSent < packetLogin->tlPacket->size) {
        int sent = send(socket, packetLogin->tlPacket->data + totalSent, packetLogin->tlPacket->size - totalSent, 0);
        if (sent <= 0) {
            DBG_FATAL("Failed to send login packet");
            exit(1);
        }
        totalSent += sent;
    }
    // sendPacket(socket, out, &socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);
    TLPacket *tlRespond = request->packet;

    PacketParseStatus parseStatus;
    PacketServerRespond *serverRespond = NULL;
    if ((parseStatus = tlUnpackServerRespond(tlRespond, &serverRespond)) != PKT_PARSE_OK) {
        DBG_ERROR("Failed to parse packet %s", getParseStatusString(parseStatus));
        // TODO handle error
        exit(1);
    }

    switch (serverRespond->status) {
    case SERV_RESPOND_OK:
        result = true;
        break;
    default:
        DBG_ERROR("Server respond not ok");
        // printStatusErrorMessage(in.status);
    }

    deletePacketServerRespond(serverRespond);
    deleteTLPacket(tlRespond);
    return result;
}

void showPrivateChats(void)
{
    DBG_FUNC();
    Contact *contact = contacts;
    static char input[128];
    int max, selected, i, ch, start = 0;

    while (true) {
        if (contact == NULL) {
            printNotification(formatDefault, "No contacts found");
            return;
        }

        printRequest("press u to go up, d to go down and i to enter insert mode and q to quit");
        while (true) {
            printContacts(contact, start);
            ch = getch();
            if (ch == 'd') {
                if (appData.contactCount - start > getMainAreaHeight())
                    start++;
                else
                    printNotification(formatDefault, "Already at top");
                printContacts(contact, start);
            } else if (ch == 'u') {
                if (start > 0)
                    start--;
                else
                    printNotification(formatDefault, "Already at bottom");
                printContacts(contact, start);
            } else if (ch == 'i') {
                break;
            } else if (ch == 'q') {
                clearScreen();
                return;
            }
        }

        printRequest("Select chat: ");
        readInBuffer(input, 128);
        selected = atoi(input);

        if (selected > appData.contactCount || selected < 0) {
            printNotification(formatDefault, "No such option");
            continue;
        }

        for (contact = contacts, i = 0; i < selected; i++)
            contact = contact->next;
        clearRequest();
        openChat(contact);
    }
}

void openChat(const Contact *contact)
{
    DBG_FUNC();
    char inputBuffer[100];
    char ch;
    HANDLE chatUpdateThreadHandle;
    ChatUpdateThreadArg chatUpdateArg;

    printContactName("Chat with %s:", contact->nickname);
    printChatHistory(contact->chatHistory, 0);

    chatUpdateArg.chatHistory = &contact->chatHistory;
    chatUpdateArg.startFromMutex = CreateMutex(NULL, FALSE, NULL);
    chatUpdateArg.startFrom = 0;

    chatUpdateThreadHandle = CreateThread(NULL, 0, chatUpdateThread, &chatUpdateArg, 0, 0);

    // Process input and send messages
    while (true) {
        printRequest("Type your message, /quit to quit and /scroll to enter scroll mode");
        readInBuffer(inputBuffer, sizeof(inputBuffer));
        if (strlen(inputBuffer) == 0)
            continue;
        if (strcmp(inputBuffer, "/quit") == 0) {
            TerminateThread(chatUpdateThreadHandle, 0);
            clearScreen();
            return;
        }
        if (strcmp(inputBuffer, "/scroll") == 0) {
            while (true) {
                printRequest("press u to go up, d to go down, i to write message");
                ch = readChar(false);
                if (ch == 'u') {
                    if (contact->chatHistory.messages - chatUpdateArg.startFrom <= getMainAreaHeight()) {
                        printNotification(formatDefault, "Already at top");
                    } else {
                        WaitForSingleObject(chatUpdateArg.startFromMutex, INFINITE);
                        chatUpdateArg.startFrom++;
                        ReleaseMutex(chatUpdateArg.startFromMutex);
                        SetEvent(appData.messageEvent);
                    }
                } else if (ch == 'd') {
                    if (chatUpdateArg.startFrom <= 0) {
                        chatUpdateArg.startFrom = 0;
                        printNotification(formatDefault, "Already at bottom");
                    } else {
                        WaitForSingleObject(chatUpdateArg.startFromMutex, INFINITE);
                        chatUpdateArg.startFrom--;
                        ReleaseMutex(chatUpdateArg.startFromMutex);
                        SetEvent(appData.messageEvent);
                    }
                } else if (ch == 'i') {
                    break;
                }
            }
            inputBuffer[0] = '\0';
            continue;
        }
        sendMessage(socketServer, contact, inputBuffer);
    }
}

void sendMessage(SOCKET serverSocket, const Contact *contact, const char *message)
{
    DBG_FUNC();
    SendMessageThreadArg *arg = malloc(sizeof(*arg)); // Freed in sendMessageThread
    Message *newMessage = addMessage(contact, message, true, MESSAGE_SEND_PENDING);
    SetEvent(appData.messageEvent);

    arg->message = newMessage;
    arg->contact = contact;
    arg->socket = serverSocket;

    _beginthread(sendMessageThread, 0, arg);
}

void createChat(SOCKET socket)
{
    // DBG_FUNC();
    // Request  *request;
    // char            nickname[NICKNAME_LEN+1];
    // TLPacket          in, out;
    //
    // printRequest("Enter nickname or q to quit: ");
    // readInBuffer(nickname, NICKNAME_LEN);
    // clearRequest();
    //
    // if (strcmp(nickname, "q") == 0)
    //     return;
    //
    // if (findContact(nickname) != NULL) {
    //     DBG_INFO("Chat already created");
    //     printNotification(formatDefault, "Chat already created");
    //     return;
    // }
    //
    // request = createRequest();
    // if (request == NULL) {
    //     DBG_FATAL("Failed to create request");
    //     printError("Can't create chat");
    //     return;
    // }
    // out = createPacket(TYPE_REQUEST, CMD_CREATE_CHAT, 0, request->id);
    // addPacketString(&out, nickname);
    // sendPacket(socket, out, &socketServerMutex);
    //
    // WaitForSingleObject(request->event, INFINITE);
    // WaitForSingleObject(request->mutex, INFINITE);
    //
    // in = packetFromBytes(request->data);
    //
    // switch (in.status) {
    // case SERV_RESPOND_OK:
    //     DBG_INFO("Chat created");
    //     printSuccess("Chat created");
    //     createContact(nickname);
    //     break;
    // default:
    //     printStatusErrorMessage(in.status);
    // }
    //
    // deleteRequest(&request);
}

void deleteChat(const Contact *contact)
{
    DBG_FUNC();
}

int updateUnreadMessages(void)
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

static DWORD WINAPI chatUpdateThread(void *arg)
{
    DBG_FUNC();
    if (arg == NULL) {
        DBG_ERROR("No contact is selected\n");
        return 0;
    }
    ChatHistory *chatHistory = ((ChatUpdateThreadArg*)arg)->chatHistory;
    int startFrom;

    static DWORD waitRes;
    while (true) {
        waitRes = WaitForSingleObject(appData.messageEvent, INFINITE);
        startFrom = ((ChatUpdateThreadArg*)arg)->startFrom;
        if (waitRes == WAIT_OBJECT_0) {
            printChatHistory(*chatHistory, startFrom);
        }
    }
}