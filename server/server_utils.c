#include "server_utils.h"
#include "client_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

/// @brief Validate if a given name is valid (letters, numbers, underscore, dash, or #).
/// @param name The string to validate.
/// @return true if valid, false otherwise.
bool is_valid_name(const char* name) {
    if (!name || strlen(name) == 0) return false;
    for (int i = 0; name[i]; i++) {
        char c = name[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '#')) return false;
    }
    return true;
}

/// @brief Send a message string to a socket.
/// @param sock The socket to send to.
/// @param msg The null-terminated message string.
void send_message(socket_t sock, const char* msg) {
    send(sock, msg, (int)strlen(msg), 0);
}

/// @brief Broadcast a message to all clients in the senders channel.
///        If sender not in a channel, tells sender to join one.
/// @param sender Socket of sender.
/// @param message Message to broadcast.
void broadcast(socket_t sender, const char* message) {
    int sender_idx = get_client_index(sender);
    if (sender_idx == -1 || strlen(clients[sender_idx].channel) == 0) {
        send_message(sender, "Join a channel to send messages.\n");
        return;
    }
    // this logs the message 
    log_message(clients[sender_idx].channel, clients[sender_idx].username, message);
    
    const char* username = clients[sender_idx].username;
    const char* channel = clients[sender_idx].channel;
    const char* color = clients[sender_idx].color_code;

    // Format message including sender's color and username.
    char formatted_message[BUFFER_SIZE];
    snprintf(formatted_message, sizeof(formatted_message), "%s%s%s: %s", color, username, "\x1b[0m", message);

    lock_clients();
    // Send message to all clients in the same channel.
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].channel, channel) == 0) {
            send_message(clients[i].socket, formatted_message);
        }
    }
    unlock_clients();
}

    /* Logs chat messages to a file with timestamp, channel, and username
    * Creates consistent format: [timestamp] channel @user: message */
    void log_message(const char* channel, const char* username, const char* message) {
    time_t now;
    time(&now);
    // Format timestamp as [YYYY-MM-DD HH:MM:SS]
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    
    // create log files to the logs folder
    FILE *logfile = fopen("logs/server_chat.log", "a");
    if (!logfile){
        perror("Failed to open log file"); 
        return;
    }
    
    fprintf(logfile, "%s %s @%s: %s\n", timestamp, channel, username, message);
    fclose(logfile);
    }

/// @brief Send a list of active channels to the client.
/// @param sock Client socket to send the channel list.
void list_channels(socket_t sock) {
    char response[BUFFER_SIZE] = "Channels:\n";
    char seen[MAX_CLIENTS][MAX_CHANNEL_NAME];
    int seen_count = 0;

    lock_clients();
    for (int i = 0; i < client_count; i++) {
        const char* ch = clients[i].channel;
        if (strlen(ch) == 0) continue;  // Skip clients not in a channel.

        // Check if channel is already listed.
        bool found = false;
        for (int j = 0; j < seen_count; j++) {
            if (strcmp(seen[j], ch) == 0) {
                found = true;
                break;
            }
        }

        // Add new channel to list and append to response string.
        if (!found) {
            strncpy(seen[seen_count++], ch, MAX_CHANNEL_NAME);
            strncat(response, " - ", sizeof(response) - strlen(response) - 1);
            strncat(response, ch, sizeof(response) - strlen(response) - 1);
            strncat(response, "\n", sizeof(response) - strlen(response) - 1);
        }
    }
    unlock_clients();

    // If no channels found, let client know.
    if (seen_count == 0)
        strncat(response, " (none)\n", sizeof(response) - strlen(response) - 1);

    send_message(sock, response);
}

/// @brief List users in the same channel as the requesting client.
/// @param sock Client socket requesting user list.
void who_in_channel(socket_t sock) {
    int idx = get_client_index(sock);
    if (idx == -1 || strlen(clients[idx].channel) == 0) {
        send_message(sock, "You're not in a channel.\n");
        return;
    }

    char response[BUFFER_SIZE] = "Users:\n";

    lock_clients();
    // Append each user in the same channel to response string.
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].channel, clients[idx].channel) == 0) {
            char entry[100];
            snprintf(entry, sizeof(entry), " - %s%s%s\n",
                     clients[i].color_code,
                     clients[i].username,
                     "\x1b[0m");
            strncat(response, entry, sizeof(response) - strlen(response) - 1);
        }
    }
    unlock_clients();

    send_message(sock, response);
}

