#include <stdio.h>
#include "common.h"
#include "err.h"

#include <stdlib.h>
#include <signal.h>
#include <limits.h>

void set_sigint_behaviour(void(*handler)(int)) {
    struct sigaction setup_action;
    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, SIGSTOP | SIGTERM | SIGQUIT);
    setup_action.sa_handler = handler;
    setup_action.sa_mask = block_mask;
    setup_action.sa_flags = 0;
    if (sigaction(SIGINT, &setup_action, 0) == -1) {
        syserr("sigaction");
    }
}

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