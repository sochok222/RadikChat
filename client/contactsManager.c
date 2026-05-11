#include "contactsManager.h"
#include <debug.h>
#include "client.h"

static void initChatHistory(ChatHistory *chatHistory);

Contact *createContact(const char *nickname)
{
    Contact *contact;

    contact = malloc(sizeof(*contact));
    initChatHistory(&contact->chatHistory);
    contact->unread = 0;
    contact->nickname = malloc(strlen(nickname) + 1);
    strcpy(contact->nickname, nickname);

    contact->next = contacts;
    contacts = contact;
    appData.contactCount++;
    return contact;
}

Contact *findContact(const char *nickname)
{
    DBG_FUNC();
    Contact *p = contacts;
    while (p != NULL) {
        if (strcmp(p->nickname, nickname) == 0) {
            DBG_DEBUG("Found contact\n");
            return p;
        }
        p = p->next;
    }
    DBG_DEBUG("Contact not found\n");
    return NULL;
}

void deleteContact(const char *nickname)
{
    DBG_FUNC();
    Contact **p = &contacts;
    while(*p) {
        if (strcmp((*p)->nickname, nickname) == 0) {
            *p = (*p)->next;
            free((*p)->nickname);
            free(*p);
            return;
        }
        p = &(*p)->next;
    }
    DBG_ERROR("Contact not found\n");
}

static void initChatHistory(ChatHistory *chatHistory)
{
    chatHistory->messages = 0;
    chatHistory->head = NULL;
}

void addMessage(Contact *contact, const char *message, bool sender)
{
    DBG_FUNC();
    Message *it = contact->chatHistory.head;
    Message *newMessage = malloc(sizeof(*newMessage));

    while (it != NULL && it->next != NULL)
        it = it->next;
    if (it != NULL)
        it->next = newMessage;
    else
        contact->chatHistory.head = newMessage;

    newMessage->message = malloc(strlen(message) + 1);
    strcpy(newMessage->message, message);
    newMessage->next = NULL;
    newMessage->sender = sender;
    contact->chatHistory.messages++;
    SetEvent(appData.messageEvent);
}

int contactCount()
{
    DBG_FUNC();
    Contact *p = contacts;
    int count = 0;

    while (p != NULL) {
        count++;
    }
    return count;
}