#ifndef RADIKCHAT_CONTACTSMANAGER_H
#define RADIKCHAT_CONTACTSMANAGER_H

#include "chatsManager.h"

Contact *createContact(const char *nickname);
Contact *findContact(const char *nickname);
void    deleteContact(const char *nickname);
Message *addMessage(Contact *contact, const char *message, bool sender, MessageState state);
int     contactCount();

#endif //RADIKCHAT_CONTACTSMANAGER_H
