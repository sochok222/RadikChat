#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "chatsManager.h"
#define LOG_TO_FILE
#include <debug.h>
#include <socketUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <synchapi.h>
#include <process.h>
#include "clientUtils.h"
#include "consoleOutput.h"
#include "consoleControl.h"

#define BUFSIZE 1024
#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

SOCKET  socketServer = INVALID_SOCKET;
HANDLE  socketThreadRunMutex;

int main(void)
{
    WSADATA wsadata;
    int     unread = 0, choice;
    HANDLE  socketThreadMutex;
    DWORD64 consoleSize = 0;

    initDebug("logs.log");
    system("cls");
    socketThreadRunMutex = CreateMutex(NULL, TRUE, NULL);
    // Init console
    setAlternateConsoleBuffer(true);
    initConsoleOutput();
    drawTextInputBar();
    drawNotificationBar();

    // Set fixed console size
    getConsoleSize(&consoleSize);
    if (getConsoleWidth(consoleSize) < 119 || getConsoleHeight(consoleSize) < 29) {
        printf("Invalid console size %dx%d.\nPlease, resize to 119x29 or larger size and start the program again.\n",
               getConsoleWidth(consoleSize), getConsoleHeight(consoleSize));
        return 0;
    }
    lockConsoleSize(true);

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
    Sleep(1500);
    clearNotificationBar();
    clearTextInputBar();
    Sleep(1500);

    socketServerMutex = CreateMutex(NULL, FALSE, NULL);

    while (1) {
        system("cls");
        printf("You have %d unread messages\n", unread);
        printf("Enter command: 1-list chats; 2-create chat; 0 - quit: ");
        fseek(stdin,0,SEEK_END);
        scanf("%d", &choice);

        switch (choice) {
        case 0:
            goto exit;
        case 1:
            showPrivateChats();
            break;
        case 2:
            createChat(socketServer);
            break;
        case 3:
            unread = updateUnreadMessages();
            break;
        default:
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