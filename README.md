# PingPals Resources

## Description 
This repo serves as the centralized location for project finalized documents and ongoing document editing
links for the PingPals project. 


## Software Project Proposal Link
- [Project Proposal Link] ([https://docs.google.com/presentation/d/1ohTR4xBHpi-dhtWwrpHq6X1sSaoh3yaWUjjtMxD1bgY/edit?usp=sharing](https://docs.google.com/document/d/1gUZ6fC7cdtD00fQJZG8WWsQUR51fD0i15C5mluRFe3Q/edit?usp=sharing)

- [Submitted Proposal Stored Project Release] (https://github.com/user-attachments/files/20930633/CSCI_682_Summer_2025_ProjectProposal.pdf)


## Team Chat Link
- [Discord Link] (https://discord.gg/ZZGTUm8c)

## 🧑‍💻 Client Functionality – Implemented by Álvaro Salvador

This section explains what I’ve built for the PingPals client, and how you can compile and test it.

### ✔ What I have implemented:
- A fully working TCP client in C for Windows using `winsock2.h`.
- The client connects to the server on port `9090`.
- Input and output are handled through the terminal.
- Users can type commands or plain messages and send them to the server.
- The client runs a background thread to receive messages from the server while still accepting input.
- The following commands are supported and parsed before being sent:
  - `/msg @user message` → becomes `MSG @user message`
  - `/join #channel` → becomes `JOIN #channel`
  - `/quit` → disconnects from the server

### ▶️ How to run and test the project (step-by-step)
To test the client-server communication, follow these steps:

1️⃣ Open two terminals
You’ll need two separate terminal windows or tabs:

One for running the server

One for running the client

You can use PowerShell, Git Bash, or VSCode’s integrated terminal.

2️⃣ In the first terminal (server)
Navigate to the project folder and compile the server:

gcc server/server.c -o server.exe -lws2_32

Then run the server:

./server.exe

You should see:

Server listening on port 9090...

3️⃣ In the second terminal (client)
Compile the client:

gcc client/client.c client/client_utils.c -o client.exe -lws2_32

Then run it:

./client.exe

You should see:

Connected to the server.
>
At this point, the client will automatically send:

HELLO FROM CLIENT

And the server will print:

Received: HELLO FROM CLIENT

4️⃣ (Optional) Test manual input
After that, you can also manually type any of the following commands in the client:

/msg @juan hello
/join #general
This is a test message
/quit
⚠️ Important:
These messages will be sent to the server, but unless the server is programmed to process them, it will just echo them back (or not respond at all).
You might not see them printed unless the server is still running and echoing back every message.

🧼 If the .exe file cannot be overwritten
If you see an error like Permission denied when recompiling:

Make sure the program is not currently running

Or just compile using a new name, like:

gcc client/client.c client/client_utils.c -o client_v2.exe -lws2_32
./client_v2.exe

---

Updated instructions with implementation: Build & Run Instructions.

This section explains how to build, run, and test the PingPals client-server project using both the Makefile or manual commands.

Building the Project:

  Using the Makefile (recommended):
  
  On Windows and Linux/macOS:
    make all

This builds both client.exe and server.exe, choosing platform-specific files automatically.

Running the Server:

  ./server.exe

The server listens on port 9090 and waits for client connections.

Running the Client:

  In a new terminal, run:

    ./client.exe

Commands that can be run by the client:

    /join #channel — to join or switch to a specific chat channel.

    /leave — to leave the current chat channel.

    /list — to display all active channels on the server.

    /who — to see the list of users currently in your channel.

    /msg @username message — to send a private message directly to another user.

    /kick @username - will remove the user from the chat and broadcast a message to everyone

    /quit — to disconnect from the server and exit the client.

    You can also send plain messages without commands, which will be broadcasted to everyone in your current channel to interact with the server.

Running Tests:

  Parser tests:

    make test_parser

  Stress test (requires Python installed and available in system path):

    make stress_test

  **Server must be running before running and starting stress test.

  Alternatively, to do both:
    make  run_stress_test_with_server

Manual Compilation (if not using Makefile)

Windows (with GCC/MinGW):

  Compile server:

    gcc server/server_main.c server/client_manager.c server/server_utils.c server/server_net_win.c -o server.exe -lws2_32 -Iinclude -Iclient

  Compile client:

    gcc client/client.c client/client_utils.c -o client.exe -lws2_32 -Iinclude

Linux/macOS:

  Compile server:

    gcc server/server_main.c server/client_manager.c server/server_utils.c server/server_net_posix.c -o server.exe -Iinclude -Iclient -lpthread

  Compile client:

    gcc client/client.c client/client_utils.c -o client.exe -Iinclude

Cleaning Build Artifacts

  Remove compiled executables with:

    make clean

## CHAT HISTORY FUNCTIONALITY - BY GODBLESS AMANKWAH
This saves all the chat history 
How to Retrieve Chat History 

Open a New Terminal
- Don't use the server/client terminals
Navigate to Project Folder
Do the following:
- to verify Log file exists, enter : ls -l server_chat.log
Viewing Options
- Full Logs:
	cat server_chat.log
- Live Updates:
	tail -f server_chat.log

You can press Ctrl+C to exit

OTHER FUNCTIONALITIES
Filtering Messages:
- BY USER
	grep "@username" server-chat.log
- BY CHANNEL
	grep "#channel" server_chat.log	
- KEYWORD
	grep "hi" server_chat.log

## REMOVING A USER FROM CHAT
- First user to be connected is admin by defaullt
- Comand for triggering removal is : /kick username
- Server validates admin priviliges before action
- A message is broadcasted to all members of the channel after removal


