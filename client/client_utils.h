#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

// Prepares a formatted command string from user input
// Returns 1 if the command was valid and formatted, 0 otherwise
int parse_command(const char* input, char* output, int output_size);

#endif
