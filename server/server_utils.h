#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "server_net.h"
#include <stdbool.h>
#include <stdarg.h>  // For variadic functions like debug_and_send

//  Checks if a provided name (username or channel) is valid.
//        Ensures non-empty and contains no spaces.
bool is_valid_name(const char* name);

//  Send a message sock client socket.
void send_message(socket_t sock, const char* msg);

//  Send message to all clients except sender.
void broadcast(socket_t sender, const char* message);

//  Send list of all existing channels client.
void list_channels(socket_t sock);

//  Send a list of users in the sender's current channel.
void who_in_channel(socket_t sock);

//  Send a private message from one client to target user..
void private_msg(socket_t sender, const char* target, const char* msg);

//  Adds user to the specified channel and announces their join.
void join_channel(socket_t sock, const char* name);

//  Removes a user from their current channel and notifies others.
void leave_channel(socket_t sock);

//declaration to log chat messgae
void log_message(const char* channel, const char* username, const char* message);

// Result codes returned by dispatch_command(). 
typedef enum {
    DISPATCH_OK = 0,      // keep connection alive 
    DISPATCH_QUIT = 1     // client requested quit 
} dispatch_result_t;


// Parse and service a line-trimmed input from client. 
// Handles normal chat lines, and command lines. 
// Returns DISPATCH_QUIT when client requested quit. 
dispatch_result_t dispatch_command(socket_t sock, const char* line);


// Helper methods for join, msg, and quit.
void handle_join(socket_t sock, char* args); 
void handle_msg(socket_t sock, char* args);
void handle_quit();



#endif