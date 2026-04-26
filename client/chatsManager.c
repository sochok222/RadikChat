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

Contact *contacts = NULL;
Contact *currentContact = NULL;

// static int  readInput(FILE *fp, char *buffer, size_t bufferSize);

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
    return result;
}

void showPrivateChats(void)
{
    DBG_FUNC();
    Contact *contact = contacts;
    int max, selected, i, ch, start = 0;

    while (true) {
        if (contact == NULL) {
            printNotification(formatDefault, "No contacts found\n");
            return;
        }

        printRequest("press u to go up, d to go down and i to enter insert mode");
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
            }
        }

        printRequest("Select chat: ");
        // TODO read input
        // fseek(stdin, 0, SEEK_END);
        // scanf("%d", &selected);

        if (selected > max || selected < 1) {
            printNotification(formatDefault, "No such option\n");
            continue;
        }

        for (contact = contacts, i = 0; i < (selected - 1); i++)
            contact = contact->next;
        openChat(contact);
    }
}

void openChat(const Contact *contact)
{
    DBG_FUNC();
    char inputBuffer[100];
    int inputPos = 0, ch;

    printf("Chat with %s:\n", contact->nickname);

    Message *it = contact->chatHistory.head;

    while (it != NULL) {
        printf("%s: %s\n", it->sender == true ? "You" : contact->nickname, it->message);
        it = it->next;
    }


    // Process input and send messages
    while (true) {
        if (_kbhit()) {
            switch (ch = getch()) {
            case 8: /* backspace */
                if (inputPos >= 0) {
                    inputPos--;
                    putch(ch);
                    putch(' ');
                    putch(ch);
                }
                break;
            case 13: case 10: /* enter key */
                inputBuffer[inputPos] = 0x0;
                inputPos = 0;
                if (strcmp(inputBuffer, "/quit") == 0)
                    return;
                sendMessage(socketServer, contact, inputBuffer);
                break;
            default:
                inputBuffer[inputPos++] = ch;
                putch(ch);
            }
        }
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
    Sleep(1500);
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
    Sleep(1500);
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
//
// static int readInput(FILE *fp, char *buffer, size_t bufferSize)
// {
//     char format[16];
//     snprintf(format, sizeof(format), "%%%zus", bufferSize);
//     fseek(fp, 0, SEEK_END);
//     return fscanf(fp, format, buffer);
// }
