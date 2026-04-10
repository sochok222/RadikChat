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

#define BUFSIZE 1024
#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

SOCKET  socketServer = INVALID_SOCKET;
HANDLE  socketThreadRunMutex;

bool SetConsoleSize(const COORD size) {
    if (!SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), size))
        return false;

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return false;

    SMALL_RECT sr;

    sr.Left = 0;
    sr.Top = 0;
    sr.Right = size.X - 1;
    sr.Bottom = size.Y - 1;

    if (!SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &sr))
        return false;

    return true;
}

int main(void)
{
    WSADATA wsadata;
    int     unread = 0, choice;
    HANDLE  socketThreadMutex;

    initDebug(NULL);
    system("cls");
    socketThreadRunMutex = CreateMutex(NULL, TRUE, NULL);

    // Set fixed console size
    HWND consoleWindow = GetConsoleWindow();
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi = {0};
    GetConsoleScreenBufferInfo(consoleHandle, &csbi);
    if (csbi.srWindow.Right < 130 || csbi.srWindow.Bottom < 35) {
        printf("Invalid console size %dx%d.\nPlease, resize to 130x35 or larger size and start the program again.\n",
                csbi.srWindow.Right, csbi.srWindow.Bottom);
        return 0;
    }
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

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
        closesocket(socketServer);
        WSACleanup();
        return 1;
    }
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
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & WS_MAXIMIZEBOX & WS_SIZEBOX);
    setTextColor(fgDefault | bgDefault);
    if (socketServer != INVALID_SOCKET)
        closesocket(socketServer);

    // Wait while thread stops
    ReleaseMutex(socketThreadRunMutex);
    WaitForSingleObject(socketThreadMutex, INFINITE);

    WSACleanup();
}