#include "client.h"
#include "contacts_manager.h"
#include <debug.h>

static void init_chat_history(ChatHistory *chat_history);

Contact *create_contact(const char *nickname)
{
    Contact *contact;

    contact = malloc(sizeof(*contact));
    init_chat_history(&contact->chat_history);
    contact->unread = 0;
    contact->nickname = malloc(strlen(nickname) + 1);
    strcpy(contact->nickname, nickname);

    contact->next = contacts;
    contacts = contact;
    app_data.contact_count++;
    return contact;
}

Contact *find_contact(const char *nickname)
{
    DBG_FUNC();
    Contact *p = contacts;
    while (p != NULL) {
        if (strcmp(p->nickname, nickname) == 0) {
            DBG_DEBUG("Found contact");
            return p;
        }
        p = p->next;
    }
    DBG_DEBUG("Contact not found");
    return NULL;
}

void delete_contact(const char *nickname)
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
    DBG_ERROR("Contact not found");
}

static void init_chat_history(ChatHistory *chat_history)
{
    chat_history->messages = 0;
    chat_history->head = NULL;
}

Message *add_message(Contact *contact, const char *message, bool sender, MessageState state)
{
    DBG_FUNC();
    Message *it = contact->chat_history.head;
    Message *new_message = malloc(sizeof(*new_message));

    while (it != NULL && it->next != NULL)
        it = it->next;
    if (it != NULL)
        it->next = new_message;
    else
        contact->chat_history.head = new_message;

    new_message->text = malloc(strlen(message) + 1);
    strcpy(new_message->text, message);
    new_message->next = NULL;
    new_message->sender = sender;
    new_message->state = state;
    contact->chat_history.messages++;
    SetEvent(app_data.message_event);
    return new_message;
}

int contact_count()
{
    DBG_FUNC();
    Contact *p = contacts;
    int count = 0;

    while (p != NULL) {
        count++;
    }
    return count;
}