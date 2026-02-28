#include <Debug.h> // DBG_ERROR ...
#include <SocketUtils.h>
#include <stdio.h>
#include <winsock2.h> // closesocket
#include <ws2tcpip.h> // SOCKET ...

#define PUBLIC
#define PRIVATE static

PUBLIC SOCKET createPassiveSocket(const TCHAR *port, const int socktype, const int family, const int backlog)
{
	struct addrinfo hints, *bindAddress = NULL;
	SOCKET socketListen = INVALID_SOCKET;
	int error_code; // For storing error code from WSAGetLastError()
	
    DBG_INFO("Configuring server address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	hints.ai_family = family;
	hints.ai_flags = AI_PASSIVE; // Socket will be used in bind()
	if (getaddrinfo(NULL, port, &hints, &bindAddress) != 0) {
                DBG_ERROR("getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

    DBG_DEBUG("Creating socket...\n");
	if ((socketListen = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol)) == INVALID_SOCKET) {
                DBG_ERROR("socket() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

    DBG_DEBUG("Binding socket...\n");
	if (bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen) != 0) {
                DBG_ERROR("bind() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	freeaddrinfo(bindAddress);
	bindAddress = NULL;

    DBG_DEBUG("Placing socket in listen state...\n");
	if (listen(socketListen, backlog) != 0) {
                DBG_ERROR("listen() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	if (socketListen == INVALID_SOCKET) {
                DBG_ERROR("socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

        DBG_INFO("Successfully created and binded socket\n");
	return socketListen;

failure:
	if (bindAddress != NULL)
		freeaddrinfo(bindAddress);
	if (socketListen != INVALID_SOCKET)
		closesocket(socketListen);

	WSASetLastError(error_code); // Restoring proper error code

	return INVALID_SOCKET;
}

PUBLIC SOCKET createActiveSocket(const TCHAR *host, const TCHAR *port, const int socktype)
{
	struct addrinfo hints, *peerAddress = NULL;
	SOCKET socketPeer = INVALID_SOCKET;
	int errorCode; // For storing error code from WSAGetLastError()

    DBG_DEBUG("Configuring remote address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	if (getaddrinfo(host, port, &hints, &peerAddress) != 0) {
                DBG_ERROR("getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		errorCode = WSAGetLastError();
		goto failure;
	}

    DBG_DEBUG("Creating socket...\n");
	if ((socketPeer = socket(peerAddress->ai_family, peerAddress->ai_socktype,
				    peerAddress->ai_protocol)) == INVALID_SOCKET) {
                DBG_ERROR("socket() failed. Error code (%d)\n", WSAGetLastError());
		errorCode = WSAGetLastError();
		goto failure;
	}
	
    DBG_DEBUG("Connecting to server...\n");
	if (connect(socketPeer, peerAddress->ai_addr, peerAddress->ai_addrlen) != 0) {
                DBG_ERROR("connect() failed. Error code (%d)\n", WSAGetLastError());
		errorCode = WSAGetLastError();
		goto failure;
	}
	freeaddrinfo(peerAddress);
	peerAddress = NULL;

	if (socketPeer == INVALID_SOCKET) {
                DBG_ERROR("socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		errorCode = WSAGetLastError();
		goto failure;
	}
	
    DBG_DEBUG("Successfully created and connected socket\n");
	return socketPeer;

failure:
	if (peerAddress != NULL)
		freeaddrinfo(peerAddress);
	if (socketPeer != INVALID_SOCKET)
		closesocket(socketPeer);

	WSASetLastError(errorCode); // Restoring proper error code

	return INVALID_SOCKET;
}
