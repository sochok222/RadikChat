#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <winsock2.h>

#define SOCKET_CLIENT 0
#define SOCKET_SERVER 1

/***********************************************************
 * Creates socket;                                         * 
 * Binds socket to local address if type is SOCKET_SERVER; *
 * Connects socket to server if type is SOCKET_CLIENT.     *
 * host and port must be null-terminated strings           *
 * Returns INVALID_SOCKET on failure                       *
 ***********************************************************/
SOCKET create_socket(const char *host, const char *port, int type);
