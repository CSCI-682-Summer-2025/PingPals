#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
#endif

//#include <sys/socket.h>

#include "client_utils.h"

/// @brief Adds @username channel_name> in front of each message.
/// @param username 
/// @param channels 
/// @param channel_count 
void print_user_channels_prompt(const char* username, char channels[][50], int channel_count) {
    printf("\r%s", username); 
    for (int i = 0; i < channel_count; ++i) {
        printf(" @%s", channels[i]);
    } 
    printf(" > ");
    fflush(stdout);
}

/// @brief Fills buffer with current time. Format: [HH:MM:SS]
/// @param buffer The array where timestamp will be written to.
/// @param len Length of buffer, ensure no overflow.
void get_timestamp(char* buffer, size_t len) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, len, "[%H:%M:%S]", t); // Format time as [HH:MM:SS], store in buffer.
}

/// @brief Determine if message sent by user (self)
/// @param msg Full message string. 
/// @param check_username Username checking if is self.
/// @return Return 1 if from self, 0 otherwise.
int is_self_message(const char* msg, const char* check_username) {
    const char* name_pos = strstr(msg, check_username); // Get where username is in the actual message.
    const char* colon_pos = strstr(msg, ":"); // Find 1st colon in message.
    
    // Return 1 if matching username exists and is in correct position.
    return (name_pos != NULL && colon_pos != NULL && name_pos < colon_pos);
}


/// @brief Return number of bytes successfully sent. 0 if input message empty or null.
/// @param sock 
/// @param msg 
/// @return 
int safe_send(int sock, const char* msg) {
    if (!msg || strlen(msg) == 0) return 0;
    return send(sock, msg, (int)strlen(msg), 0);
}
