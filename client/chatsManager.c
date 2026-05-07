#include "chatsManager.h"

#include "client.h"

#include <conio.h>
#include <debug.h>
#include <pendingOperation/request.h>
#include <stdio.h>
#include <synchapi.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "clientUtils.h"
#include "consoleInput.h"
#include "consoleOutput.h"
#include "contactsManager.h"

#include <process.h>
#include <processthreadsapi.h>

Contact *contacts = NULL;
Contact *currentContact = NULL;

static DWORD WINAPI chatUpdateThread(void*);
typedef struct sChatUpdateThreadArg
{
    ChatHistory *chatHistory;
    int startFrom;
} ChatUpdateThreadArg;

bool logIn(const SOCKET socket)
{
    DBG_FUNC();
    char    nickname[NICKNAME_LEN+1];
    bool    result = false;
    Packet  out, in;

    PendingRequest *request;

    printRequest("Enter nickname or q to quit:");
    readInBuffer(nickname, NICKNAME_LEN);
    clearRequest();
    if (strcmp(nickname, "q") == 0)
        return false;

    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        return false;
    }

    out = createPacket(TYPE_REQUEST, COMMAND_LOGIN, 0, request->id);
    addPacketString(&out, nickname);
    sendPacket(socket, out, socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    in = packetFromBytes(request->data);

    switch (in.status) {
    case STATUS_OK:
        result = true;
        break;
    default:
        printStatusErrorMessage(in.status);
    }

    if (in.data != NULL)
        deletePacket(in);
    if (out.data != NULL)
        deletePacket(out);
    deleteRequest(&request);
    clearRequest();
    return result;
}

void showPrivateChats(void)
{
    DBG_FUNC();
    Contact *contact = contacts;
    char input;
    int max, selected, i, ch, start = 0;

    while (true) {
        if (contact == NULL) {
            printNotification(formatDefault, "No contacts found\n");
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
        readInBuffer(&input, sizeof(char));
        selected = input - '0';

        if (selected > max || selected < 0) {
            printNotification(formatDefault, "No such option\n");
            continue;
        }

        for (contact = contacts, i = 0; i < (selected); i++)
            contact = contact->next;
        clearRequest();
        openChat(contact);
    }
}

void openChat(const Contact *contact)
{
    DBG_FUNC();
    char inputBuffer[100];
    HANDLE chatUpdateThreadHandle;
    ChatUpdateThreadArg chatUpdateArg;

    printContactName("Chat with %s:", contact->nickname);
    printChatHistory(contact->chatHistory, 0);
    printRequest("Type your message, type /quit to quit");

    chatUpdateArg.chatHistory = &contact->chatHistory;
    chatUpdateArg.startFrom = 0;

    chatUpdateThreadHandle = CreateThread(NULL, 0, chatUpdateThread, &chatUpdateArg, 0, 0);

    // Process input and send messages
    while (true) {
        readInBuffer(inputBuffer, sizeof(inputBuffer));
        if (strcmp(inputBuffer, "/quit") == 0) {
            TerminateThread(chatUpdateThreadHandle, 0);
            clearScreen();
            return;
        }
        sendMessage(socketServer, contact, inputBuffer);
        printChatHistory(contact->chatHistory, 0);
    }
}

void sendMessage(const SOCKET socket, const Contact *contact, const char *message)
{
    DBG_FUNC();
    PendingRequest  *request = NULL;
    int             try, respond;
    Packet          pIn, pOut;

    if (contact == NULL) {
        DBG_ERROR("No contact is selected\n");
        return;
    }

    pIn.data = NULL; pOut.data = NULL;

    for (try = 0; try < 3; try++) {
        deleteRequest(&request);
        deletePacket(pIn); deletePacket(pOut);
        request = createRequest();
        if (request == NULL) {
            DBG_FATAL("Failed to create request\n");
            return;
        }

        pOut = createPacket(TYPE_REQUEST, COMMAND_MESSAGE, 0, request->id);
        addPacketString(&pOut, contact->nickname);
        addPacketString(&pOut, message);
        sendPacket(socket, pOut, &socketServerMutex);

        WaitForSingleObject(request->event, INFINITE);
        WaitForSingleObject(request->mutex, INFINITE);

        pIn = packetFromBytes((char*)request->data);
        deleteRequest(&request);
        if (pIn.data == NULL) {
            DBG_ERROR("Can't create packet from respond\n");
            continue;
        }
        break;
    }
    respond = pIn.status; // FIXME handle if loop terminated without proper respond

    switch (respond) {
    case STATUS_OK:
        DBG_INFO("Message sent\n");
        addMessage(contact, message, true);
        break;
    default:
        printStatusErrorMessage(respond);
    }
    deleteRequest(&request);
    deletePacket(pIn); deletePacket(pOut);
}

void createChat(SOCKET socket)
{
    DBG_FUNC();
    PendingRequest  *request;
    char            nickname[NICKNAME_LEN+1];
    Packet          in, out;

    printRequest("Enter nickname or q to quit: ");
    readInBuffer(nickname, NICKNAME_LEN);
    clearRequest();

    if (strcmp(nickname, "q") == 0)
        return;

    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        printError("Can't create chat");
        return;
    }
    out = createPacket(TYPE_REQUEST, COMMAND_CREATE_CHAT, 0, request->id);
    addPacketString(&out, nickname);
    sendPacket(socket, out, &socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    in = packetFromBytes(request->data);

    switch (in.status) {
    case STATUS_OK:
        DBG_INFO("Chat created\n");
        printSuccess("Chat created");
        createContact(nickname);
        break;
    default:
        printStatusErrorMessage(in.status);
    }

    deleteRequest(&request);
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