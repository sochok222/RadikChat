#include "packetProcessor.h"
#include "server.h"

#include "consoleControl.h"
#include "serverUtils.h"
#include <debug.h>
#include <packetManager/packet.h>
#include <process.h>
#include <socketUtils.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT "1423"
#define MAX_THREAD_HANDLES 100
#define BACKLOG 10

SOCKET              g_socketListen;
CRITICAL_SECTION    g_criticalSection;
DWORD               g_threadCount;
HANDLE              g_threadHandles[MAX_THREAD_HANDLES];
HANDLE              g_IOCP;
PerSocketContext    *g_clients = NULL;
bool                g_shutDown = false;

int _cdecl main(int argc, char **argv)
{
    initDebug(NULL);
    setAlternateConsoleBuffer(true);
    enableVirtualProcessing(true);
    DBG_INFO("Starting server...\n");
    WSADATA wsadata;
    SYSTEM_INFO systemInfo;
    SOCKET acceptSocket;
    PerSocketContext *perSocketContext = NULL;
    DWORD recvNumBytes = 0;
    DWORD flags = 0;
    int nRet = 0;

    DBG_DEBUG("Initializing winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }

    GetSystemInfo(&systemInfo);
    g_threadCount = systemInfo.dwNumberOfProcessors * 2;

    InitializeCriticalSection(&g_criticalSection);

    for (int i = 0; i < MAX_THREAD_HANDLES; i++) {
        g_threadHandles[i] = INVALID_HANDLE_VALUE;
    }

    g_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_IOCP == INVALID_HANDLE_VALUE) {
        DBG_FATAL("CreateIoCompletionPort() failed to create completion port: %d", GetLastError());
        return 1;
    }

    // Create worker threads that will service overlapped IO packets
    DBG_DEBUG("Creating %d threads...\n", g_threadCount);
    for (DWORD i = 0; i < g_threadCount; i++) {
        g_threadHandles[i] = CreateThread(NULL, 0, workerThread, g_IOCP, 0, NULL);
        if (g_threadHandles[i] == INVALID_HANDLE_VALUE) {
            DBG_FATAL("CreateThread() failed to create thread: %d", GetLastError());
            return 1;
        }
    }

    g_socketListen = createPassiveSocket(PORT, SOCK_STREAM, AF_INET, BACKLOG);
    if (g_socketListen == INVALID_SOCKET) {
        DBG_FATAL("createPassiveSocket() failed to create socket: %d", GetLastError());
        return 1;
    }

    // Loop forever to accept incoming connections
    while (true) {
        acceptSocket = WSAAccept(g_socketListen, NULL, NULL, NULL, 0);
        if (acceptSocket == INVALID_SOCKET) {
            DBG_FATAL("WSAAccept() failed to create accept_socket: %d", GetLastError());
            return 1;
        }

        // Add returned socket to the IOCP; allocate and add context
        // data to global list of context structures
        perSocketContext = updateCompletionPort(acceptSocket, IO_OP_READ, true);
        if (perSocketContext == NULL) {
            DBG_FATAL("updateCompletionPort() failed to update completion port: %d", GetLastError());
            return 1;
        }

        // Post initial receive on this socket
        nRet = WSARecv(acceptSocket, &(perSocketContext->pIOContext->wsabuf),
                       1, &recvNumBytes, &flags,
                       &(perSocketContext->pIOContext->overlapped), NULL);
        if (nRet == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
            DBG_FATAL("WSARecv() failed: %d", WSAGetLastError());
            closeClient(perSocketContext, false);
        }
    }

    return 0;
}

