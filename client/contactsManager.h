#ifndef RADIKCHAT_CONTACTSMANAGER_H
#define RADIKCHAT_CONTACTSMANAGER_H

#include "chatsManager.h"

Contact *create_contact(const char *nickname);
Contact *find_contact(const char *nickname);
void    delete_contact(const char *nickname);
Message *add_message(Contact *contact, const char *message, bool sender, MessageState state);
int     contact_count();

#endif //RADIKCHAT_CONTACTSMANAGER_H
