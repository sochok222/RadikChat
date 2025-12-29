#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <winsock2.h>

#define PUBLIC
#define PRIVATE static

/***********************************************************
 * Creates socket;                                         * 
 * Connects socket to server if type is SOCKET_CLIENT;     *
 * host and port must be null-terminated strings;          *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
PUBLIC SOCKET create_client_socket(const char *host, const char *port, const int socktype);

/***********************************************************
 * Creates socket;                                         * 
 * Binds socket to local address if type is SOCKET_SERVER; *
 * host and port must be null-terminated strings;          *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * family must be AF_INET or AF_INET6;                     *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
PUBLIC SOCKET create_server_socket(const char *port, const int socktype, const int family, const int backlog);

#endif
