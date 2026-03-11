#include "chatsManager.h"

#include "client.h"

#include <debug.h>
#include <networkTypes.h>
#include <stdio.h>
#include <synchapi.h>
#include <winsock.h>
#include <ws2tcpip.h>

#include "clientUtils.h"

#define NICKNAME_SIZE 20

Contact *contacts;

void initChatHistory(void)
{
    contacts = NULL;
}

bool signIn(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_SIZE+1];
    int     nicknameLen, type = PACKET_LOGIN, i, packetSize = 0;
    bool    result = false;

    PendingRequest * request = nullptr;

    printf("Enter nickname or q to quit: ");
    scanf("%20s", nickname);
    if (strcmp(nickname, "q") == 0)
        return false;
    nicknameLen = strlen(nickname) + 1;

    for (i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (pendingRequests[i] == NULL) {
            request = (pendingRequests[i] = malloc(sizeof(PendingRequest)));
            if (request == NULL) {
                DBG_FATAL("malloc failed");
                return false;
            }
            request->buffer = malloc(MAX_PENDING_REQUESTS_BUFFER_SIZE);
            if (request->buffer == NULL) {
                DBG_FATAL("malloc failed");
                free(request);
                return false;
            }
            request->requestId = i;
            break;
        }
    }

    packetSize = PACKET_SIZE_SIZE + PACKET_TYPE_SIZE + PACKET_ID_SIZE + nicknameLen;

    WaitForSingleObject(socketServerMutex, INFINITE);
    send(socket, (char*)&packetSize, sizeof(packetSize), 0);
    send(socket, (char*)&type, sizeof(type), 0);
    send(socket, (char*)&request->requestId, sizeof(request->requestId), 0);
    send(socket, (char*)&nicknameLen, sizeof(nicknameLen), 0);
    send(socket, nickname, nicknameLen, 0);
    ReleaseMutex(socketServerMutex);

    /* TODO move pending request initialization to function and add handling for
     * event or mutex creation failure */
    request->bufferSize = 0;
    request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
    request->mutex = CreateMutex(NULL, FALSE, NULL);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    if (request->bufferSize != 16) {
        DBG_WARNING("Expected different packet size\n");
    }

    if (*(int*)(request->buffer + PACKET_PAYLOAD_OFFSET) == PACKET_LOGIN_SUCCESS) {
        DBG_INFO("Login success\n");
        result =  true;
        goto clearRequest;
    }
    DBG_INFO("Login failed\n");
    result = false;

    clearRequest:
    free(request);
    request = NULL;
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
    }
}

void createChat(SOCKET socket)
{

}

void deleteChat(const Contact *contact)
{
    DBG_FUNC();
}

void updateUnreadMessages(void)
{
    DBG_FUNC();
}