#include "server.h"
#include "packet_processor.h"

#include "console_control.h"
#include "debug.h"
#include "global_values.h"
#include "miscellaneous.h"
#include "packet.h"
#include "queue.h"
#include "server_database.h"
#include "socket_utils.h"
#include "tl_packet.h"

#include <process.h>
#include <signal.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BACKLOG 10

SOCKET              g_listen_socket = INVALID_SOCKET;
CriticalSection     g_critical_section = NULL;
DWORD               g_thread_count;
HANDLE              *g_thread_handles = NULL;
HANDLE              g_iocp = INVALID_HANDLE_VALUE;
PerSocketContext    *g_clients = NULL;
bool                g_shut_down = false;
HANDLE              g_shutdown_event = INVALID_HANDLE_VALUE;
LPFN_ACCEPTEX       acceptex = NULL;

static void signal_handler(int signal_code);
static void clear_all();
static bool process_packet(TlPacket *tl_packet, PerSocketContext *per_socket_context);

int _cdecl main(int argc, char **argv)
{
    // Register signal/exit handlers
    atexit(clear_all);
    signal(SIGABRT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGTERM, signal_handler);

    if (!init_debug(NULL)) {
        DBG_FATAL("Failed to initialize debugging");
        goto exit;
    }

    g_shutdown_event = CreateEvent(NULL, false, false, NULL);
    initialize_critical_section(&g_critical_section);

    enable_virtual_processing(true);
    set_alternate_console_buffer(true);

    init_server_database(SERVER_DATABASE_PATH);

    DBG_DEBUG("Initializing winsock...");
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        DBG_FATAL("Error: Failed to initialize winsock");
        goto exit;
    }

    // Create global completion port
    g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_iocp == INVALID_HANDLE_VALUE) {
        DBG_FATAL("CreateIoCompletionPort() failed to create completion port: %d", GetLastError());
        goto exit;
    }

    // Get count of cores in cpu
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    g_thread_count = system_info.dwNumberOfProcessors * 2;
    g_thread_handles = malloc(g_thread_count * sizeof(HANDLE));

    // Create worker threads that will service overlapped IO packets
    DBG_DEBUG("Creating %d threads...", g_thread_count);
    for (DWORD i = 0; i < g_thread_count; i++) {
        g_thread_handles[i] = CreateThread(NULL, 0, worker_thread, g_iocp, 0, NULL);
        if (g_thread_handles[i] == INVALID_HANDLE_VALUE) {
            DBG_FATAL("CreateThread() failed to create thread: %d", GetLastError());
            goto exit;
        }
    }

    g_listen_socket = create_passive_socket(SERVER_PORT, SOCK_STREAM, AF_INET, BACKLOG);
    if (g_listen_socket == INVALID_SOCKET) {
        DBG_FATAL("create_passive_socket() failed to create socket: %d", GetLastError());
        goto exit;
    }

    // Loop forever to accept incoming connections
    PerSocketContext *per_socket_context = NULL;
    int n_ret = 0;

    // Add listen socket to the completion port
    per_socket_context = update_completion_port(g_listen_socket, IO_OP_ACCEPT, true);

    // Get AcceptEx function pointer
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD num_bytes = 0;
    n_ret = WSAIoctl(g_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                     &acceptex_guid, sizeof(acceptex_guid),
                     &acceptex, sizeof(acceptex),
                     &num_bytes, NULL, NULL);
    if (n_ret == SOCKET_ERROR) {
        DBG_FATAL("WSAIoctl failed with error code %d", WSAGetLastError());
        log_wsa_error(WSAGetLastError());
        goto exit;
    }

    // Post initial accept
    SOCKET accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    per_socket_context->accept_socket = accept_socket;
    bool ret_val = acceptex(g_listen_socket, accept_socket, per_socket_context->io_context->buffer,
        0, sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
        &per_socket_context->io_context->total_bytes, &per_socket_context->io_context->overlapped);
    if (ret_val == false && WSAGetLastError() != WSA_IO_PENDING) {
        DBG_FATAL("Failed to post initial acceptex! Error %d", WSAGetLastError());
        log_wsa_error(WSAGetLastError());
        goto exit;
    }
    DBG_INFO("Successfully posted initial accept");

    WaitForSingleObject(g_shutdown_event, INFINITE);
    exit:
    return 0;
}

