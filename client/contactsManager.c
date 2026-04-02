#include "contactsManager.h"
#include <debug.h>

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
    return contact;
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
}