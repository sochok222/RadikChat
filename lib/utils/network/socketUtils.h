#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <winsock2.h>

/***********************************************************
 * Creates socket;                                         * 
 * host and port must be null-terminated strings;          *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
SOCKET createActiveSocket(const char *host, const char *port, const int socktype);

/***********************************************************
 * Creates socket;                                         * 
 * host and port must be null-terminated strings;          *
 * socktype must be SOCK_STREAM or SOCK_DGRAM;             *
 * family must be AF_INET or AF_INET6;                     *
 * backlog length of the queue of pending connections      *
 * Returns INVALID_SOCKET on failure.                      *
 ***********************************************************/
SOCKET createPassiveSocket(const char *port, const int socktype, const int family, const int backlog);

#endif /* SOCKET_UTILS_H */
