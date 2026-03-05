#include "clientUtils.h"
#include <ws2tcpip.h>
#include <debug.h>

static fd_set fdMaster;

void initClientUtils()
{
    FD_ZERO(&fdMaster);
}

fd_set  waitForSeverRespond(SOCKET server)
{
    fd_set fdReads;
    if (!FD_ISSET(server, &fdMaster))
        FD_SET(server, &fdMaster);

    fdReads = fdMaster;

    if (select(0, &fdReads, 0, 0, 0) < 0) {
        DBG_FATAL("select() failed.\n");
        logWsaError(WSAGetLastError());
        exit(1); // TODO: add proper error handling
    }
    return fdReads;
}