DWORD WINAPI workerThread(LPVOID arg)
{
    HANDLE IOCP = (HANDLE)arg;
    bool success = false;
    int nRet = 0;
    LPWSAOVERLAPPED overlapped = NULL;
    PerSocketContext *perSocketContext = NULL;
    PerIOContext *perIOContext = NULL;
    WSABUF buffRecv;
    WSABUF buffSend;
    DWORD recvNumBytes = 0;
    DWORD sendNumBytes = 0;
    DWORD flags = 0;
    DWORD IOSize = 0;

    while (true) {
        success = GetQueuedCompletionStatus(IOCP, &IOSize,
                                            (PDWORD_PTR)&perSocketContext,
                                            (LPOVERLAPPED *)&overlapped,
                                            INFINITE);

        if (!success) {
            // Not always mean error
            DBG_INFO("GetQueuedCompletionStatus() failed: %d\n", GetLastError());
        }

        if (perSocketContext == NULL) {
            // PostQueuedCompletionStatus posted IO packet with
            // a NULL key. It is time to exit
            return 0;
        }

        if (!success || (success && (IOSize == 0))) {
            closeClient(perSocketContext, false);
            continue;
        }

        // Determine what type of IO packet has completed
        perIOContext = (PerIOContext*)overlapped;
        switch (perIOContext->IOOperation) {
        case IO_OP_READ:
            // A read operation has completed, process received packet and post another action on the socket
            DBG_INFO("Thread(%lu) received %d bytes\n", GetCurrentThreadId(), IOSize);
            perIOContext->wsabuf.buf += IOSize;
            perIOContext->wsabuf.len -= IOSize;
            if (IOSize >= PACKET_HEADER_SIZE && IOSize >= *(size_t*)(perIOContext + PACKET_SIZE_OFFSET)) {
                Packet packetReceived = packetFromBytes(perIOContext->buffer);
                if (packetReceived.command == COMMAND_LOGIN) {
                    DBG_INFO("Thread(%lu) received login packet\n", GetCurrentThreadId());
                    PerIOContext *sendIOContext = allocateIOContext();

                    Packet packetRespond = createPacket(TYPE_RESPOND, COMMAND_LOGIN, STATUS_OK, packetReceived.id);
                    packPacket(perSocketContext->Socket, packetRespond, NULL);

                    // Fill IO context with needed data to process packed send
                    sendIOContext->IOOperation = IO_OP_WRITE;
                    sendIOContext->totalBytes = PACKET_HEADER_SIZE;
                    sendIOContext->sentBytes = 0;
                    sendIOContext->wsabuf.buf = sendIOContext->buffer;
                    sendIOContext->wsabuf.len = PACKET_HEADER_SIZE;

                    memcpy(sendIOContext->buffer, packetRespond.data, PACKET_HEADER_SIZE);

                    nRet = WSASend(perSocketContext->Socket, &(sendIOContext->wsabuf), 1, &sendNumBytes, flags, &sendIOContext->overlapped, NULL);

                    if (nRet == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                        DBG_ERROR("Thread(%lu) WSASend failed error(%d)\n", GetCurrentThreadId(), WSAGetLastError());
                        closeClient(perSocketContext, false);
                        deleteIOContext(sendIOContext);
                    }
                } else {
                    DBG_INFO("Thread(%lu) unknown packet received packet\n", GetCurrentThreadId());
                }
            }

            // Post receive
            WSARecv(perSocketContext->Socket, &(perSocketContext->pIOContext->wsabuf),
                       1, &recvNumBytes, &flags,
                       &(perSocketContext->pIOContext->overlapped), NULL);
            break;
        case IO_OP_WRITE:
            DBG_INFO("Thread(%lu) sent %d bytes\n", GetCurrentThreadId(), IOSize);
            perIOContext->IOOperation = IO_OP_WRITE;
            perIOContext->sentBytes += IOSize;
            flags = 0;

            if (perIOContext->sentBytes < perIOContext->totalBytes) {
                // Previous IO operation sent not all bytes, post another WSASend
                buffSend.buf = perIOContext->buffer + perIOContext->sentBytes;
                buffSend.len = perIOContext->totalBytes - perIOContext->sentBytes;

                nRet = WSASend(perSocketContext->Socket, &buffSend, 1,
                               &sendNumBytes, flags, &perIOContext->overlapped, NULL);
                if (nRet == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                    DBG_ERROR("Thread(%lu) WSASend failed\n", GetCurrentThreadId(), WSAGetLastError());
                    closeClient(perSocketContext, false);
                    deleteIOContext(perIOContext);
                } else {
                    DBG_DEBUG("Thread(&lu) WSASend partially completed (%d bytes), WSASend posted\n", GetCurrentThreadId(), IOSize);
                }
            } else {
                // Previous write operation completed
                DBG_INFO("Thread(%lu) sent all respond\n", GetCurrentThreadId(), IOSize);
                deleteIOContext(perIOContext);
            }
            break;
        default: break;
        }
    }

    return 0;
}

PerSocketContext *updateCompletionPort(SOCKET socket, IO_Operation clientIO, bool addToList)
{
    DBG_FUNC();
    PerSocketContext *perSocketContext = NULL;

    perSocketContext = allocateSocketContext(socket, clientIO);
    if (perSocketContext == NULL) {
        return NULL;
    }

    g_IOCP = CreateIoCompletionPort((HANDLE)socket, g_IOCP, (DWORD_PTR)perSocketContext, 0);
    if (g_IOCP == INVALID_HANDLE_VALUE) {
        DBG_FATAL("CreateIoCompletionPort() failed to create completion port: %d", GetLastError());
        if (perSocketContext->pIOContext != NULL) {
            free(perSocketContext->pIOContext);
        }
        free(perSocketContext);
        return NULL;
    }

    if (addToList)
        addToSocketContextList(perSocketContext);

    DBG_INFO("updateCompletionPort: socket(%d) added to IOCP\n", perSocketContext->Socket);

    return perSocketContext;
}

