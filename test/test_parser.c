#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "client_utils.h"

void test_valid_commands() {
    char output[1024];

    assert(parse_command("/join #test", output, 1024) == 1);
    assert(strcmp(output, "JOIN #test") == 0);

    assert(parse_command("/msg @gary hello", output, 1024) == 1);
    assert(strcmp(output, "MSG @gary hello") == 0);

    assert(parse_command("/leave", output, 1024) == 1);
    assert(strcmp(output, "LEAVE") == 0);

    assert(parse_command("/list", output, 1024) == 1);
    assert(strcmp(output, "LIST") == 0);

    assert(parse_command("/who", output, 1024) == 1);
    assert(strcmp(output, "WHO") == 0);

    assert(parse_command("/quit", output, 1024) == 1);
    assert(strcmp(output, "QUIT") == 0);
}

void test_invalid_commands() {
    char output[1024];

    assert(parse_command("/randocomando", output, 1024) == 0);
    assert(parse_command("", output, 1024) == 0);
    assert(parse_command("/msggaryhello", output, 1024) == 0); // malformed
}

void test_edge_cases() {
    char output[1024];

    // Very long input.
    char long_input[1100];
    memset(long_input, 'a', sizeof(long_input));
    long_input[1099] = '\0';

    assert(parse_command(long_input, output, 1024) == 0);
}

int main() {
    test_valid_commands();
    test_invalid_commands();
    test_edge_cases();

    printf("All parser tests passed.\n");
    return 0;
}
