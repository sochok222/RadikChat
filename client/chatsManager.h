#ifndef RADIKCHAT_CHATSMANAGER_H
#define RADIKCHAT_CHATSMANAGER_H

#include <winsock.h>

typedef struct sMessage
{
    bool            sender; // true - 'our' false - 'their'
    char            *message;
    struct sMessage *next;
} Message;

typedef struct sChatHistory
{
    long long   messages;
    Message     *head;
} ChatHistory;

typedef struct sContact
{
    char            *nickname;
    int             unread;
    ChatHistory     chatHistory;
    struct sContact *next;
} Contact;

extern Contact *contacts;
extern Contact *currentContact;

void showPrivateChats(void);
void createChat(SOCKET socket);
void sendMessage(const SOCKET socket, const Contact *contact, const char *message);
bool logIn(const SOCKET socket);
void deleteChat(const Contact *contact);
int  updateUnreadMessages(void);
void openChat(Contact *contact);

#endif //RADIKCHAT_CHATSMANAGER_H
