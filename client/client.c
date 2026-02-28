#include "client.h"

#include "socket_utils.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SERVER_ADDRESS "192.168.0.184"
#define SERVER_PORT "1423"

void showPrivateChats(void);
void createChat(void);
bool signIn(SOCKET socket);

int main(void)
{
    SOCKET socketServer = createActiveSocket(SERVER_ADDRESS, SERVER_PORT, SOCK_STREAM);
    if (socketServer == INVALID_SOCKET) {
        printf("Error creating socket\n");
        return 1;
    }

    int unread, choice;

    if (!signIn())
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

bool signIn(void)
{

}

void showPrivateChats(void)
{
}