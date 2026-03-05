#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include <ws2tcpip.h>
#include <winsock.h>

void    initClientUtils();
fd_set  waitForSeverRespond(SOCKET server);

#endif //RADIKCHAT_CLIENTUTILS_H
