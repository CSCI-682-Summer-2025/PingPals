COMPILER = gcc
CFLAGS = -Wall -Wextra -pthread -g

SERVER_DIR = server/server.c
CLIENT_DIR = client/client.c

SERVER_EXE = output_server
CLIENT_EXE = output_client

.PHONY: all clean


all: $(SERVER_EXE) $(CLIENT_EXE)

$(SERVER_EXE): $(SERVER_DIR)
	$(COMPILER) $(CFLAGS) -o $@ $^

$(CLIENT_EXE): $(CLIENT_DIR)
	$(COMPILER) $(CFLAGS) -o $@ $^


clean:
	rm -f $(SERVER_EXE) $(CLIENT_EXE)