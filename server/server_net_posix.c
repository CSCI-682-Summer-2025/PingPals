#include "server_net.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/// @brief Initializes networking. Not needed 
/// for POSIX.
int net_init() {
    return 0; // Not needed for POSIX systems
}

/// @brief Cleans up networking (no-op on POSIX).
/// Not used for POSIX.
void net_cleanup() {
}

/// @brief Creates a TCP socket (IPv4, stream-based).
socket_t net_create_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

/// @brief Binds a socket to the given port on all interfaces.
/// @param sock Socket file descriptor.
/// @param port Port to bind to.
/// @return 0 on success, -1 on error.
int net_bind(socket_t sock, int port) {
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));            // Zero out the struct.
    server.sin_family = AF_INET;                   // Set to IPv4.
    server.sin_addr.s_addr = INADDR_ANY;           // Bind to all local interfaces.
    server.sin_port = htons(port);                 // Convert port network byte order.
    return bind(sock, (struct sockaddr *)&server, sizeof(server));
}

/// @brief Puts the socket into listening mode. Allows accept connections.
/// @param sock The bound socket.
/// @return 0 on success, -1 on error.
int net_listen(socket_t sock) {
    return listen(sock, BACKLOG); 
}

/// @brief Accepts new client connection.
/// @param server_sock Listening socket.
/// @return A new socket for the accepted connection, or INVALID_SOCKET on error.
socket_t net_accept(socket_t server_sock) {
    struct sockaddr_in client;
    socklen_t c = sizeof(client);
    return accept(server_sock, (struct sockaddr *)&client, &c);
}

/// @brief Closes socket, providing output. 
/// @param sock The socket to close.
/// @return 0 on success, -1 on error.
int net_close_socket(socket_t sock) {
    return close(sock);
}
