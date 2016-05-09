#include <stdio.h>
#include "common.h"

#include <stdlib.h>
#include <limits.h>
#include <netinet/in.h>
#include <string.h>

void validate_arguments_and_set_connection_port(
        int argc, char **argv, int max_expected, int *port, char *param_info) {
    if (argc > max_expected || argc < max_expected - 1) {
        printf("Usage: %s host [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == max_expected) {
        *port = atoi(argv[max_expected-1]);
    }
    if (*port > USHRT_MAX || *port < 0) {
        printf("Usage: %s %s\n", argv[0], param_info);
        exit(EXIT_FAILURE);
    }
}

bool is_message_valid(message *msg, ssize_t len) {
//    fprintf(stderr, "read val: %ld, msg len: %d, str len: %d\n", len, ntohs(msg->len), (int)strnlen(msg->data, (size_t)len));
    if (ntohs(msg->len) <= BUF_SIZE
        && ntohs(msg->len) == len
        && len == strnlen(msg->data, (size_t)len)) {
        for (int i = 0; i < len; i++) {
            if (msg->data[i] == '\0' || msg->data[i] == '\n') {
                return false;
            }
        }
        return true;
    };
    return false;
}