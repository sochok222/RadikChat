#ifndef RADIKCHAT_CHATSMANAGER_H
#define RADIKCHAT_CHATSMANAGER_H

#include <winsock.h>
#include "contact.h"

extern Contact *contacts;
extern Contact *current_contact;

void show_private_chats(void);
void create_chat(SOCKET socket);
void send_message(SOCKET server_socket, const Contact *contact, const char *message);
bool log_in(SOCKET socket);
void delete_chat(const Contact *contact);
int  update_unread_messages(void);
void open_chat(const Contact *contact);

#endif //RADIKCHAT_CHATSMANAGER_H
