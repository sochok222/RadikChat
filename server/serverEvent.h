#ifndef RADIKCHAT_SERVEREVENT_H
#define RADIKCHAT_SERVEREVENT_H

typedef enum eServerEventType
{
    EventPacket
} ServerEventType;

typedef struct sServerEvent {
    ServerEventType type;
} ServerEvent;

#endif //RADIKCHAT_SERVEREVENT_H
