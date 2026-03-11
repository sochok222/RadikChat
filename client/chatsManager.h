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
    char                *nickname;
    int                 unread;
    struct ChatHistory  *chatHistory;
} Contact;

extern Contact *contacts;

void showPrivateChats(void);
void createChat(SOCKET socket);
void deleteChat(const Contact *contact);
bool signIn(const SOCKET socket);
void updateUnreadMessages(void);
void initChatHistory(void);

#endif //RADIKCHAT_CHATSMANAGER_H
