#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "client_utils.h"

/// @brief Adds @username channel... > in front of each message.
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

/// @brief 
/// @param input Pointer to character array to check. 
/// @param output Pointer to character array holds output. 
/// @param output_size 
/// @return 
int parse_command(const char* input, char* output, int output_size) {
    if (strncmp(input, "/msg ", 5) == 0) {
        const char* body = input + 5;   // Skip past "/msg " get rest of input.
        const char* at = strchr(body, '@'); // Find @ symbol, indicating username.
        const char* space = strchr(body, ' '); // Find 1st space after /msg command

        bool has_at_symbol = (at != NULL);  
        bool has_space = (space != NULL);
        bool at_before_space = has_at_symbol && has_space && (at < space);
        bool has_message = has_space && (strlen(space + 1) > 0);

        // If any checks fail, reject command.
        if (!has_at_symbol || !has_space || !at_before_space || !has_message) {
            return 0;  // Invalid /msg format
        }

        snprintf(output, output_size, "MSG %s", body);
        return 1;
    } else if (strncmp(input, "/join ", 6) == 0) {
        const char* chan = input + 6;   // Skip "/join " prefix, get rest of input. 

        if (strlen(chan) == 0 || strchr(chan, ' ')) { // If is empty, or has spaces, invalid. Thus, return 0.
            return 0;  
        }

        snprintf(output, output_size, "JOIN %s", chan);
        return 1;
    } else if (strcmp(input, "/leave") == 0) {
        snprintf(output, output_size, "LEAVE");
        return 1;
    } else if (strcmp(input, "/list") == 0) {
        snprintf(output, output_size, "LIST");
        return 1;
    } else if (strcmp(input, "/who") == 0) {
        snprintf(output, output_size, "WHO");
        return 1;
    } else if (strncmp(input, "/kick ", 6) == 0) {
        snprintf(output, output_size, "KICK %s", input + 6);
        return 1;
    } else if (strcmp(input, "/quit") == 0) {
        snprintf(output, output_size, "QUIT");
        return 1;
    }

    return 0;
}

/// @brief Return number of bytes successfully sent. 0 if input message empty or null.
/// @param sock 
/// @param msg 
/// @return 
int safe_send(int sock, const char* msg) {
    if (!msg || strlen(msg) == 0) return 0;
    return send(sock, msg, (int)strlen(msg), 0);
}
