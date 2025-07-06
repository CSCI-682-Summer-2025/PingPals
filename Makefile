# Compiler and flags
COMPILER = gcc
CFLAGS = -Wall -Wextra -g -lws2_32

# Paths
CLIENT_SRC = client/client.c client/client_utils.c
SERVER_SRC = server/server.c

CLIENT_EXE = client.exe
SERVER_EXE = server.exe

.PHONY: all client server clean

# Default target
all: client server

client: $(CLIENT_SRC)
	$(COMPILER) $(CFLAGS) -o $(CLIENT_EXE) $(CLIENT_SRC)

server: $(SERVER_SRC)
	$(COMPILER) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC)

clean:
	del /f /q $(CLIENT_EXE) $(SERVER_EXE) 2>nul || true
