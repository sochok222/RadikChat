#ifndef RADIKCHAT_CONTACT_H
#define RADIKCHAT_CONTACT_H

typedef enum
{
    MESSAGE_SEND_FAILED,
    MESSAGE_SEND_SUCCESS,
    MESSAGE_SEND_PENDING,
} MessageState;

typedef struct sMessage
{
    bool            sender; // true - 'our' false - 'their'
    MessageState    state;
    char            *text;
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
    ChatHistory     chat_history;
    struct sContact *next;
} Contact;

#endif //RADIKCHAT_CONTACT_H
