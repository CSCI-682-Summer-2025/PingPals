#include <stdio.h>
#include <string.h>
#include "client_utils.h"

int parse_command(const char* input, char* output, int output_size) {
    if (strncmp(input, "/msg ", 5) == 0) {
        // Format: /msg @user Hello → MSG @user Hello
        snprintf(output, output_size, "MSG %s", input + 5);
        return 1;
    } else if (strncmp(input, "/join ", 6) == 0) {
        // Format: /join #channel → JOIN #channel
        snprintf(output, output_size, "JOIN %s", input + 6);
        return 1;
    } else if (strcmp(input, "/quit") == 0) {
        snprintf(output, output_size, "QUIT");
        return 1;
    } else {
        // Not a valid command
        return 0;
    }
}
