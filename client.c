#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <poll.h>
#include "err.h"
#include "common.h"

static int sock;
static struct addrinfo addr_hints, *addr_result;
static bool finish = false;
static int connection_port = DEFAULT_PORT;
static struct pollfd streams[2];
static void catch_int (int sig) {
    finish = true;
    fprintf(stderr, "Signal %d caught. No new messages will be received.\n", sig);
}

static void create_socket() {
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        syserr("socket");
    }
}

static void get_server_address(int argc, char **argv) {
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_flags = 0;
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;

    int rc =  getaddrinfo(argv[1], argc == 3 ? argv[2] : "20160",
                          &addr_hints, &addr_result);
    if (rc != 0) {
        fprintf(stderr, "rc=%d\n", rc);
        syserr("getaddrinfo: %s", gai_strerror(rc));
    }
}

static void connect_to_server() {
    if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) != 0) {
        syserr("connect");
    }
    freeaddrinfo(addr_result);
}

static void initialize_streams() {
    streams[0].fd = 0; // stdin
    streams[1].fd = sock;
    for (int i = 0; i <= 1; i++) {
        streams[i].events = POLLIN;
        streams[i].revents = 0;
    }
}

static void close_socket() {
    if (close(sock) < 0) {
        perror("closing stream socket");
    }
}

static void receive_message() {
    if (streams[1].revents & (POLLIN | POLLERR)) {
        message msg;
        ssize_t rval = read(sock, &msg, sizeof(message));
        if (rval < 0) {
            perror("Reading stream message");
            close_socket();
        }
        else if (rval == 0) {
            fprintf(stderr, "Ending connection\n");
            close_socket();
        }
        else {
            size_t len = ntohs(msg.len);
            if (!is_message_valid(&msg, rval)) {
                perror("message from server invalid\n");
                close_socket();
                exit(EXIT_WRONG_MESSAGE);
            } else {
                fwrite(msg.data, 1, len, stdout);
                fwrite("\n", 1, 1, stdout);
            }
        }
    }
}

static void send_message() {
    if (streams[0].revents & POLLIN) {
        char line[BUF_SIZE];
        message msg;
        fgets(line, sizeof line, stdin);
        //TODO: validate input, handle edge cases
        uint16_t line_len = strlen(line) - 1;
        msg.len = htons(line_len);
        strncpy(msg.data, line, line_len);
        if (write(sock, &msg, sizeof(uint16_t) + line_len) < 0) {
            perror("writing on stream socket");
        }
    }
}

static void do_poll() {
    int ret = poll(streams, 2, POLL_REFRESH_TIME);
    if (ret < 0) {
        perror("poll");
    }
    else if (ret > 0) {
        receive_message();
        send_message();
    }
}

static void reset_revents() {
    for (int i = 0; i <= 1; ++i) {
        streams[i].revents = 0;
    }
}

int main (int argc, char **argv) {

    validate_arguments_and_set_connection_port(argc, argv, 3, &connection_port,
                                               "hostname [port]");
    set_sigint_behaviour(catch_int);
    create_socket();
    get_server_address(argc, argv);
    connect_to_server();
    initialize_streams();

    do {
//        reset_revents();
        do_poll();
    }
    while (!finish);
    close_socket();
    exit(EXIT_SUCCESS);
}