static void clear_all()
{
    g_shut_down = true;
    // Make all threads to stop
    DBG_DEBUG("Stopping all threads...");
    for (DWORD i = 0; i < g_thread_count; i++) {
        if (g_thread_handles[i] != INVALID_HANDLE_VALUE) {
            PostQueuedCompletionStatus(g_thread_handles[i], 0, 0, NULL);
        }
    }
    DWORD wait_result = WaitForMultipleObjects(g_thread_count, g_thread_handles, TRUE, 5000);
    if (wait_result == WAIT_TIMEOUT || wait_result == WAIT_FAILED) {
        DBG_ERROR("WaitForMultipleObjects() failed to wait for all threads!!! %s",
            wait_result == WAIT_TIMEOUT ? "WAIT_TIMEOUT" : "WAIT_FAILED");
    }
    DBG_DEBUG("All threads are stopped");
    for (DWORD i = 0; i < g_thread_count; i++) {
        safe_close_handle(g_thread_handles[i]);
    }
    if (g_thread_handles != NULL) {
        free(g_thread_handles);
    }
    DBG_DEBUG("All threads are closed");

    safe_close_handle(g_shutdown_event);
    closesocket(g_listen_socket);
    free_socket_context_list();
    delete_critical_section(&g_critical_section);

    deinit_debug();
    WSACleanup();
    DBG_DEBUG("Finished clearing");
}

