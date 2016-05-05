#ifndef SIK_CHAT_COMMON_H
#define SIK_CHAT_COMMON_H

#include <stdint.h>

#define BUF_SIZE 1000
#define DEFAULT_PORT 20160
#define POLL_REFRESH_TIME 5000

typedef struct __attribute__((__packed__)) message {
    uint16_t len;
    char data[BUF_SIZE];
} message;

void set_sigint_behaviour(void(*handler)(int));
void validate_arguments_and_set_connection_port(
        int argc, char **argv, int max_expected, int *port, char *param_info);

#endif //SIK_CHAT_COMMON_H
