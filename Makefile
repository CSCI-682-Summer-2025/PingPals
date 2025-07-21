# Detect OS
ifeq ($(OS),Windows_NT)
	PLATFORM_SRC = server/server_net_win.c
	DEL = del /f /q
	CLEAN_EXIT = exit 0
	WS_LIB = -lws2_32
else
	DEL = rm -f
	CLEAN_EXIT = true
	PLATFORM_SRC = server/server_net_posix.c
endif

# Setup python variables.

PYTHON := $(shell (command -v python3 || command -v python || command -v py) 2>/dev/null)

ifeq ($(PYTHON),)
$(error Python interpreter not found. Please install Python or adjust your PATH.)
endif

# Compiler and flags
COMPILER = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -Iclient -Ishared

# Source file paths.
SHARED_SRC = shared/command_parser.c 
CLIENT_SRC = client/client.c client/client_utils.c $(SHARED_SRC)
SERVER_SRC = server/server_main.c server/client_manager.c server/server_utils.c $(PLATFORM_SRC) $(SHARED_SRC)
TEST_SRC = test/test_parser.c client/client_utils.c $(SHARED_SRC)

# Executable paths. 
CLIENT_EXE = client.exe
SERVER_EXE = server.exe
TEST_PARSER_EXE = test_parser.exe

.PHONY: all client server test clean

# Default target
all: client server

client: $(CLIENT_SRC)
	$(COMPILER) $(CFLAGS) -o $(CLIENT_EXE) $(CLIENT_SRC) $(WS_LIB)

server: $(SERVER_SRC)
	$(COMPILER) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC) $(WS_LIB)

stress_test:
	$(PYTHON) ./test/stress_test.py

test_parser: $(TEST_SRC)
	$(COMPILER) $(CFLAGS) -o $(TEST_PARSER_EXE) $(TEST_SRC) $(WS_LIB)
	./$(TEST_PARSER_EXE)

run_stress_test_with_server:
	@echo "Launching server..."
	@./$(SERVER_EXE) & sleep 1
	@$(PYTHON) ./test/stress_test.py

clean:
	-$(DEL) $(CLIENT_EXE) $(SERVER_EXE) $(TEST_PARSER_EXE) 2>nul || true

remake:
	make clean
	make all

	