DWORD WINAPI worker_thread(LPVOID arg)
{
    HANDLE IOCP = (HANDLE)arg;
    bool success = false;
    int n_ret = 0;
    LPWSAOVERLAPPED overlapped = NULL;
    PerSocketContext *per_socket_context = NULL;
    PerIoContext *per_io_context = NULL;
    DWORD flags = 0;
    DWORD io_size = 0;
    PacketParseStatus parse_status;

    while (true) {
        success = GetQueuedCompletionStatus(IOCP, &io_size,
                                            (PDWORD_PTR)&per_socket_context,
                                            (LPOVERLAPPED *)&overlapped,
                                            INFINITE);

        if (!success) {
            // Not always mean error
            DBG_INFO("Thread (%llu) GetQueuedCompletionStatus() failed: %d", GetCurrentThreadId(), GetLastError());
        }

        if (per_socket_context == NULL) {
            // PostQueuedCompletionStatus posted IO packet with
            // a NULL key. It is time to exit
            return 0;
        }

        per_io_context = (PerIoContext*)overlapped;
        if ( (!success || (success == true && io_size == 0)) && per_io_context->io_operation != IO_OP_ACCEPT ) {
            DBG_WARNING("Thread (%llu) IOSize == 0, Error code (%d)");
            log_wsa_error(WSAGetLastError());
            close_client(per_socket_context, false);
            continue;
        }

        // Determine type of IO operation
        switch (per_io_context->io_operation) {
        case IO_OP_READ:
            DBG_INFO("Thread(%llu) received %d bytes", GetCurrentThreadId(), io_size);
            per_io_context->wsabuf.buf += io_size;
            per_io_context->wsabuf.len -= io_size;

            if (io_size >= PKT_HEADER_SIZE) {
                TlPacketSize packet_size = *(TlPacketSize*)(per_io_context->buffer + PKT_SIZE_OFFSET);
                if (packet_size > MAX_PACKET_SIZE) {
                    DBG_WARNING("Client sent too large packet. Size %u bytes, but maximum is %u", packet_size, MAX_PACKET_SIZE);
                    close_client(per_socket_context, false);
                    continue;
                }
                if (packet_size < per_io_context->wsabuf.len) {
                    DBG_INFO("Received %lu of %lu bytes, waiting for more data...", per_io_context->wsabuf.len, packet_size);
                    WSARecv(per_socket_context->socket, &(per_socket_context->io_context->wsabuf),
                            1, NULL, &flags, &(per_socket_context->io_context->overlapped), NULL);
                    continue;
                }

                TlPacket *tl_packet;
                if ((parse_status = packet_from_raw_data(per_io_context->buffer, &tl_packet)) != PKT_PARSE_OK) {
                    DBG_ERROR("Failed to parse packet %s", get_parse_status_string(parse_status));
                    // TODO discard this packet, send respond, clear read buffer
                    close_client(per_socket_context, false);
                    continue;
                }

                bool is_processed = process_packet(tl_packet, per_socket_context); // FIX OWNERSHIP PROBLEM

                if (is_processed == false) {
                    DBG_ERROR("Thread (%llu) failed to process packet");
                    close_client(per_socket_context, false);
                }
            }

            // Post receive
            WSARecv(per_socket_context->socket, &(per_socket_context->io_context->wsabuf),
                       1, NULL, &flags, &(per_socket_context->io_context->overlapped), NULL);
            break;

        case IO_OP_WRITE:
            DBG_DEBUG("Thread(%llu) sent %d bytes\n", GetCurrentThreadId(), io_size);
            per_io_context->sent_bytes += io_size;
            flags = 0;

            if (per_io_context->sent_bytes < per_io_context->total_bytes) {
                // Previous IO operation sent not all bytes, post another WSASend
                per_io_context->wsabuf.buf = per_io_context->buffer + per_io_context->sent_bytes;
                per_io_context->wsabuf.len = per_io_context->total_bytes - per_io_context->sent_bytes;

                n_ret = WSASend(per_socket_context->socket, &per_io_context->wsabuf, 1,
                               NULL, flags, &per_io_context->overlapped, NULL);
                if (n_ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                    DBG_ERROR("Thread(%llu) WSASend failed\n", GetCurrentThreadId(), WSAGetLastError());
                    close_client(per_socket_context, false);
                    delete_io_context_from_socket_context(per_socket_context, per_io_context);
                } else {
                    DBG_DEBUG("Thread(&llu) WSASend partially completed (%d bytes), WSASend posted\n", GetCurrentThreadId(), io_size);
                }
            } else {
                // Previous write operation completed
                DBG_DEBUG("Thread(%llu) send completed\n", GetCurrentThreadId(), io_size);
                delete_io_context_from_socket_context(per_socket_context, per_io_context);
            }
            break;

        case IO_OP_ACCEPT:
            DBG_DEBUG("Thread(%llu) accepting new connection\n", GetCurrentThreadId());
            SOCKET accept_socket = per_socket_context->accept_socket;
            if (accept_socket == INVALID_SOCKET) {
                DBG_FATAL("acceptex() failed to create accept_socket: %d", GetLastError());
                exit(1);
            }

            // Add new socket to the IOCP
            // allocate and add context data to global list of context structures
            PerSocketContext *new_connection_socket_context = update_completion_port(accept_socket, IO_OP_READ, true);
            if (new_connection_socket_context == NULL) {
                DBG_FATAL("update_completion_port() failed to update completion port: %d", GetLastError());
                exit(1);
            }

            // Post initial receive on new socket
            n_ret = WSARecv(accept_socket, &(new_connection_socket_context->io_context->wsabuf),
                           1, NULL, &flags,
                           &(new_connection_socket_context->io_context->overlapped), NULL);
            if (n_ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                DBG_FATAL("WSARecv() failed: %d", WSAGetLastError());
                close_client(new_connection_socket_context, false);
            }

            // SOCKET for upcoming connection
            SOCKET new_connection_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            // Post next accept
            per_socket_context->accept_socket = new_connection_socket;
            bool ret_val = acceptex(g_listen_socket, new_connection_socket, per_socket_context->io_context->buffer,
                                    0, sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
                                    &per_socket_context->io_context->total_bytes, &per_socket_context->io_context->overlapped);
            if (ret_val == false && WSAGetLastError() != WSA_IO_PENDING) {
                DBG_FATAL("Failed to post acceptex! Error %d", WSAGetLastError());
                log_wsa_error(WSAGetLastError());
                exit(1);
            }
            break;
        default:
            DBG_FATAL("UNKNOWN IO OPERATION");
            close_client(per_socket_context, false);
            break;
        }
    }

    return 0;
}

