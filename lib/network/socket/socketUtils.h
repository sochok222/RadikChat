#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <winsock2.h>

/***********************************************************
 * Creates socket;                                         * 
 * host and port must be null-terminated strings;          *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
SOCKET create_active_socket(const char *host, const char *port, int socktype);

/***********************************************************
 * Creates socket;                                         * 
 * port must be null-terminated strings;                   *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * family must be AF_INET or AF_INET6;                     *
 * backlog length of the queue of pending connections      *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
SOCKET create_passive_socket(const char *port, int socktype, int family, int backlog);

#endif /* SOCKET_UTILS_H */
