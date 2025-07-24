#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../shared/command_parser.h"  // adjust path if needed

void test_valid_commands() {
    char args[1024];
    assert(parse_command("/join #test", args, sizeof(args)) == CMD_JOIN);
    assert(strcmp(args, "#test") == 0);

    assert(parse_command("/msg @gary hello", args, sizeof(args)) == CMD_MSG);
    assert(strcmp(args, "@gary hello") == 0);

    assert(parse_command("/leave", args, sizeof(args)) == CMD_LEAVE);
    assert(strcmp(args, "") == 0);

    assert(parse_command("/list", args, sizeof(args)) == CMD_LIST);
    assert(strcmp(args, "") == 0);

    assert(parse_command("/who", args, sizeof(args)) == CMD_WHO);
    assert(strcmp(args, "") == 0);

    assert(parse_command("/quit", args, sizeof(args)) == CMD_QUIT);
    assert(strcmp(args, "") == 0);
}

void test_invalid_commands() {
    char args[1024];
    /* malformed /msg */
    assert(parse_command("/msg gary hello", args, sizeof(args)) == CMD_INVALID);
    assert(parse_command("/msg @gary", args, sizeof(args)) == CMD_INVALID);

    /* malformed /join */
    assert(parse_command("/join ", args, sizeof(args)) == CMD_INVALID);
    assert(parse_command("/join bad chan", args, sizeof(args)) == CMD_INVALID);

    /* unknown commands */
    assert(parse_command("/randocomando", args, sizeof(args)) == CMD_UNKNOWN);
    assert(parse_command("/msggaryhello", args, sizeof(args)) == CMD_UNKNOWN);
}

void test_non_commands() {
    char args[1024];
    assert(parse_command("", args, sizeof(args)) == CMD_NONE);
    assert(parse_command("hello world", args, sizeof(args)) == CMD_NONE);
}

void test_edge_cases() {
    char args[1024];
    char long_input[1100];
    memset(long_input, 'a', sizeof(long_input));
    long_input[sizeof(long_input) - 1] = '\0';

    assert(parse_command(long_input, args, sizeof(args)) == CMD_NONE);

    /* case-insensitive tests if supported */
    assert(parse_command("/JOIN #channel1", args, sizeof(args)) == CMD_JOIN);
    assert(strcmp(args, "#channel1") == 0);

    assert(parse_command("/Msg @bobbyf Yo", args, sizeof(args)) == CMD_MSG);
    assert(strcmp(args, "@bobbyf Yo") == 0);
}

int main() {
    test_valid_commands();
    test_invalid_commands();
    test_non_commands();
    test_edge_cases();

    printf("All parser tests passed.\n");
    return 0;
}
