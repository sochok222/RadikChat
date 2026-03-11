#include "client.h"

#include "chatsManager.h"

#include <debug.h>
#include <socketUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <synchapi.h>
#include <process.h>
#include "clientUtils.h"

#define BUFSIZE 1024
#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

SOCKET socketServer;

int main(void)
{
    WSADATA wsadata;
    int unread = 0, choice;

    initDebug();
    initChatHistory();

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

    socketServer = createActiveSocket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socketServer == INVALID_SOCKET) {
        printf("Error creating socket\n");
        return 1;
    }
    _beginthread(socketThread, 0, NULL);

    if (!signIn(socketServer)) {
        closesocket(socketServer);
        WSACleanup();
        return 1;
    }
    _sleep(3000);

    socketServerMutex = CreateMutex(NULL, FALSE, NULL);

    while (1) {
        system("cls");
        printf("You have %d unread messages\n", unread);
        printf("Enter command: 1-private chats; 2-create chat: ");
        fseek(stdin,0,SEEK_END);
        scanf("%d", choice);

        switch (choice) {
        case 1:
            showPrivateChats();
        case 2:
            createChat(socketServer);
        case 3:
            updateUnreadMessages();
        default:
            break;
        }
    }

    // ReSharper disable once CppDFAUnreachableCode
    closesocket(socketServer);
    WSACleanup();
}

