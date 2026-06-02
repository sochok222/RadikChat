#ifndef RADIKCHAT_SERVER_H
#define RADIKCHAT_SERVER_H

#include <mswsock.h>
#define MAX_BUFFER_SIZE 1024

typedef enum eIOOperation
{
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_ACCEPT,
} IOOperation;

typedef struct sPerIOContext
{
    WSAOVERLAPPED   overlapped;
    char            buffer[MAX_BUFFER_SIZE]; // TODO make heap-allocated
    WSABUF          wsabuf;
    int             totalBytes;
    int             sentBytes;
    IOOperation     iooperation;
    SOCKET          socketAccept;

    struct sPerIOContext *forward;
} PerIOContext;

#endif //RADIKCHAT_SERVER_H