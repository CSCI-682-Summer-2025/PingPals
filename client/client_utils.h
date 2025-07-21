#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

// Parses input to extract command. 
//int parse_command(const char* input, char* output, int output_size);

// Used to append username and channels to output.
void print_user_channels_prompt(const char* username, char channels[][50], int channel_count);

// Retrieves current timestamp in [HH:MM:SS] format.
void get_timestamp(char* buffer, size_t len);

// Checks whether the message originated self.
int is_self_message(const char* msg, const char* user);

// Safe send wrapper.
int safe_send(int sock, const char* msg);

#endif
