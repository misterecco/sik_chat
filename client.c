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
static buffer_state buffer;

static void reset_buffer() {
    buffer.length_read = 0;
    buffer.data_read = 0;
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
    reset_buffer();
}

static void close_socket() {
    if (sock > 0 && close(sock) < 0) {
        perror("closing stream socket");
    }
    sock = -1;
}

static void receive_message() {
    if (streams[1].revents & (POLLIN | POLLERR | POLLHUP)) {
        size_t len;
        ssize_t rval;
        if (buffer.length_read < 2) {
            rval = read(sock,
                        (char *)&(buffer.msg.len) + buffer.length_read,
                        2 - buffer.length_read);
            if (rval == 0) {
                close_socket();
                exit(EXIT_WRONG_MESSAGE);
            } else if (rval > 0) {
                buffer.length_read += rval;
            } else {
                perror("Reading stream message");
                close_socket();
                exit(EXIT_FAILURE);
            }
        }
        if (buffer.length_read < 2) {
            return;
        }
        len = ntohs(buffer.msg.len);
        if (len > BUF_SIZE) {
            fprintf(stderr, "Message from server invalid, closing\n");
            close_socket();
            exit(EXIT_WRONG_MESSAGE);
        }
        rval = read(sock,
                    buffer.msg.data + buffer.data_read,
                    len - buffer.data_read);
        if (rval < 0) {
            perror("Reading stream message");
            close_socket();
            return;
        }
        buffer.data_read += rval;
        if (buffer.data_read < len) {
            return; // wait for the rest of the message
        }
        if (!is_message_valid(&(buffer.msg), len)) {
            fprintf(stderr, "Message from server invalid, closing\n");
            close_socket();
            exit(EXIT_WRONG_MESSAGE);
        } else {
            fwrite(buffer.msg.data, 1, len, stdout);
            fwrite("\n", 1, 1, stdout);
            fflush(stdout);
            reset_buffer();
        }
    }
}

static void send_message() {
    if (streams[0].revents & (POLLIN | POLLHUP)) {
        char line[BUF_SIZE + 1];
        message msg;
        char* a = fgets(line, sizeof(line), stdin);
        if (!a) {
            finish = true;
            return;
        }
        uint16_t line_len = strlen(line);
        if (line_len == 1 && line[0] == '\n') {
            return;
        }
        if (line[line_len - 1] == '\n') {
            line_len -= 1;
        }
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
    create_socket();
    get_server_address(argc, argv);
    connect_to_server();
    initialize_streams();
    do {
        reset_revents();
        do_poll();
    }
    while (!finish);
    close_socket();
    exit(EXIT_SUCCESS);
}
