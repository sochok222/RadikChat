#ifndef RADIKCHAT_CONTACTSMANAGER_H
#define RADIKCHAT_CONTACTSMANAGER_H

#include "chatsManager.h"

Contact *createContact(const char *nickname);
void deleteContact(const char *nickname);
void addMessage(Contact *contact, const char *message, bool sender);

#endif //RADIKCHAT_CONTACTSMANAGER_H
