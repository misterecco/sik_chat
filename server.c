#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "err.h"
#include "comm.h"

#define DEFAULT_PORT 20160
#define MAX_CLIENTS 21

static int finish = false;
static int connection_port = DEFAULT_PORT;
static int active_clients = 0;
static struct pollfd client[MAX_CLIENTS];
static struct sockaddr_in server;
static int msgsock, ret;

#ifdef DEBUG
static bool debug = true;
#else
static bool debug = false;
#endif

static void catch_int (int sig) {
    finish = true;
    fprintf(stderr,
            "Signal %d caught. No new connections will be accepted.\n", sig);
}

static void validate_and_set_connection_port(int argc, char **argv) {
    if (argc > 2) {
        printf("Usage: %s [port]\n", argv[0]);
        exit(1);
    }
    if (argc == 2) {
        connection_port = atoi(argv[1]);
    }
    if (connection_port > USHRT_MAX || connection_port < 0) {
        printf("Invalid number of port\n");
        exit(EXIT_FAILURE);
    }
}

static void set_sigint_behaviour(void(*handler)(int)) {
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

static void initialize_clients_array() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client[i].fd = -1;
        client[i].events = POLLIN;
        client[i].revents = 0;
    }
}

static void create_central_socket() {
    client[0].fd = socket(PF_INET, SOCK_STREAM, 0);
    if (client[0].fd < 0) {
        perror("Opening stream socket");
        exit(EXIT_FAILURE);
    }
}

static void bind_port_to_socket() {
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(connection_port);
    if (bind(client[0].fd, (struct sockaddr*)&server,
             (socklen_t)sizeof(server)) < 0) {
        perror("Binding stream socket");
        exit(EXIT_FAILURE);
    }
}

static void get_socket_info() {
    size_t length;
    length = sizeof(server);
    if (getsockname (client[0].fd, (struct sockaddr*)&server,
                     (socklen_t*)&length) < 0) {
        perror("Getting socket name");
        exit(EXIT_FAILURE);
    }
    printf("Socket port: %u\n", (unsigned)ntohs(server.sin_port));
}

static void listen_on_central_socket() {
    if (listen(client[0].fd, 5) == -1) {
        perror("Starting to listen");
        exit(EXIT_FAILURE);
    }
}

static void close_central_socket_if_necessary() {
    if (finish == true && client[0].fd >= 0) {
        if (close(client[0].fd) < 0)
            perror("close");
        client[0].fd = -1;
    }
}

static void reset_revents() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client[i].revents = 0;
    }
}

static void accept_new_client() {
    if (finish == false && (client[0].revents & POLLIN)) {
        msgsock = accept(client[0].fd, (struct sockaddr*)0, (socklen_t*)0);
        if (msgsock == -1) {
            perror("accept");
        }
        else {
            int i;
            for (i = 1; i < MAX_CLIENTS; ++i) {
                if (client[i].fd == -1) {
                    client[i].fd = msgsock;
                    active_clients += 1;
                    break;
                }
            }
            if (i >= MAX_CLIENTS) {
                fprintf(stderr, "Too many clients\n");
                if (close(msgsock) < 0)
                    perror("close");
            }
        }
    }
}

static void close_client_socket_if_necessary(int client_number) {
    if (close(client[client_number].fd) < 0)
        perror("close");
    client[client_number].fd = -1;
    active_clients -= 1;
};

static void read_and_broadcast_messages_and_close_connections() {
    message msg;
    for (int i = 1; i < MAX_CLIENTS; ++i) {
        if (client[i].fd != -1 && (client[i].revents & (POLLIN | POLLERR))) {
            ssize_t rval = read(client[i].fd, &msg, sizeof(message));
            if (rval < 0) {
                perror("Reading stream message");
                close_client_socket_if_necessary(i);
            }
            else if (rval == 0) {
                if (debug) fprintf(stderr, "Ending connection\n");
                close_client_socket_if_necessary(i);
            }
            else
                //TODO: validate and broadcast instead of printing
                printf("Msg: %d, %d, %s\n", (int)rval, ntohs(msg.len), msg.data);
        }
    }
}

static void do_poll() {
    ret = poll(client, MAX_CLIENTS, 5000);
    if (ret < 0) {
        perror("poll");
    }
    else if (ret > 0) {
        accept_new_client();
        read_and_broadcast_messages_and_close_connections();
    }
    else if (debug) {
        fprintf(stderr, "Do something else\n");
    }
}

int main (int argc, char** argv) {

    validate_and_set_connection_port(argc, argv);
    set_sigint_behaviour(catch_int);
    initialize_clients_array();
    create_central_socket();
    bind_port_to_socket();
    if (debug) get_socket_info();
    listen_on_central_socket();

    do {
        reset_revents();
        close_central_socket_if_necessary();
        do_poll();
    } while (finish == false || active_clients > 0);

    close_central_socket_if_necessary();
    exit(EXIT_SUCCESS);
}
