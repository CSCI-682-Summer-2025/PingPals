#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
#endif

#include "client_utils.h"
#include "../shared/command_parser.h"

// OS sepecific typedefs and macros.
#ifdef _WIN32
    typedef HANDLE thread_t;
    #define SHUT_RDWR SD_BOTH
    #define CLOSESOCKET closesocket
    SOCKET sockfd;
#else
    typedef pthread_t thread_t;
    #define CLOSESOCKET close
    int sockfd;
#endif

// Constants
#define MAX_INPUT 1024
#define MAX_CHANNELS 5
#define MAX_NAME_LEN 50

// Global variables
char username[MAX_NAME_LEN];
char current_channels[MAX_CHANNELS][MAX_NAME_LEN];
int channel_count = 0;



/// @brief Update channels based on command.
/// @param cmd Leave or Join command. 
/// @param args  Stores which channels need to be joined.
static void update_channel_list(command_t cmd, const char *args) {
    switch (cmd) {
    case CMD_JOIN:
        if (args && *args && channel_count < MAX_CHANNELS) {
            strncpy(current_channels[channel_count], args, MAX_NAME_LEN - 1);
            current_channels[channel_count][MAX_NAME_LEN - 1] = '\0';
            channel_count++;
        }
        break;
    case CMD_LEAVE:
        if (channel_count > 0) channel_count--;
        break;
    default:
        break;
    }
}


/// @brief Cleanly discconnect from server. Close resources, exit program. 
/// @param recv_thread Handle or ID of receive thread, wait for and clean.
void disconnect_cleanly(thread_t recv_thread) {
#ifdef _WIN32
    // If receive thread handle provided, wait for finish, then close.
    if (recv_thread) {
        WaitForSingleObject(recv_thread, INFINITE);
        CloseHandle(recv_thread);
    }

    // Close socket and Winsock resources.
    CLOSESOCKET(sockfd);
    WSACleanup();
#else
    // For POSIX, join receive thread if provided, ensure clean termination.
    if (recv_thread) pthread_join(recv_thread, NULL);
    CLOSESOCKET(sockfd);
#endif
    // Output successful close, and exit.
    printf("Connection closed.\n");
    exit(0);
}

/// @brief Signal for SIGNINT (Ctrl+C) gracefully disconnect from server.
/// @param sig Signal number (unused.)
void handle_sigint(int sig) {
    (void)sig; // suppress unused parameter warning (makefile)
    
    printf("\nCaught Ctrl+C. Disconnecting...\n");
    
    // Send quit message to server notify about disconnect.
    const char* quit_msg = "QUIT";
    safe_send(sockfd, quit_msg);

    // Shutdown socket sending and receiving, clean up resources.
    shutdown(sockfd, SHUT_RDWR);
    disconnect_cleanly(0);
}



/// @brief Core loop for receiving messages from the server and displaying them.
///        Runs until the connection closed or error occurs.
static void receive_loop(void) {
    char buffer[MAX_INPUT];
    int bytes_received;
    char ts[10];

    // Keep receiving data from the socket until closed or error.
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate received data.

        get_timestamp(ts, sizeof(ts));  // Get current timestamp [HH:MM:SS].

        // Check if the message is from the current user (self).
        if (is_self_message(buffer, username)) {
            // Print self messages in green.
            printf("\n%s \x1b[32m%s\x1b[0m\n", ts, buffer);
        } else {
            // Print other users' messages normally
            printf("\n%s %s\n", ts, buffer);
        }

        // Update the prompt to show username and joined channels.
        printf("\33[2K\r");
        print_user_channels_prompt(username, current_channels, channel_count);
    }

    // If recv returns 0 / negative, connection is closed / errored.
    printf("\nDisconnected from server.\n");
}

#ifdef _WIN32
/// @brief Entry point for receiving (Windows version).
/// @param param Unused. Required by signature.
/// @return DWORD exit code (always 0).
DWORD WINAPI receive_messages(LPVOID param) {
    (void)param;  // Suppress unused parameter warning

    receive_loop();

    ExitThread(0);
}
#else
/// @brief Thread entry point for receiving (POSIX (Unix, Mac) version).
/// @param param Unused. Required by signature.
/// @return Always NULL.
void* receive_messages(void* param) {
    (void)param;  // Suppress unused parameter warning

    receive_loop();

    pthread_exit(NULL);
}
#endif





int main() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // Initialize Winsock version 2.2. Must be called before using Winsock functions.
        printf("WSAStartup failed. Error: %d\n", WSAGetLastError()); // end program if fails.
        return 1;
    }
#endif

    // Set up handler for Ctrl+C to cleanly disconnect
    signal(SIGINT, handle_sigint);

    struct sockaddr_in server_addr;
    char buffer[MAX_INPUT];
    //char formatted_cmd[MAX_INPUT]; // Holds formatted_cmd version of user input before sending to server.

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed. Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
#else
    if (sockfd < 0) { // Negative, is invalid id.
        perror("Socket creation failed");
        return 1;
    }
#endif

    // Initialize server address struct.
    server_addr.sin_family = AF_INET;   // Set IPV4 type address family.
    server_addr.sin_port = htons(9090); // Set port #. Convert from host byte to network byte order.
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Set IP address. Convert dotted string -> binary.

    // Get username input from user.
    printf("Enter your username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        printf("Failed to read username.\n");
        return 1;
    }
    // Remove trailing newline from username input.
    username[strcspn(username, "\n")] = '\0';

    // Attempt connection to the server.
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        printf("Connection failed. Error: %d\n", WSAGetLastError());
        CLOSESOCKET(sockfd);
        WSACleanup();
#else
        perror("Connection failed");
        CLOSESOCKET(sockfd);
#endif
        return 1;
    }

    // Send username as initial identification to server
    safe_send(sockfd, username);

    // Create thread to receive messages asynchronously.
    // Runs receive_messages. Listenes messages from server.
    thread_t recv_thread;
#ifdef _WIN32
    recv_thread = CreateThread(NULL, 0, receive_messages, NULL, 0, NULL);
    if (recv_thread == NULL) {
        printf("Failed to create receive thread. Error: %d\n", GetLastError());
        CLOSESOCKET(sockfd);
        WSACleanup();
        return 1;
    }
#else
    // Create posix systems thread. 
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Failed to create receive thread");
        CLOSESOCKET(sockfd);
        return 1;
    }
#endif

    // Made it to this point, then successful. Output so user knows can continue inputting.
    print_user_channels_prompt(username, current_channels, channel_count);

    // Main loop. Read user input, process commands, send messages.
    while (1) {
        

        // Check for EOF or error reading input. 
        if (fgets(buffer, MAX_INPUT, stdin) == NULL) {
            
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';  // Strip newline

        // Gets command types. Still send entire buffer to server.
        char args[MAX_INPUT];
        command_t cmd = parse_command(buffer, args, sizeof(args));
        

        // Handle quit command.
        if (cmd == CMD_QUIT) {
            safe_send(sockfd, buffer);
            shutdown(sockfd, SHUT_RDWR);
            break;
        }

        /* Local UI maintenance */
        update_channel_list(cmd, args);

        // Notify user if command is invalid.
        if (cmd == CMD_INVALID) {
            printf("Invalid command.\n");
        } else if (cmd == CMD_UNKNOWN) {
            printf("Unknown command.\n");
        }

        // Send buffer to server.
        safe_send(sockfd, buffer);

        /* Reprint prompt */
        print_user_channels_prompt(username, current_channels, channel_count);

        
    }

    // Clean up: close connection and wait for receiver thread to finish
    disconnect_cleanly(recv_thread);
}