#include <network_manager.h>
#include <debug.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

PUBLIC SOCKET create_passive_socket(const char *port, const int socktype, const int family, const int backlog)
{
	struct addrinfo hints, *bind_address = NULL;
	SOCKET socket_listen;

	DEBUG(MSG, "Configuring server address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	hints.ai_family = family;
	hints.ai_flags = AI_PASSIVE; // Socket will be used in bind()
	if (getaddrinfo(NULL, port, &hints, &bind_address) != 0) {
		DEBUG(ERR, "getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}

	DEBUG(MSG, "Creating socket...\n");
	if ((socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol)) == INVALID_SOCKET) {
		DEBUG(ERR, "socket() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}

	DEBUG(MSG, "Binding socket...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
		DEBUG(ERR, "bind() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}
	freeaddrinfo(bind_address);
	bind_address = NULL;

	if (listen(socket_listen, backlog) != 0) {
		DEBUG(ERR, "listen() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}

	if (socket_listen == INVALID_SOCKET) {
		DEBUG(ERR, "socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}

	DEBUG(MSG, "Successfully created socket\n");
	return socket_listen;

failure:
	if (bind_address != NULL)
		freeaddrinfo(bind_address);
	if (socket_listen != INVALID_SOCKET)
		closesocket(socket_listen);
	return INVALID_SOCKET;
}





PUBLIC SOCKET create_active_socket(const char *host, const char *port, const int socktype)
{
	struct addrinfo hints, *peer_address = NULL;
	SOCKET socket_peer;

	DEBUG(MSG, "Configuring remote address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = socktype;
	if (getaddrinfo(host, port, &hints, &peer_address) != 0) {
		DEBUG(ERR, "getaddrinfo() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}

	DEBUG(MSG, "Creating socket...\\n");
	if ((socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, 
				    peer_address->ai_protocol)) == INVALID_SOCKET) {
		DEBUG(ERR, "socket() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}
	
	DEBUG(MSG, "Connecting to server...\n");
	if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen) != 0) {
		DEBUG(ERR, "connect() failed. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}
	freeaddrinfo(peer_address);
	peer_address = NULL;

	if (socket_peer == INVALID_SOCKET) {
		DEBUG(ERR, "socket is INVALID_SOCKET. Error code (%d)\n", WSAGetLastError());
		goto failure;
	}
	
	DEBUG(MSG, "Successfully created socket\n");
	return socket_peer;

failure:
	if (peer_address != NULL)
		freeaddrinfo(peer_address);
	if (socket_peer != INVALID_SOCKET)
		closesocket(socket_peer);
	return INVALID_SOCKET;
}