static bool process_packet(TlPacket *tl_packet, PerSocketContext *per_socket_context)
{
    bool success = false;
    switch (tl_packet->command) {
    case CMD_LOGIN:
        DBG_DEBUG("Thread (%llu) received login packet", GetCurrentThreadId());
        // Process received packet
        ServerRespondPacket *respond_packet = process_login_packet(tl_packet, per_socket_context);

        // Pack respond packet
        TlPacket *serialized = serialize_server_respond_packet(respond_packet);
        delete_server_respond_packet(respond_packet);

        bool is_sent = send_to_client(per_socket_context, serialized);
        delete_tl_packet(serialized);

        if (is_sent == false) {
            DBG_ERROR("Thread(%llu) WSASend failed error(%d)", GetCurrentThreadId(), WSAGetLastError());
            success = false;
        } else {
            success = true;
        }
        break;
    default:
        success = false;
    }
    return success;
}

bool send_to_client(PerSocketContext *per_socket_context, TlPacket *packet)
{
    // TODO: convert data to network order before send
    EnterCriticalSection(per_socket_context->send_critical_section);

    PerIoContext *send_io_context = allocate_io_context();
    send_io_context->io_operation = IO_OP_WRITE;
    send_io_context->total_bytes = packet->size;
    send_io_context->sent_bytes = 0;
    send_io_context->wsabuf.buf = send_io_context->buffer;
    send_io_context->wsabuf.len = packet->size;
    memcpy(send_io_context->buffer, packet->data, packet->size);

    add_io_context_to_socket_context(per_socket_context, send_io_context);
    if (per_socket_context->send_in_progress == false) {
        per_socket_context->send_in_progress = true;

        int n_ret = WSASend(per_socket_context->socket, &send_io_context->wsabuf, 1,
                               NULL, 0, &send_io_context->overlapped, NULL);
        if (n_ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
            DBG_ERROR("Thread(%llu) WSASend failed\n", GetCurrentThreadId(), WSAGetLastError());
            delete_io_context_from_socket_context(per_socket_context, send_io_context);
            return false;
        }
    }

    LeaveCriticalSection(per_socket_context->send_critical_section);
    return true;
}

PerSocketContext *update_completion_port(SOCKET socket, IoOperation client_io, bool add_to_list)
{
    DBG_FUNC();
    PerSocketContext *per_socket_context = NULL;

    per_socket_context = allocate_socket_context(socket, client_io);
    if (per_socket_context == NULL) {
        return NULL;
    }

    g_iocp = CreateIoCompletionPort((HANDLE)socket, g_iocp, (DWORD_PTR)per_socket_context, 0);
    if (g_iocp == INVALID_HANDLE_VALUE) {
        DBG_FATAL("CreateIoCompletionPort() failed to create completion port: %d", GetLastError());
        if (per_socket_context->io_context != NULL) {
            free(per_socket_context->io_context);
        }
        free(per_socket_context);
        return NULL;
    }

    if (add_to_list)
        add_to_socket_context_list(per_socket_context);

    DBG_INFO("update_completion_port: socket(%d) added to IOCP\n", per_socket_context->socket);

    return per_socket_context;
}

