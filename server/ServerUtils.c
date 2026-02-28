#include "ServerUtils.h"
#include <ws2tcpip.h>

typedef struct sClientInfo
{
    socklen_t addressSize;
    struct sockaddr_storage address;
    SOCKET socket;
    char request[MAX_REQUEST_SIZE];
    int recievedBytes;
    struct sClientInfo* next;
} ClientInfo;