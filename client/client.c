#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "chatsManager.h"
#define LOG_TO_FILE
#include "clientUtils.h"
#include "consoleControl.h"
#include "consoleInput.h"
#include "consoleOutput.h"
#include "contactsManager.h"

#include <conio.h>
#include <debug.h>
#include <process.h>
#include <socketUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <synchapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "client.h"

#define BUFSIZE 1024
#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

SOCKET  socketServer = INVALID_SOCKET;
HANDLE  socketThreadRunMutex;
AppData appData;

static void initConsole(void)
{
    initDebug("logs.log");
    enableVirtualProcessing(true);
    setAlternateConsoleBuffer(true);
    disableSelection(true);
    hideCursor();
    initConsoleSize();

    lockConsoleSize(true);
    _beginthread(consoleDrawThread, 0, 0);
}

int main(void)
{
    WSADATA wsadata;
    int     unread = 0, choice;
    HANDLE  socketThreadMutex;
    appData.contactCount = 0;

    socketThreadRunMutex = CreateMutex(NULL, TRUE, NULL);

    initConsole();
    // Set fixed console size
    if (getConsoleWidth() < 119 || getConsoleHeight() < 29) {
        printf("Invalid console size %dx%d.\nPlease, resize to 119x29 or larger size and start the program again.\n",
               getConsoleWidth(), getConsoleHeight());
        return 0;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

    socketServer = createActiveSocket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socketServer == INVALID_SOCKET) {
        printf("Error creating socket\n");
        return 1;
    }
    socketThreadMutex = (HANDLE)_beginthread(socketThread, 0, NULL);
    // TODO add handle for run mutex
    _beginthread(notificationThread, 0, NULL);

    if (!logIn(socketServer)) {
        printError("Can't login\n");
        closesocket(socketServer);
        WSACleanup();
        return 1;
    }
    printSuccess("Login success\n");

    socketServerMutex = CreateMutex(NULL, FALSE, NULL);

    while (1) {
        printRequest("Enter command: 1-list chats; 2-create chat; 0 - quit: ");

        choice = readChar(false) - '0';
        clearRequest();

        switch (choice) {
        case 0:
            goto exit;
        case 1:
            showPrivateChats();
            break;
        case 2:
            createChat(socketServer);
            break;
        default:
            printError("Invalid choice");
        }
    }

    exit:
    lockConsoleSize(false);
    setTextColor(fgDefault | bgDefault);
    if (socketServer != INVALID_SOCKET)
        closesocket(socketServer);

    // Wait while thread stops
    ReleaseMutex(socketThreadRunMutex);
    WaitForSingleObject(socketThreadMutex, INFINITE);

    WSACleanup();
}