PerSocketContext *allocate_socket_context(SOCKET socket, IoOperation client_io)
{
    DBG_FUNC();
    PerSocketContext *per_socket_context = NULL;

    EnterCriticalSection(g_critical_section);
    per_socket_context = malloc(sizeof(*per_socket_context));
    if (per_socket_context != NULL) {
        per_socket_context->io_context = malloc(sizeof(*per_socket_context->io_context));
        if (per_socket_context->io_context != NULL) {
            per_socket_context->socket = socket;
            per_socket_context->accept_socket = INVALID_SOCKET;
            per_socket_context->ctxt_back = NULL;
            per_socket_context->ctxt_forward = NULL;
            per_socket_context->nickname = NULL;
            per_socket_context->accept_socket = INVALID_SOCKET;
            initialize_critical_section(&per_socket_context->send_critical_section);
            per_socket_context->send_in_progress = false;

            per_socket_context->io_context->overlapped.Internal = 0;
            per_socket_context->io_context->overlapped.InternalHigh = 0;
            per_socket_context->io_context->overlapped.Offset = 0;
            per_socket_context->io_context->overlapped.OffsetHigh = 0;
            per_socket_context->io_context->overlapped.hEvent = NULL;
            per_socket_context->io_context->io_operation = client_io;
            per_socket_context->io_context->io_context_forward = NULL;
            per_socket_context->io_context->total_bytes = 0;
            per_socket_context->io_context->sent_bytes  = 0;
            per_socket_context->io_context->wsabuf.buf  = per_socket_context->io_context->buffer;
            per_socket_context->io_context->wsabuf.len  = sizeof(per_socket_context->io_context->buffer);

            ZeroMemory(per_socket_context->io_context->wsabuf.buf, per_socket_context->io_context->wsabuf.len);
        } else {
            DBG_FATAL("malloc PerIoContext failed");
        }
    } else {
        DBG_FATAL("malloc PerSocketContext failed");
    }

    LeaveCriticalSection(g_critical_section);

    return per_socket_context;
}