PerSocketContext *allocateSocketContext(SOCKET socket, IO_Operation clientIO)
{
    DBG_FUNC();
    PerSocketContext *perSocketContext = NULL;

    EnterCriticalSection(&g_criticalSection);
    perSocketContext = malloc(sizeof(*perSocketContext));
    if (perSocketContext != NULL) {
        perSocketContext->pIOContext = malloc(sizeof(*perSocketContext->pIOContext));
        if (perSocketContext->pIOContext != NULL) {
            perSocketContext->Socket = socket;
            perSocketContext->pCtxtBack = NULL;
            perSocketContext->pCtxtForward = NULL;

            perSocketContext->pIOContext->overlapped.Internal = 0;
            perSocketContext->pIOContext->overlapped.InternalHigh = 0;
            perSocketContext->pIOContext->overlapped.Offset = 0;
            perSocketContext->pIOContext->overlapped.OffsetHigh = 0;
            perSocketContext->pIOContext->overlapped.hEvent = NULL;
            perSocketContext->pIOContext->IOOperation = clientIO;
            perSocketContext->pIOContext->IOContextForward = NULL;
            perSocketContext->pIOContext->totalBytes = 0;
            perSocketContext->pIOContext->sentBytes  = 0;
            perSocketContext->pIOContext->wsabuf.buf  = perSocketContext->pIOContext->buffer;
            perSocketContext->pIOContext->wsabuf.len  = sizeof(perSocketContext->pIOContext->buffer);

            ZeroMemory(perSocketContext->pIOContext->wsabuf.buf, perSocketContext->pIOContext->wsabuf.len);
        } else {
            DBG_FATAL("malloc PerIOContext failed");
        }
    } else {
        DBG_FATAL("malloc PerSocketContext failed");
    }

    InitializeCriticalSection(&perSocketContext->IOCriticalSection);

    LeaveCriticalSection(&g_criticalSection);

    return perSocketContext;
}

void closeClient(PerSocketContext *perSocketContext, bool graceful)
{
    DBG_FUNC();
    EnterCriticalSection(&g_criticalSection);

    if (perSocketContext != NULL) {
        DBG_DEBUG("closeClient: socket(%d) connection closing (graceful=%s)\n",
            perSocketContext->Socket, graceful ? "true" : "false");
        if (!graceful) {
            // force subsequent closesocket to be abortative
            LINGER linger;
            linger.l_onoff = 1;
            linger.l_linger = 0;
            setsockopt(perSocketContext->Socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
        }
        closesocket(perSocketContext->Socket);
        perSocketContext->Socket = INVALID_SOCKET;
        deleteFromSocketContextList(perSocketContext);
        perSocketContext = NULL;
    } else {
        DBG_DEBUG("perSocketContext is NULL\n");
    }

    LeaveCriticalSection(&g_criticalSection);
}

void addToSocketContextList(PerSocketContext *perSocketContext)
{
    PerSocketContext *temp = NULL;

    EnterCriticalSection(&g_criticalSection);

    if (g_clients == NULL) {
        // Add first node to the list
        perSocketContext->pCtxtBack = NULL;
        perSocketContext->pCtxtForward = NULL;
        g_clients = perSocketContext;
    } else {
        // Add node to head of list
        temp = g_clients;

        g_clients = perSocketContext;
        perSocketContext->pCtxtBack = temp;
        perSocketContext->pCtxtForward = NULL;

        temp->pCtxtForward = perSocketContext;
    }

    LeaveCriticalSection(&g_criticalSection);
}

void deleteFromSocketContextList(PerSocketContext *perSocketContext)
{
    PerSocketContext    *back;
    PerSocketContext    *forward;
    PerIOContext        *nextIO = NULL;
    PerIOContext        *tempIO = NULL;

    EnterCriticalSection(&g_criticalSection);

    if (perSocketContext != NULL) {
        back = perSocketContext->pCtxtBack;
        forward = perSocketContext->pCtxtForward;

        if ((back == NULL) && (forward == NULL)) {
            // This is the only node in the list
            g_clients = NULL;
        } else if ((back == NULL) && (forward != NULL)) {
            // Start node
            forward->pCtxtBack = NULL;
            g_clients = forward;
        } else if ((back != NULL) && (forward == NULL)) {
            // End node
            back->pCtxtForward = NULL;
        } else if (back && forward) {
            back->pCtxtForward = forward;
            forward->pCtxtBack = back;
        }

        // Free all IO context structs per socket
        tempIO = perSocketContext->pIOContext;
        do {
            nextIO = tempIO->IOContextForward;
            if (tempIO) {
                // Overlapped structure is safe to delete only when
                // posted IO has completed. This only need to be tested only
                // in the shutdown process.
                if (g_shutDown)
                    while (!HasOverlappedIoCompleted((LPOVERLAPPED)&tempIO)) Sleep(0);
                free(tempIO);
                tempIO = NULL;
            }
            tempIO = nextIO;
        } while (nextIO != NULL);
        free(perSocketContext);
        perSocketContext = NULL;
    } else {
        DBG_WARNING("perSocketContext is NULL\n");
    }

    LeaveCriticalSection(&g_criticalSection);
}

void deleteIOContext(PerIOContext *perIOContext)
{
     if (g_shutDown)
         while (!HasOverlappedIoCompleted((LPOVERLAPPED)&perIOContext)) Sleep(0);
     free(perIOContext);
}

PerIOContext *allocateIOContext()
{
    PerIOContext *perSocketContext = NULL;
    perSocketContext = malloc(sizeof(*perSocketContext));
    ZeroMemory(perSocketContext, sizeof(*perSocketContext));
    return perSocketContext;
}
