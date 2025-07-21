#include "command_parser.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

command_t parse_command(const char *input, char *args, size_t args_size) {
    if (args_size > 0) args[0] = '\0';
    if (!input || input[0] == '\0') return CMD_NONE;

    // Must start with /.
    if (input[0] != '/') return CMD_NONE;

    const char *cmd = input + 1;  // Skip the /
    
    // Extract first token.
    char token[16];
    size_t ti = 0;
    while (*cmd && !isspace((unsigned char)*cmd) && ti < sizeof(token) - 1) {
        token[ti++] = (char)tolower((unsigned char)*cmd);
        cmd++;
    }
    token[ti] = '\0';

    // Skip spaces to find args.
    while (*cmd && isspace((unsigned char)*cmd)) 
    {
        cmd++;
    }

    if (args_size > 0) snprintf(args, args_size, "%s", cmd);

    if (ti == 0) return CMD_INVALID;

    if (strcmp(token, "msg") == 0) return CMD_MSG;
    if (strcmp(token, "join") == 0) return CMD_JOIN;
    if (strcmp(token, "leave") == 0) return CMD_LEAVE;
    if (strcmp(token, "list") == 0) return CMD_LIST;
    if (strcmp(token, "who") == 0) return CMD_WHO;
    if (strcmp(token, "quit") == 0) return CMD_QUIT;

    return CMD_UNKNOWN;
}