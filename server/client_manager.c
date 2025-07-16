#include "client_manager.h"
#include <string.h>

#ifdef _WIN32
CRITICAL_SECTION client_lock;  // Windows critical section. Lock operations that need be thread safe.
#else
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;  // POSIX mutex
#endif

Client clients[MAX_CLIENTS];  // Global array connected clients
int client_count = 0;         // Current number active clients

/// @brief Lock the global client list for thread-safe access.
void lock_clients() {
#ifdef _WIN32
    EnterCriticalSection(&client_lock);
#else
    pthread_mutex_lock(&client_lock);
#endif
}

/// @brief Unlock the global client list.
void unlock_clients() {
#ifdef _WIN32
    LeaveCriticalSection(&client_lock);
#else
    pthread_mutex_unlock(&client_lock);
#endif
}

/// @brief Get the index of a client by its socket.
/// @param sock Socket to serach for.
/// @return Return index, or -1 if not found.
int get_client_index(socket_t sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == sock)
            return i;
    }
    return -1;
}

/// @brief Remove a client from the client with matching socket (sock)
///        Shifts remaining clients to fill the gap.
/// @param sock The socket of the client to remove.
void remove_client(socket_t sock) {
    int idx = get_client_index(sock);
    if (idx != -1) {
        for (int i = idx; i < client_count - 1; i++)
            clients[i] = clients[i + 1];  // Shift left
        client_count--;
    }
}

/// @brief Add a new client to the list if space and name is available.
/// @param sock Client socket, trying to get added.
/// @param username Client passed in username.
/// @return true =added successfully, false on failure (full or duplicate).
bool add_client(socket_t sock, const char* username) {
    if (client_count >= MAX_CLIENTS)
        return false;

    // Check for duplicate username
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            return false; // Username already taken
        }
    }

    // Add client to clients.
    clients[client_count].socket = sock;
    strncpy(clients[client_count].username, username, MAX_USERNAME);
    clients[client_count].username[MAX_USERNAME - 1] = '\0';  // Null-terminate
    clients[client_count].channel[0] = '\0';  // Start with no channel

    
    // Generate hash from the username to pick a color in the 216-color ANSI cube. Avoid system colors 0-15
    unsigned hash = 0;
    for (const char* p = username; *p; p++) {
        hash = (hash * 31) + (unsigned char)(*p);
    }
    int color_index = 16 + (hash % 216);
    snprintf(clients[client_count].color_code, MAX_COLOR, "\x1b[38;5;%dm", color_index);

    // Update client count, return. 
    client_count++;
    return true;
}

/// @brief Get the username associated with a socket.
/// @param sock Socket looking up username for.
/// @return Pointer to username string. Not found = "Unknown".
const char* get_client_name(socket_t sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == sock) {
            return clients[i].username;
        }
    }
    return "Unknown";
}
