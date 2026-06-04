#ifndef RADIKCHAT_SERVER_H
#define RADIKCHAT_SERVER_H

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
    int             totalBytes;
    int             sentBytes;
    IO_Operation    IOOperation;
    SOCKET          socketAccept;

    struct sPerIOContext *IOContextForward;
} PerIOContext;

typedef struct sPerSocketContext {
    SOCKET                    Socket;

    PerIOContext*             pIOContext;
    struct sPerSocketContext  *pCtxtBack;
    struct sPerSocketContext  *pCtxtForward;
} PerSocketContext;

DWORD WINAPI        workerThread(LPVOID arg);
// addToList is false for listening socket as it stored in global variable
PerSocketContext    *updateCompletionPort(SOCKET s, IO_Operation clientIO, bool addToList);
void                closeClient(PerSocketContext *perSocketContext, bool graceful);
PerSocketContext    *allocateSocketContext(SOCKET s, IO_Operation clientIO);
void                freeSocketContextList();
void                addToSocketContextList(PerSocketContext *perSocketContext);
void                deleteFromSocketContextList(PerSocketContext *perSocketContext);

#endif //RADIKCHAT_SERVER_H