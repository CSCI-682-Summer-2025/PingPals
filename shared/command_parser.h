#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stddef.h>

// Command enumeration.
typedef enum {
    CMD_NONE = -1,    // Normal message.
    CMD_MSG,
    CMD_JOIN,
    CMD_LEAVE,
    CMD_LIST,
    CMD_WHO,
    CMD_QUIT,
    CMD_INVALID,       // Invalid syntax.
    CMD_UNKNOWN        // Starts with '/'. Unrecognized.
} command_t;

// Parse an input string to detect a command.
// Returns one of the command_t values.
// input: Raw user input string.
// args:  Buffer for text after the command.
// args_size: Size of args buffer.
command_t parse_command(const char *input, char *args, size_t args_size);

#endif