void close_client(PerSocketContext *per_socket_context, bool graceful)
{
    DBG_FUNC();
    EnterCriticalSection(g_critical_section);

    if (per_socket_context != NULL) {
        DBG_DEBUG("close_client: socket(%d) connection closing (graceful=%s)\n",
            per_socket_context->socket, graceful ? "true" : "false");
        if (!graceful) {
            // force subsequent closesocket to be abortative
            LINGER linger;
            linger.l_onoff = 1;
            linger.l_linger = 0;
            setsockopt(per_socket_context->socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
        }
        closesocket(per_socket_context->socket);
        per_socket_context->socket = INVALID_SOCKET;
        if (per_socket_context->accept_socket != INVALID_SOCKET) {
            closesocket(per_socket_context->accept_socket);
            per_socket_context->accept_socket = INVALID_SOCKET;
        }
        if (per_socket_context->nickname)
            free(per_socket_context->nickname);
        delete_from_socket_context_list(per_socket_context);
        per_socket_context = NULL;
    } else {
        DBG_DEBUG("per_socket_context is NULL\n");
    }

    LeaveCriticalSection(g_critical_section);
}

void add_to_socket_context_list(PerSocketContext *per_socket_context)
{
    PerSocketContext *temp = NULL;

    EnterCriticalSection(g_critical_section);

    if (g_clients == NULL) {
        // Add first node to the list
        per_socket_context->ctxt_back = NULL;
        per_socket_context->ctxt_forward = NULL;
        g_clients = per_socket_context;
    } else {
        // Add node to head of list
        temp = g_clients;

        g_clients = per_socket_context;
        per_socket_context->ctxt_back = temp;
        per_socket_context->ctxt_forward = NULL;

        temp->ctxt_forward = per_socket_context;
    }

    LeaveCriticalSection(g_critical_section);
}

void delete_from_socket_context_list(PerSocketContext *per_socket_context)
{
    PerSocketContext    *back;
    PerSocketContext    *forward;
    PerIoContext        *next_io = NULL;
    PerIoContext        *temp_io = NULL;

    EnterCriticalSection(g_critical_section);

    if (per_socket_context != NULL) {
        back = per_socket_context->ctxt_back;
        forward = per_socket_context->ctxt_forward;

        if ((back == NULL) && (forward == NULL)) {
            // This is the only node in the list
            g_clients = NULL;
        } else if ((back == NULL) && (forward != NULL)) {
            // Start node
            forward->ctxt_back = NULL;
            g_clients = forward;
        } else if ((back != NULL) && (forward == NULL)) {
            // End node
            back->ctxt_forward = NULL;
        } else if (back && forward) {
            back->ctxt_forward = forward;
            forward->ctxt_back = back;
        }

        // Free all IO context structs per socket
        temp_io = per_socket_context->io_context;
        do {
            next_io = temp_io->io_context_forward;
            if (temp_io) {
                // Overlapped structure is safe to delete only when
                // posted IO has completed. This only need to be tested only
                // in the shutdown process.
                if (g_shut_down)
                    while (!HasOverlappedIoCompleted((LPOVERLAPPED)&temp_io)) Sleep(0);
                free(temp_io);
                temp_io = NULL;
            }
            temp_io = next_io;
        } while (next_io != NULL);
        free(per_socket_context);
        per_socket_context = NULL;
    } else {
        DBG_WARNING("per_socket_context is NULL\n");
    }

    LeaveCriticalSection(g_critical_section);
}

void delete_io_context(PerIoContext *per_io_context)
{
    if (g_shut_down)
        while (!HasOverlappedIoCompleted((LPOVERLAPPED)&per_io_context)) Sleep(0);
    free(per_io_context);
}

void add_io_context_to_socket_context(PerSocketContext *per_socket_context, PerIoContext *io_context)
{
    PerIoContext *tail = NULL, *head = per_socket_context->io_context;

    while (head != NULL) {
        tail = head;
        head = head->io_context_forward;
    }
    if (tail == NULL) {
        DBG_FATAL("Socket has no IO context!");
        free(io_context);
        exit(1);
    }
    tail->io_context_forward = io_context;
}

void delete_io_context_from_socket_context(PerSocketContext *per_socket_context, PerIoContext *io_context)
{
    PerIoContext *it = per_socket_context->io_context, *tail = NULL;

    while (it != io_context && it != NULL) {
        tail = it;
        it = it->io_context_forward;
    }
    if (it == NULL) {
        DBG_FATAL("Socket has no such IO context!");
        exit(1);
    }
    if (tail == NULL) {
        DBG_FATAL("Attempted to delete the primary IO(incoming) context!");
        exit(1);
    }

    tail->io_context_forward = it->io_context_forward;
    delete_io_context(io_context);
}

PerIoContext *allocate_io_context()
{
    PerIoContext *per_io_context = NULL;
    per_io_context = malloc(sizeof(*per_io_context));
    ZeroMemory(per_io_context, sizeof(*per_io_context));
    return per_io_context;
}

void free_socket_context_list()
{
    PerSocketContext *temp1, *temp2;

    EnterCriticalSection(g_critical_section);

    temp1 = g_clients;

    while(temp1) {
        temp2 = temp1->ctxt_back;
        close_client(temp1, FALSE);
        temp1 = temp2;
    }

    LeaveCriticalSection(g_critical_section);
}

static void signal_handler(int signal_code)
{
    switch (signal_code) {
    case SIGABRT:
        DBG_INFO("Caught abort signal_code.\naborting...");
        break;
    case SIGSEGV:
        DBG_INFO("Caught segfault");
        break;
    case SIGTERM:
        DBG_INFO("Caught termination");
        break;
    default:
        DBG_INFO("Caught unknown signal_code");
    }
}
