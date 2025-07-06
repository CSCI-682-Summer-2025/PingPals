#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "client_utils.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_INPUT 1024

SOCKET sockfd;

// Thread to receive messages from server
DWORD WINAPI receive_messages(LPVOID param) {
    char buffer[MAX_INPUT];
    int bytes_received;

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("\n[Server]: %s\n> ", buffer);
        fflush(stdout);
    }

    printf("\n Disconnected from server.\n");
    ExitThread(0);
}

int main() {
    WSADATA wsa;
    struct sockaddr_in server_addr;
    char buffer[MAX_INPUT];
    char formatted[MAX_INPUT];

    // 1. Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error: %d\n", WSAGetLastError());
        return 1;
    }

    // 2. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed. Error: %d\n", WSAGetLastError());
        return 1;
    }

    // 3. Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9090);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 4. Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed. Error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");
    char test[] = "HELLO FROM CLIENT";
    send(sockfd, test, strlen(test), 0);


    // 5. Start the receive thread
    HANDLE recv_thread = CreateThread(NULL, 0, receive_messages, NULL, 0, NULL);
    if (recv_thread == NULL) {
        printf("Failed to create receive thread. Error: %d\n", GetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // 6. Input loop
    while (1) {
        printf("> ");
        if (fgets(buffer, MAX_INPUT, stdin) == NULL) break;

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "/quit") == 0) {
            strcpy(formatted, "QUIT");
            send(sockfd, formatted, strlen(formatted), 0);
            break;
        }

        if (buffer[0] == '/') {
            if (parse_command(buffer, formatted, MAX_INPUT)) {
                send(sockfd, formatted, strlen(formatted), 0);
            } else {
                printf(" Invalid command format.\n");
            }
        } else {
            send(sockfd, buffer, strlen(buffer), 0);
        }
    }

    // 7. Cleanup
    closesocket(sockfd);
    WSACleanup();
    printf("Connection closed.\n");
    return 0;
}
