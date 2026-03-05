#include "client.h"

#include "clientUtils.h"
#include "networkTypes.h"

#include <windows.h>
#include <debug.h>
#include <socketUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define NICKNAME_SIZE 20

#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

struct Message
{
    bool sender; // true - 'our' false - 'their'
    char *message;
    struct Message *next;
};

struct ChatHistory
{
    long long messages;
    struct Message *head;
};

struct Contact
{
    char *nickname;
    struct ChatHistory *chatHistory;
};

void showPrivateChats(void);
void createChat(void);
bool signIn(SOCKET socket);
void updateUnreadMessages(void);

int main(void)
{
    WSADATA wsadata;

    initDebug();

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

    SOCKET socketServer = createActiveSocket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socketServer == INVALID_SOCKET) {
        printf("Error creating socket\n");
        return 1;
    }

    int unread = 0, choice;

    if (!signIn(socketServer)) {
        closesocket(socketServer);
        WSACleanup();
        return 1;
    }
    _sleep(3000);

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
            createChat();
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

bool signIn(SOCKET socket)
{
    char    nickname[NICKNAME_SIZE+1];
    int     type = TYPE_LOGIN;
    int     nicknameLen;
    char    readBuffer[100];
    int     read, sent;
    system("cls");

    printf("Enter your nickname (max 20 characters): ");
    fseek(stdin,0,SEEK_END);
    scanf("%20s", nickname);
    nicknameLen = strlen(nickname) + 1;

    sent = send(socket, (char*)&type, 4, 0);
    DBG_INFO("Sent %d bytes\n", sent);
    send(socket, (char*)&nicknameLen, 4, 0);
    send(socket, nickname, nicknameLen, 0);

    fd_set fdReads = waitForSeverRespond(socket);

    if (FD_ISSET(socket, &fdReads)) {
        read = recv(socket, readBuffer, sizeof(readBuffer), 0);

        if (*(int*)(readBuffer) == TYPE_LOGIN_SUCCESS) {
            DBG_INFO("Login successful\n");
        } else if (*(int*)(readBuffer) == TYPE_LOGIN_FAILURE) {
            DBG_INFO("Login failed\n");
            return false;
        } else {
            DBG_ERROR("Unknown respond\n");
            return false;
        }
    }

    return true;
}

void showPrivateChats(void)
{

}

void createChat(void)
{

}

void updateUnreadMessages(void)
{

}