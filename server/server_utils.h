#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "server_net.h"
#include <stdbool.h>
#include <stdarg.h>  // For variadic functions like debug_and_send

//  Checks if a provided name (username or channel) is valid.
//        Ensures non-empty and contains no spaces.
bool is_valid_name(const char* name);

//  Sends a message sock client socket.
void send_message(socket_t sock, const char* msg);

//  Sends message to all clients except sender.
void broadcast(socket_t sender, const char* message);

//  Sends list of all existing channels client.
void list_channels(socket_t sock);

//  Sends a list of users in the sender's current channel.
void who_in_channel(socket_t sock);

//  Sends a private message from one client to target user..
void private_msg(socket_t sender, const char* target, const char* msg);

//  Adds user to the specified channel and announces their join.
void join_channel(socket_t sock, const char* name);

//  Removes a user from their current channel and notifies others.
void leave_channel(socket_t sock);

//  Sends a formatted message to a socket (for example,  printf + send).
// For debugging.
void debug_and_send(socket_t sock, const char* format, ...);


void handle_join(socket_t sock, char* args);
void handle_leave(socket_t sock, char* args);
void handle_list(socket_t sock, char* args);
void handle_who(socket_t sock, char* args);
void handle_msg(socket_t sock, char* args);
void handle_quit(socket_t sock, char* args);

// Runs server commands.
void dispatch_command(socket_t sock, const char* cmd, char* args);

#endif