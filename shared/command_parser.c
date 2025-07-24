#include "command_parser.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

command_t parse_command(const char *input, char *args, size_t args_size) {
    if (args_size > 0) args[0] = '\0';
    if (!input || input[0] == '\0') return CMD_NONE;

    // Must start with /
    if (input[0] != '/') return CMD_NONE;

    const char *cmd_ptr = input + 1;  // skip '/'

    // Extract command token.
    char token[16];
    size_t ti = 0;
    while (*cmd_ptr && !isspace((unsigned char)*cmd_ptr) && ti < sizeof(token) - 1) {
        token[ti++] = (char)tolower((unsigned char)*cmd_ptr);
        cmd_ptr++;
    }
    token[ti] = '\0';

    // Skip spages.
    while (*cmd_ptr && isspace((unsigned char)*cmd_ptr)) {
        cmd_ptr++;
    }

    if (ti == 0) return CMD_INVALID;

    // Validate commands and their argument.
    if (strcmp(token, "msg") == 0) {
        // /msg must start with '@username ' & have message text. 
        if (args_size > 0) args[0] = '\0'; // clear first

        if (*cmd_ptr != '@') return CMD_INVALID;

        // Find first space after '@username'
        const char *space = strchr(cmd_ptr, ' ');
        if (!space) return CMD_INVALID; // no message part
        if ((size_t)(space - cmd_ptr) < 2) return CMD_INVALID; // username too short (@ + at least 1 char)

        // Check message is non-empty after space
        if (*(space + 1) == '\0') return CMD_INVALID;

        // Valid - copy args
        if (args_size > 0) snprintf(args, args_size, "%s", cmd_ptr);
        return CMD_MSG;
    }

    if (strcmp(token, "join") == 0) {
        // /join must have exactly one argument passed without any spaces, and can't be empty.
        if (cmd_ptr == NULL || *cmd_ptr == '\0') return CMD_INVALID;
        // no spaces allowed in args
        if (strchr(cmd_ptr, ' ') != NULL) return CMD_INVALID;

        // If it's valid, copy  - copy string pointed to by cmd_ptr into args.
        if (args_size > 0) snprintf(args, args_size, "%s", cmd_ptr);
        return CMD_JOIN;
    }

    if (strcmp(token, "leave") == 0) {
        if (args_size > 0) args[0] = '\0';
        return CMD_LEAVE;
    }

    if (strcmp(token, "list") == 0) {
        if (args_size > 0) args[0] = '\0';
        return CMD_LIST;
    }

    if (strcmp(token, "who") == 0) {
        if (args_size > 0) args[0] = '\0';
        return CMD_WHO;
    }

    if (strcmp(token, "quit") == 0) {
        if (args_size > 0) args[0] = '\0';
        return CMD_QUIT;
    }

    return CMD_UNKNOWN;
}
