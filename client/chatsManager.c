#include "chatsManager.h"

#include <debug.h>
#include <networkTypes.h>
#include <stdio.h>
#include <synchapi.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "clientUtils.h"
#include "contactsManager.h"

Contact *contacts = NULL;
Contact *currentContact = NULL;

static int  readInput(FILE *fp, char *buffer, size_t bufferSize);

bool logIn(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_LEN+1];
    int     *res;
    size_t  readPos = 0;
    bool    result = false;
    Packet  out, in;

    PendingRequest * request;

    printf("Enter nickname or q to quit: ");
    readInput(stdin, nickname, NICKNAME_LEN);
    if (strcmp(nickname, "q") == 0)
        return false;

    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        return false;
    }

    out = createPacket(PACKET_LOGIN_REQUEST, request->id);
    addPacketString(&out, nickname);
    sendPacket(socket, out, socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    in = packetFromBytes((char*)request->data);
    if (in.data == NULL) {
        result = false;
        goto clear;
    }
    if (in.type != PACKET_LOGIN_RESPOND) {
        result = false;
        goto clear;
    }

    if ((res = readPacketInt(&in, &readPos)) == NULL) {
        result = false;
        goto clear;
    }
    switch (*res) {
    case PACKET_LOGIN_SUCCESS:
        DBG_INFO("Login success\n");
        result = true;
        break;
    case PACKET_LOGIN_FAILURE:
        DBG_ERROR("Login failed\n");
        break;
    case PACKET_LOGIN_ALREADY_EXISTS:
        DBG_INFO("User with this username already exists\n");
        break;
    case PACKET_ERROR_CANT_PROCESS: case PACKET_ITERNAL_SERVER_ERROR:
        DBG_ERROR("Server error\n");
        break;
    default:
        DBG_ERROR("Unknown respond (%d)\n", *(int*)(request->data + PACKET_TYPE_OFFSET));
    }

    clear:
    if (in.data != NULL)
        deletePacket(in);
    if (out.data != NULL)
        deletePacket(out);
    deleteRequest(request);
    return result;
}

void showPrivateChats(void)
{
    DBG_FUNC();
    Contact *contact = contacts;
    int max = 0, selected, i;

    while (true) {
        system("cls");

        if (contact == NULL) {
            printf("No contact found\n");
            return;
        }

        while (contact != NULL) {
            max++;
            printf("%d: %s - unread: %d\n", max, contact->nickname, contact->unread);
            contact = contact->next;
        }
        printf("Select chat: ");
        fseek(stdin, 0, SEEK_END);
        scanf("%d", &selected);

        if (selected > max || selected < 1) {
            printf("No such option\n");
            Sleep(1000);
            continue;
        }

        for (contact = contacts, i = 0; i < selected; i++)
            contact = contact->next;
        openChat(contact);
    }
}

void openChat(Contact *contact)
{
    DBG_FUNC();

    Message *it = contact->chatHistory.head;

    while (it != NULL) {
        printf("%s: %s\n", it->sender == true ? "You" : contact->nickname, it->message);
    }

    // Process input and send messages
    while (true) {

    }
}

void sendMessage(const SOCKET socket, const Contact *contact, const char *message)
{
    DBG_FUNC();
    PendingRequest  *request = NULL;
    int             try = 0, *respond = NULL;
    size_t          readPos = 0;
    Packet          pIn, pOut;

    if (contact == NULL) {
        DBG_ERROR("No contact is selected\n");
        return;
    }

    pIn.data = NULL; pOut.data = NULL;

    for (int i = 0; i < 3; i++) {
        deleteRequest(request);
        deletePacket(pIn); deletePacket(pOut);
        request = createRequest();
        if (request == NULL) {
            DBG_FATAL("Failed to create request\n");
            return;
        }

        pOut = createPacket(PACKET_MESSAGE_REQUEST, request->id);
        addPacketString(&pOut, contact->nickname);
        addPacketString(&pOut, message);
        sendPacket(socket, pOut, &socketServerMutex);

        WaitForSingleObject(request->event, INFINITE);
        WaitForSingleObject(request->mutex, INFINITE);

        pIn = packetFromBytes((char*)request->data);
        deleteRequest(request);
        if (pIn.data == NULL) {
            DBG_ERROR("Can't create packet from respond\n");
            continue;
        }

        if ((respond = readPacketInt(&pIn, &readPos)) != NULL) {
            break;
        }
    }

    if (respond != NULL)
    switch (*respond) {
    case PACKET_MESSAGE_SUCCESS:
        DBG_INFO("Message sent\n");
        break;
    case PACKET_MESSAGE_FAILURE:
        DBG_ERROR("Message sending failed\n");
        break;
    case PACKET_MESSAGE_CLIENT_NOT_FOUND:
        DBG_INFO("Client not found\n");
        break;
    case PACKET_ITERNAL_SERVER_ERROR:
        DBG_WARNING("Internal Server Error\n");
        break;
    default:
        DBG_WARNING("Unknown respond\n");
    }
    deleteRequest(request);
    deletePacket(pIn); deletePacket(pOut);
    _sleep(1500);
}

void createChat(SOCKET socket)
{
    DBG_FUNC();
    PendingRequest  *request;
    int             *respond;
    size_t          readPos = 0;
    char            nickname[NICKNAME_LEN+1];
    Packet          in, out;

    printf("Enter nickname or q to quit: ");
    readInput(stdin, nickname, NICKNAME_LEN);

    if (strcmp(nickname, "q") == 0)
        return;

    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        return;
    }
    out = createPacket(PACKET_CREATE_CHAT_REQUEST, request->id);
    addPacketString(&out, nickname);
    sendPacket(socket, out, &socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    in = packetFromBytes(request->data);
    if (request->data == NULL) {
        DBG_ERROR("Can't create packet from request\n");
        deleteRequest(request);
        deletePacket(out);
        return;
    }
    if ((respond = readPacketInt(&in, &readPos)) == NULL) {
        DBG_DEBUG("Can't read respond from packet\n");
        deleteRequest(request);
        deletePacket(out);
        deletePacket(in);
        return;
    }

    bool success = false;
    switch (*respond) {
    case PACKET_CREATE_CHAT_SUCCESS:
        DBG_INFO("Chat created\n");
        success = true;
        break;
    case PACKET_CREATE_CHAT_FAILURE:
        DBG_ERROR("Failed\n");
        break;
    case PACKET_CREATE_CHAT_CLIENT_NOT_FOUND:
        DBG_INFO("Client not found\n");
        break;
    case PACKET_ITERNAL_SERVER_ERROR:
        DBG_ERROR("Internal Server Error\n");
        break;
    case PACKET_CREATE_CHAT_ITS_YOUR_NICK:
        DBG_INFO("Thats your nickname\n");
        break;
    default:
        DBG_WARNING("Unknown respond\n");
    }

    if (success == true) {
        createContact(nickname);
    }

    deleteRequest(request);
    _sleep(1500);
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

    while (contact != NULL)
        unread += contact->unread;

    return unread;
}

static int readInput(FILE *fp, char *buffer, size_t bufferSize)
{
    char format[16];
    snprintf(format, sizeof(format), "%%%zus", bufferSize);
    fseek(fp, 0, SEEK_END);
    return fscanf(fp, format, buffer);
}