/// @brief Send a private message from sender to target user.
/// @param sender Socket of sender.
/// @param target Username of recipient.
/// @param msg Message to send.
void private_msg(socket_t sender, const char* target, const char* msg) {
    int sender_idx = get_client_index(sender);
    if (sender_idx == -1) return;  // Sender not found.

    const char* sender_name = clients[sender_idx].username;
    const char* sender_color = clients[sender_idx].color_code;

    lock_clients();
    // Find the target client by username and send PM.
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, target) == 0) {
            char pm[BUFFER_SIZE];
            snprintf(pm, sizeof(pm), "%s[PM from %s]:%s %s\n",
                     sender_color, sender_name, "\x1b[0m", msg);
            send_message(clients[i].socket, pm);
            unlock_clients();
            return;
        }
    }
    unlock_clients();

    // Target not found; let sender know.
    send_message(sender, "User not found.\n");
}

/// @brief Set a clients channel for joining.
/// @param sock Client socket.
/// @param name Channel name to join.
void join_channel(socket_t sock, const char* name) {
    if (!is_valid_name(name)) {
        send_message(sock, "Invalid channel name.\n");
        return;
    }

    int idx = get_client_index(sock);
    if (idx != -1) {
        lock_clients();
        strncpy(clients[idx].channel, name, MAX_CHANNEL_NAME);
        clients[idx].channel[MAX_CHANNEL_NAME - 1] = '\0';  // Ensure null-termination.
        unlock_clients();

        char msg[BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "Joined channel %s\n", name);
        send_message(sock, msg);
    }
}

/// @brief Remove a client from their current channel.
/// @param sock Client socket.
void leave_channel(socket_t sock) {
    int idx = get_client_index(sock);
    if (idx != -1) {
        lock_clients();
        clients[idx].channel[0] = '\0';
        unlock_clients();
        send_message(sock, "You have left the channel.\n");
    }
}

// Command Handler Functions

/// @brief Handler for JOIN command.
/// @param sock Client socket.
/// @param args Channel name to join.
void handle_join(socket_t sock, char* args) {
    if (args && *args) {
        join_channel(sock, args);
    } else {
        send_message(sock, "JOIN command requires a channel name.\n");
    }
}

/// @brief Handler for LEAVE, /leave command
/// @param sock Client socket.
/// @param args Unused.
void handle_leave(socket_t sock, char* args) {
    (void)args;
    leave_channel(sock);
}

/// @brief Handler for LIST, /list command.
/// @param sock Client socket.
/// @param args Unused.
void handle_list(socket_t sock, char* args) {
    (void)args;
    list_channels(sock);
}

/// @brief Handler for WHO, /who command.
/// @param sock Client socket.
/// @param args Unused.
void handle_who(socket_t sock, char* args) {
    (void)args;
    who_in_channel(sock);
}

/// @brief Handler for MSG, /msg command. Parses and sends private message.
/// @param sock Client socket.
/// @param args Message argument string starting with '@username'.
void handle_msg(socket_t sock, char* args) {
    if (args && *args) {
        if (args[0] == '@') {
            char* space = strchr(args, ' ');
            if (space) {
                *space = '\0';  // Separate target username and message.
                private_msg(sock, args + 1, space + 1);
            } else {
                send_message(sock, "Invalid MSG format.\n");
            }
        } else {
            send_message(sock, "Unrecognized input\n");
        }
    } else {
        send_message(sock, "Unrecognized input\n");
    }
}

/// @brief Handler for QUIT, /quit command.
/// @param sock Client socket.
/// @param args Unused.
/// @note Actual quit handling is done in main loop by breaking on quit.
void handle_quit(socket_t sock, char* args) {
    (void)sock;
    (void)args;
    // No operation here. Handled main client loop.
}

/// @brief Call respective handler for cmd. 
///        If command not recognized, treats as broadcast message.
/// @param sock Client socket issuing the command.
/// @param cmd Command string.
/// @param args Arguments string following the command.
void dispatch_command(socket_t sock, const char* cmd, char* args) {
    typedef void (*cmd_handler_t)(socket_t, char*);
    struct {
        const char* name;
        cmd_handler_t handler;
    } commands[] = {
        {"JOIN", handle_join},
        {"LEAVE", handle_leave},
        {"LIST", handle_list},
        {"WHO", handle_who},
        {"MSG", handle_msg},
        {"QUIT", handle_quit},
        {NULL, NULL}  // End of array. 
    };

    // Iterate through commands and call matching handler.
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            commands[i].handler(sock, args);
            return;
        }
    }

    // If arguments exist and are not an empty string, broadcast them.
    // otherwise, broadcast an empty string.
    if (args != NULL && *args != '\0') {
        broadcast(sock, args);
    } else {
        broadcast(sock, "");
    }
}
