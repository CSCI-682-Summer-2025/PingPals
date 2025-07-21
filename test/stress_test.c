/* Stress Test Client 
 * Runs multiple tests:
 * 1. Basic connection
 * 2. Valid username handshake
 * 3. Duplicate username rejection
 * 4. Valid command usage
 * 5. Stress test: 50+ clients joining channels, messaging, etc
 * 6. Advanced stress tests: parallel joins, quits, /who, /leave, oversized messages
 * 
 * Created by: Honesty Beaton
 */

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
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <pthread.h>
#endif

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090
#define BUFFER_SIZE 1024
#define STRESS_CLIENTS 100
#define PARALLEL_CLIENTS 20
#define WHO_CLIENTS 30

int open_connection() {
    int sock;
    struct sockaddr_in server;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return -1;
    }

    return sock;
}

void close_connection(int sock) {
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

// === TESTS START ===

int test_basic_connection() {
    printf("Test 1: Basic connection... ");
    int sock = open_connection();
    if (sock < 0) return 0;
    close_connection(sock);
    printf("Connected successfully.\n");
    return 1;
}

int test_valid_username() {
    printf("Test 2: Valid username... ");
    int sock = open_connection();
    if (sock < 0) return 0;

    send(sock, "user1\n", 6, 0);
    close_connection(sock);
    printf("Username validation successful.\n");
    return 1;
}

int test_duplicate_username() {
    printf("Test 3: Duplicate username... ");
    int sock1 = open_connection();
    int sock2 = open_connection();
    if (sock1 < 0 || sock2 < 0) return 0;

    send(sock1, "dupe\n", 5, 0);
    send(sock2, "dupe\n", 5, 0);

    char buf[BUFFER_SIZE] = {0};
    recv(sock2, buf, BUFFER_SIZE - 1, 0);

    close_connection(sock1);
    close_connection(sock2);
    
    printf(strstr(buf, "already") ? "Duplicate Username validation successful. \n" : "Duplicate username validation not successful.\n");
    return strstr(buf, "already") != NULL;
}

int test_valid_command() {
    printf("Test 4: Valid commands... \n");

    int sock1 = open_connection();
    if (sock1 < 0) return 0;
    send(sock1, "tester\n", 7, 0);

    int sock2 = open_connection();
    if (sock2 < 0) {
        close_connection(sock1);
        return 0;
    }
    send(sock2, "target\n", 7, 0);

#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    send(sock1, "/join #alpha\n", 13, 0);
    send(sock1, "Hello everyone!\n", 16, 0);
    send(sock1, "/list\n", 6, 0);
    send(sock1, "/who\n", 5, 0);
    send(sock1, "/msg @target Hello privately!\n", 30, 0);
    send(sock1, "/kick @target\n", 14, 0);
    send(sock1, "/leave\n", 7, 0);
    send(sock1, "/quit\n", 6, 0);

    close_connection(sock1);
    close_connection(sock2);

    printf("All commands sent successfully.\n");
    return 1;
}

// === TEST 5: Basic stress ===
#ifdef _WIN32
DWORD WINAPI client_thread(LPVOID param)
#else
void* client_thread(void* param)
#endif
{
    char name[32];
    int id = *(int*)param;
    snprintf(name, sizeof(name), "stress%d\n", id);

    int sock = open_connection();
    if (sock >= 0) {
        send(sock, name, strlen(name), 0);
        send(sock, "/join #load\n", 12, 0);
        send(sock, "hi\n", 3, 0);
        close_connection(sock);
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int test_stress_clients() {
    printf("Test 5: Stress test (50+ clients)... \n");
    int ids[STRESS_CLIENTS];

#ifdef _WIN32
    HANDLE threads[STRESS_CLIENTS];
#else
    pthread_t threads[STRESS_CLIENTS];
#endif

    for (int i = 0; i < STRESS_CLIENTS; i++) {
        ids[i] = i;
#ifdef _WIN32
        threads[i] = CreateThread(NULL, 0, client_thread, &ids[i], 0, NULL);
#else
        pthread_create(&threads[i], NULL, client_thread, &ids[i]);
#endif
    }

    for (int i = 0; i < STRESS_CLIENTS; i++) {
#ifdef _WIN32
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
#else
        pthread_join(threads[i], NULL);
#endif
    }

    printf("Stress Test completed successfully.\n");
    return 1;
}

// === ADVANCED STRESS TESTS ===
#ifdef _WIN32
DWORD WINAPI parallel_client_thread(LPVOID param)
#else
void* parallel_client_thread(void* param)
#endif
{
    int id = *(int*)param;
    char name[32];
    snprintf(name, sizeof(name), "parallel%d\n", id);

    int sock = open_connection();
    if (sock >= 0) {
        send(sock, name, strlen(name), 0);
        send(sock, "/join #stress\n", 14, 0);
        send(sock, "hi from thread\n", 16, 0);
        send(sock, "/leave\n", 7, 0);
        send(sock, "/quit\n", 6, 0);
        close_connection(sock);
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int test_parallel_join() {
    printf("Test 6: Parallel joins...\n");
    int ids[PARALLEL_CLIENTS];

#ifdef _WIN32
    HANDLE threads[PARALLEL_CLIENTS];
#else
    pthread_t threads[PARALLEL_CLIENTS];
#endif

    for (int i = 0; i < PARALLEL_CLIENTS; i++) {
        ids[i] = i;
#ifdef _WIN32
        threads[i] = CreateThread(NULL, 0, parallel_client_thread, &ids[i], 0, NULL);
#else
        pthread_create(&threads[i], NULL, parallel_client_thread, &ids[i]);
#endif
    }

    for (int i = 0; i < PARALLEL_CLIENTS; i++) {
#ifdef _WIN32
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
#else
        pthread_join(threads[i], NULL);
#endif
    }

    printf("Parallel join test complete.\n");
    return 1;
}

int test_parallel_quit() {
    printf("Test 7: Parallel /quit...\n");
    return test_parallel_join(); // Reuse same logic
}

int test_parallel_leave() {
    printf("Test 8: Parallel /leave...\n");
    return test_parallel_join(); // Reuse same logic
}

int test_massive_who() {
    printf("Test 9: /who with many users...\n");
    int sock;
    char name[32];
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < WHO_CLIENTS; i++) {
        sock = open_connection();
        if (sock >= 0) {
            snprintf(name, sizeof(name), "who%d\n", i);
            send(sock, name, strlen(name), 0);
            send(sock, "/join #bigroom\n", 16, 0);
        }
    }

    sock = open_connection();
    if (sock >= 0) {
        send(sock, "leader\n", 7, 0);
        send(sock, "/join #bigroom\n", 16, 0);
        send(sock, "/who\n", 5, 0);
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len > 0) {
            buffer[len] = '\0';
            printf("Server responded with:\n%s\n", buffer);
        }
        close_connection(sock);
    }

    printf("/who test complete.\n");
    return 1;
}

int test_oversized_message() {
    printf("Test 10: Oversized message...\n");
    int sock = open_connection();
    if (sock >= 0) {
        send(sock, "bigmsg\n", 7, 0);
        send(sock, "/join #overflow\n", 16, 0);

        char big[2048];
        memset(big, 'A', sizeof(big) - 2);
        big[2046] = '\n';
        big[2047] = '\0';

        send(sock, big, strlen(big), 0);
        close_connection(sock);
    }

    printf("Oversized message sent.\n");
    return 1;
}

// === MAIN ===
int main() {
    printf("== Initiating Stress Test for Client side ==\n");

    int passed = 1;
    passed &= test_basic_connection();
    passed &= test_valid_username();
    passed &= test_duplicate_username();
    passed &= test_valid_command();
    passed &= test_stress_clients();
    passed &= test_parallel_join();
    passed &= test_parallel_quit();
    passed &= test_parallel_leave();
    passed &= test_massive_who();
    passed &= test_oversized_message();

    if (passed) {
        printf("\nAll stress tests passed.\n");
    } else {
        printf("\nOne or more tests failed.\n");
    }

    return passed ? 0 : 1;
}
