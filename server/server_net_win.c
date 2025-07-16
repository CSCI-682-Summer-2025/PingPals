#include "server_net.h"
#include <stdio.h>

// Link with the Winsock library on Windows
#pragma comment(lib, "ws2_32.lib")

/// @brief Initializes Winsock (required before using sockets on Windows).
/// @return 0 on success, non-zero on failure.
int net_init() {
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);  // Request version 2.2
}

/// @brief Cleans up Winsock resources.
void net_cleanup() {
    WSACleanup();
}

/// @brief Creates a TCP socket (IPv4, stream-based).
/// @return A SOCKET handle or INVALID_SOCKET on failure.
socket_t net_create_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

/// @brief Binds a socket to the specified port on all interfaces.
/// @param sock The socket handle.
/// @param port Port to bind to.
/// @return 0 = success or SOCKET_ERROR on failure.
int net_bind(socket_t sock, int port) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;               // IPv4
    server.sin_addr.s_addr = INADDR_ANY;       // Listen on all interfaces
    server.sin_port = htons(port);             // Host to network byte order
    return bind(sock, (struct sockaddr *)&server, sizeof(server));
}

/// @brief Puts the socket into listening mode for incoming connections.
/// @param sock The bound socket.
/// @return 0 on success, SOCKET_ERROR on failure.
int net_listen(socket_t sock) {
    return listen(sock, BACKLOG);
}

/// @brief Accepts an incoming client connection.
/// @param server_sock Listening socket.
/// @return A new socket for the accepted connection, or INVALID_SOCKET on error.
socket_t net_accept(socket_t server_sock) {
    struct sockaddr_in client;
    int c = sizeof(client);
    return accept(server_sock, (struct sockaddr *)&client, &c);
}

/// @brief Closes socket sock.
/// @param sock The socket to close.
/// @return 0 = success or SOCKET_ERROR on failure.
int net_close_socket(socket_t sock) {
    return closesocket(sock);
}
