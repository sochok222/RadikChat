#ifndef RADIKCHAT_CONTACT_H
#define RADIKCHAT_CONTACT_H

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
    char            *nickname;
    int             unread;
    ChatHistory     chatHistory;
    struct sContact *next;
} Contact;

#endif //RADIKCHAT_CONTACT_H
