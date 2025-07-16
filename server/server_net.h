#ifndef SERVER_NET_H
#define SERVER_NET_H

// Platform-specific includes and definitions
#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    typedef SOCKET socket_t;  // Windows socket type alias
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int socket_t;            // POSIX socket is just an int
    #define INVALID_SOCKET (-1)      // Define equivalent of INVALID_SOCKET on POSIX
#endif

// Server constants
#define SERVER_PORT 9090    // Default port server listens on
#define BACKLOG 10          // Max number of pending connections in listen queue

// Initialize networking (Windows = WSAStartup, POSIX not use it).
int net_init();

// Clean up networking (WSACleanup on Windows, POSIX not use it).
void net_cleanup();

// Create a socket (platform-abstracted).
socket_t net_create_socket();
int net_bind(socket_t sock, int port);
int net_listen(socket_t sock);
socket_t net_accept(socket_t server_sock);

// Close a socket (platform-abstracted).
int net_close_socket(socket_t sock);

#endif