#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "server_net.h"
#include "client_manager.h"
#include "server_utils.h"

#ifdef _WIN32
    #include <windows.h>
    typedef HANDLE thread_t;
    #define THREAD_RETURN DWORD WINAPI
    #define THREAD_RET_VAL 0
#else
    #include <pthread.h>
    #include <unistd.h>
    typedef pthread_t thread_t;
    #define THREAD_RETURN void*
    #define THREAD_RET_VAL NULL
#endif

volatile bool server_running = true;

/// @brief Handle a single client connection.
///        Handles user registration, commands, messaging, and cleanup.
/// @param arg Pointer to socket_t (client socket).
/// @return Thread exit value.
THREAD_RETURN handle_client(void* arg) {
    socket_t sock = *(socket_t*)arg;
    free(arg); // arg not used after this. Free it.
    char buf[BUFFER_SIZE];

    // Step 1: Receive username from client
    int len = recv(sock, buf, BUFFER_SIZE - 1, 0);
    if (len <= 0) {
        // Connection closed or error, close socket and exit thread
        net_close_socket(sock);
        return THREAD_RET_VAL;
    }
    buf[len] = '\0';

    // Step 2: Clean username input (strip newline and carriage return)
    char* newline = strchr(buf, '\n');
    if (newline) *newline = '\0';
    char* carriage = strchr(buf, '\r');
    if (carriage) *carriage = '\0';

    // Step 3: Validate username format
    if (!is_valid_name(buf)) {
        send_message(sock, "Invalid username.\n");
        net_close_socket(sock);
        return THREAD_RET_VAL;
    }

    // Step 4: Attempt to add client to clients list
    lock_clients();
    bool added = add_client(sock, buf);
    unlock_clients();

    if (!added) {
        // Username taken or max clients reached.
        send_message(sock, "Username already taken.\n");
        shutdown(sock, SHUT_WR);
    // Add pause.
    #ifdef _WIN32
        Sleep(250);
    #else
        usleep(250000);
    #endif
        net_close_socket(sock);
        return THREAD_RET_VAL;
    }

    // Step 5: Main client message handling loop.
    while ((len = recv(sock, buf, BUFFER_SIZE - 1, 0)) > 0) {
        buf[len] = '\0';
        printf("[DEBUG] Full input buffer: '%s'\n", buf);
        
        // Clean trailing newlines/carriage returns.
        newline = strchr(buf, '\n');
        if (newline) *newline = '\0';
        carriage = strchr(buf, '\r');
        if (carriage) *carriage = '\0';


        // Extract command word (up to space or 15 characters).
        char command[16] = {0};
        int i = 0;
        while (buf[i] && buf[i] != ' ' && i < 15) {
            command[i] = buf[i];
            i++;
        }
        command[i] = '\0';

        // Convert command to uppercase. Allows for comparison.
        for (int j = 0; j < i; j++) {
            command[j] = (char)toupper((unsigned char)command[j]);
        }

        printf("Command: %s\n", command);

        // Extract command arguments if any.
        char* message_text = NULL;
        if (buf[i] == ' ') {
            message_text = buf + i + 1;
        }

        // Step 6: Process recognized commands.
        if (strcmp(command, "QUIT") == 0) {
            // Client requested to disconnect
            const char* name = get_client_name(sock);
            if (name) {
                printf("Client user '%s' disconnected.\n", name);
            } else {
                printf("A client requested disconnect--unknown username.\n");
            }
            break;
        } else {
            // All other commands, call utility method.
            dispatch_command(sock, command, message_text);
        }
    }

    // Step 7: Handle client disconnect.
    if (len == 0) {
        // Client closed connection normally
    } else if (len < 0) {
        perror("recv failed");
    }

    // Step 8: Remove client from other clients.
    lock_clients();
    remove_client(sock);
    unlock_clients();

    net_close_socket(sock);
    return THREAD_RET_VAL;
}


/// @brief Monitor for main shutdown command. Q + enter
///        Typing 'q' or 'Q' + Enter will start shutdown.
/// @param arg Unused.
/// @return Thread exit value.
THREAD_RETURN server_control_thread(void* arg) {
    (void)arg;
    char cmd[10];
    printf("Type 'q' and Enter to quit server.\n");
    while (fgets(cmd, sizeof(cmd), stdin)) {
        if (cmd[0] == 'q' || cmd[0] == 'Q') {
            printf("Shutdown command received.\n");
            server_running = false;

            // Connecting to the dummy socket to unblocks accept()

            socket_t dummy = net_create_socket();
            if (dummy != INVALID_SOCKET) {
                struct sockaddr_in server_addr = {
                    .sin_family = AF_INET,
                    .sin_port = htons(SERVER_PORT),
                    .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
                };
                connect(dummy, (struct sockaddr*)&server_addr, sizeof(server_addr));
                net_close_socket(dummy);
            }
            break;
        }
    }
    return THREAD_RET_VAL;
}

/// @brief Main server entry point.
///        Initializes networking, listens for connections, starts client handler threads.
int main() {
    // Step 1: Initialize networking (Winsock on Windows, posix doesn't use it.)
    net_init();

#ifdef _WIN32
    InitializeCriticalSection(&client_lock);
#endif

    // Step 2: Create server socket
    socket_t server = net_create_socket();
    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        return 1;
    }

    // Step 3: Bind server socket to port. Start listening.
    if (net_bind(server, SERVER_PORT) < 0 || net_listen(server) < 0) {
        fprintf(stderr, "Bind/listen failed.\n");
        net_close_socket(server);
        net_cleanup();
        return 1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    // Step 4: Start console control thread for shutdown
    thread_t ctrl_thread;
#ifdef _WIN32
    ctrl_thread = CreateThread(NULL, 0, server_control_thread, NULL, 0, NULL);
#else
    pthread_create(&ctrl_thread, NULL, server_control_thread, NULL);
#endif

    // Step 5: Main server loop.
    while (server_running) {
        socket_t* client_ptr = malloc(sizeof(socket_t));
        *client_ptr = net_accept(server);
        if (*client_ptr == INVALID_SOCKET) {
            printf("Net_accept returned INVALID_SOCKET\n");
            free(client_ptr);
            continue;
        }

        printf("Accepted new connection.\n");

        thread_t tid;
#ifdef _WIN32
        tid = CreateThread(NULL, 0, handle_client, client_ptr, 0, NULL);
#else
        pthread_create(&tid, NULL, handle_client, client_ptr);
        pthread_detach(tid);
#endif
    }

    // Step 6: Shutdown sequence.
    printf("Server shutting down.\n");

    net_close_socket(server);
    net_cleanup();

#ifdef _WIN32
    DeleteCriticalSection(&client_lock);
#endif

    return 0;
}
