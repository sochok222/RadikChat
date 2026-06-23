#ifndef RADIKCHAT_SERVER_H
#define RADIKCHAT_SERVER_H

#include "tlPacket.h"

#include <mswsock.h>
#define MAX_BUFFER_SIZE 4096

typedef enum eIOOperation
{
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_ACCEPT,
} IO_Operation;

typedef struct sPerIOContext
{
    WSAOVERLAPPED   overlapped;
    char            buffer[MAX_BUFFER_SIZE];
    WSABUF          wsabuf;
    uint32_t        totalBytes;
    DWORD           sentBytes;
    IO_Operation    IOOperation;
    SOCKET          socketAccept;
    TLPacket        *tlPacket;

    struct sPerIOContext *IOContextForward;
} PerIOContext;

typedef struct sPerSocketContext {
    SOCKET                      Socket;
    char                        *nickname;
    PerIOContext                *pIOContext;
    CRITICAL_SECTION            IOCriticalSection;
    struct sPerSocketContext    *pCtxtBack;
    struct sPerSocketContext    *pCtxtForward;
} PerSocketContext;

extern PerSocketContext *g_clients;

DWORD WINAPI        workerThread(LPVOID arg);
PerSocketContext    *updateCompletionPort(SOCKET s, IO_Operation clientIO, bool addToList);
void                closeClient(PerSocketContext *perSocketContext, bool graceful);
PerSocketContext    *allocateSocketContext(SOCKET s, IO_Operation clientIO);
PerIOContext        *allocateIOContext();
void                freeSocketContextList();
void                addToSocketContextList(PerSocketContext *perSocketContext);
void                deleteFromSocketContextList(PerSocketContext *perSocketContext);
void                deleteIOContext(PerIOContext *perIOContext);

#endif //RADIKCHAT_SERVER_H