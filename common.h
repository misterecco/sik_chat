#ifndef SIK_CHAT_COMMON_H
#define SIK_CHAT_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define BUF_SIZE 1000
#define DEFAULT_PORT 20160
#define POLL_REFRESH_TIME 5000
#define EXIT_WRONG_MESSAGE 100

typedef struct __attribute__((__packed__)) message {
    uint16_t len;
    char data[BUF_SIZE];
} message;

typedef struct buffer_state {
    size_t length_read;
    int data_read;
    message msg;
} buffer_state;

void validate_arguments_and_set_connection_port(
        int argc, char **argv, int max_expected, int *port, char *param_info);
bool is_message_valid(message *msg, ssize_t len);

#endif //SIK_CHAT_COMMON_H
