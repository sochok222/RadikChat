#include "Client.h"

#include "SocketUtils.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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

int main(void)
{
    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

    SOCKET socketServer = createActiveSocket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socketServer == INVALID_SOCKET) {
        printf("Error creating socket\n");
        return 1;
    }
    closesocket(socketServer);

    int unread, choice;

    if (!signIn(socketServer))
        return 1;

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
        }
    }
}

bool signIn(SOCKET socket)
{
    // Signin process
}

void showPrivateChats(void)
{

}

void createChat(void)
{

}