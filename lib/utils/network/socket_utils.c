#include <socket_utils.h>
#include <debug.h> // DBG_ERROR ...
#include <winsock2.h> // closesocket 
#include <ws2tcpip.h> // SOCKET ...
#include <stdio.h>

PUBLIC SOCKET create_passive_socket(const char *port, const int socktype, const int family, const int backlog)
{
	struct addrinfo hints, *bind_address = NULL;
	SOCKET socket_listen = INVALID_SOCKET;
	int error_code; // For storing error code from WSAGetLastError()
	
	DBG_ERROR("Configuring server address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	hints.ai_family = family;
	hints.ai_flags = AI_PASSIVE; // Socket will be used in bind()
	if (getaddrinfo(NULL, port, &hints, &bind_address) != 0) {
		DBG_ERROR("getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	DBG_DEBUG("Creating socket...\n");
	if ((socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol)) == INVALID_SOCKET) {
		DBG_ERROR("socket() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	DBG_DEBUG("Binding socket...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
		DBG_ERROR("bind() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	freeaddrinfo(bind_address);
	bind_address = NULL;

	DBG_DEBUG("Placing socket in listen state...\n");
	if (listen(socket_listen, backlog) != 0) {
		DBG_ERROR("listen() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	if (socket_listen == INVALID_SOCKET) {
		DBG_ERROR("socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	DBG_INFO("Successfully created and binded socket\n");
	return socket_listen;

failure:
	if (bind_address != NULL)
		freeaddrinfo(bind_address);
	if (socket_listen != INVALID_SOCKET)
		closesocket(socket_listen);

	WSASetLastError(error_code); // Restoring proper error code

	return INVALID_SOCKET;
}





PUBLIC SOCKET create_active_socket(const char *host, const char *port, const int socktype)
{
	struct addrinfo hints, *peer_address = NULL;
	SOCKET socket_peer = INVALID_SOCKET;
	int error_code; // For storing error code from WSAGetLastError()

	DBG_DEBUG("Configuring remote address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	if (getaddrinfo(host, port, &hints, &peer_address) != 0) {
		DBG_ERROR("getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}

	DBG_DEBUG("Creating socket...\n");
	if ((socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, 
				    peer_address->ai_protocol)) == INVALID_SOCKET) {
		DBG_ERROR("socket() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}
	
	DBG_DEBUG("Connecting to server...\n");
	if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen) != 0) {
		DBG_ERROR("connect() failed. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}
	freeaddrinfo(peer_address);
	peer_address = NULL;

	if (socket_peer == INVALID_SOCKET) {
		DBG_ERROR("socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		error_code = WSAGetLastError();
		goto failure;
	}
	
	DBG_DEBUG("Successfully created and connected socket\n");
	return socket_peer;

failure:
	if (peer_address != NULL)
		freeaddrinfo(peer_address);
	if (socket_peer != INVALID_SOCKET)
		closesocket(socket_peer);

	WSASetLastError(error_code); // Restoring proper error code

	return INVALID_SOCKET;
}
