#include "chatsManager.h"

#include "client.h"

#include <debug.h>
#include <networkTypes.h>
#include <stdio.h>
#include <synchapi.h>
#include <winsock.h>
#include <ws2tcpip.h>

#include "clientUtils.h"



Contact *contacts;
Contact *currentContact = NULL;

static int readInput(FILE *fp, char *buffer, size_t bufferSize);

void initChatHistory(void)
{
    contacts = NULL;
}

bool logIn(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_LEN+1];
    int     *inType;
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

    in = packetFromBytes(request->data);
    if (in.data == NULL) {
        result = false;
        goto clear;
    }
    if (in.type != PACKET_LOGIN_RESPOND) {
        result = false;
        goto clear;
    }

    int *res;
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

    if (contact == NULL) {
        printf("No contact found\n");
        return;
    }

    while (contact != NULL) {
        printf("%s - unread: %d\n", contact->nickname, contact->unread);
        contact = contact->next;
    }
}

void sendMessage(SOCKET socket)
{
    DBG_FUNC();
    PendingRequest  *request;
    char            *nickname = NULL;
    char            message[MAX_MESSAGE_LEN+1];
    int             nicknameLen, messageLen, packetLen,
                    try = 0, packetType = PACKET_MESSAGE;

    if (currentContact == NULL) {
        DBG_ERROR("No contact is selected\n");
        return;
    }
    nickname = currentContact->nickname;

    printf("Enter message: ");
    readInput(stdin, message, MAX_MESSAGE_LEN);

    nicknameLen = strlen(nickname) + 1;
    messageLen = strlen(message) + 1;

    packetLen = PACKET_HEADER_SIZE + sizeof(nicknameLen) + nicknameLen + sizeof(messageLen) + messageLen;

    tryAgain:
    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        return;
    }

    WaitForSingleObject(socketServerMutex, INFINITE);
    send(socket, (char*)&packetLen, sizeof(packetLen), 0);
    send(socket, (char*)&packetType, sizeof(packetType), 0);
    send(socket, (char*)&request->id, sizeof(request->id), 0);
    send(socket, (char*)&nicknameLen, sizeof(nicknameLen), 0);
    send(socket, nickname, nicknameLen, 0);
    send(socket, (char*)&messageLen, sizeof(messageLen), 0);
    send(socket, message, messageLen, 0);
    ReleaseMutex(socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    if (request->size < PACKET_HEADER_SIZE && try++ < MAX_RETRIES) {
        DBG_WARNING("Respond too short, trying again (%d)\n", try);
        deleteRequest(request);
        goto tryAgain;
    }
    if (request->size < PACKET_HEADER_SIZE) {
        DBG_ERROR("Respond too short\n");
        deleteRequest(request);
        return;
    }

    switch (*(int*)(request->data + PACKET_TYPE_OFFSET)) {
    case PACKET_MESSAGE_SUCCESS:
        DBG_INFO("Message sent\n");
        break;
    case PACKET_MESSAGE_FAILURE:
        DBG_ERROR("Message failed\n");
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
    _sleep(1500);
}

void createChat(SOCKET socket)
{
    DBG_FUNC();
    PendingRequest  *request;
    int             packetType = PACKET_CREATE_CHAT,
                    packetLen = 0, nicknameLen,
                    try = 0;
    char            nickname[NICKNAME_LEN+1];

    printf("Enter nickname or q to quit: ");
    readInput(stdin, nickname, NICKNAME_LEN);

    if (strcmp(nickname, "q") == 0)
        return;
    nicknameLen = strlen(nickname) + 1;

    tryAgain:
    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        return;
    }

    packetLen = PACKET_HEADER_SIZE + sizeof(nicknameLen) + nicknameLen;
    WaitForSingleObject(socketServerMutex, INFINITE);
    send(socket, (char*)&packetLen, sizeof(packetLen), 0);
    send(socket, (char*)&packetType, sizeof(packetType), 0);
    send(socket, (char*)&request->id, sizeof(request->id), 0);
    send(socket, (char*)&nicknameLen, sizeof(nicknameLen), 0);
    send(socket, nickname, nicknameLen, 0);
    ReleaseMutex(socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    if (request->size < PACKET_HEADER_SIZE && try++ < MAX_RETRIES) {
        DBG_WARNING("Respond too short, trying again (%d)\n", try);
        deleteRequest(request);
        goto tryAgain;
    }
    if (request->size < PACKET_HEADER_SIZE) {
        DBG_ERROR("Respond too short\n");
        deleteRequest(request);
        return;
    }
    switch (*(int*)(request->data + PACKET_TYPE_OFFSET)) {
    case PACKET_MESSAGE_SUCCESS:
        DBG_INFO("Message sent\n");
        break;
    case PACKET_MESSAGE_FAILURE:
        DBG_ERROR("Message failed\n");
        break;
    case PACKET_MESSAGE_CLIENT_NOT_FOUND:
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
    deleteRequest(request);
    _sleep(1500);
}

void deleteChat(const Contact *contact)
{
    DBG_FUNC();
}

void updateUnreadMessages(void)
{
    DBG_FUNC();
}

static int readInput(FILE *fp, char *buffer, size_t bufferSize)
{
    char format[16];
    snprintf(format, sizeof(format), "%%%zus", bufferSize);
    fseek(fp, 0, SEEK_END);
    return fscanf(fp, format, buffer);
}
