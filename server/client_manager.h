#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "server_net.h"
#include <stdio.h>
#include <stdbool.h>

#define MAX_CLIENTS 10  // maximum number of clients server supports.
#define MAX_CHANNEL_NAME 50 // maximum number of channels server suppors.
#define MAX_USERNAME 10 // maximum number of usernames 
#define MAX_COLOR 20 // Possible colors selectable for clients.
#define BUFFER_SIZE 1024 

// Main struct for storing a client. 
typedef struct {
    socket_t socket;
#ifdef _WIN32
    void* thread;  // opaque HANDLE or pthread_t not exposed here
#else
    void* thread;
#endif
    char username[MAX_USERNAME]; 
    char channel[MAX_CHANNEL_NAME];
    char color_code[MAX_COLOR];  // ANSI color escape code for console output. 
} Client;

// Client list and count. Used by both server_main and server_utils
extern Client clients[MAX_CLIENTS];
extern int client_count;

// Generates client_lock. Used to seal critical sections during multi-threading operations.
#ifdef _WIN32
extern CRITICAL_SECTION client_lock;
#else
#include <pthread.h>
extern pthread_mutex_t client_lock;
#endif



void lock_clients();
void unlock_clients();

int get_client_index(socket_t sock);
void remove_client(socket_t sock);
bool add_client(socket_t sock, const char* username);
const char* get_client_name(socket_t sock);

#endif
