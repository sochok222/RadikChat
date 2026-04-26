#ifndef RADIKCHAT_CHATSMANAGER_H
#define RADIKCHAT_CHATSMANAGER_H

#include <winsock.h>
#include "contact.h"

extern Contact *contacts;
extern Contact *currentContact;

void showPrivateChats(void);
void createChat(SOCKET socket);
void sendMessage(SOCKET socket, const Contact *contact, const char *message);
bool logIn(SOCKET socket);
void deleteChat(const Contact *contact);
int  updateUnreadMessages(void);
void openChat(const Contact *contact);

#endif //RADIKCHAT_CHATSMANAGER_H
