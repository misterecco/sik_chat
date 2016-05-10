#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

#define MAX_CLIENTS 21

static int finish = false;
static int connection_port = DEFAULT_PORT;
static int active_clients = 0;
static struct pollfd client[MAX_CLIENTS];
static struct sockaddr_in server;
static int msgsock;
static struct buffer_state buffers[MAX_CLIENTS];

static void reset_client_buffer(int client_number) {
    buffers[client_number].data_read = 0;
    buffers[client_number].length_read = 0;
}

static void initialize_clients_array() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client[i].fd = -1;
        client[i].events = POLLIN;
        client[i].revents = 0;
        reset_client_buffer(i);
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

static void close_client_socket(int client_number) {
    if (close(client[client_number].fd) < 0) {
        perror("close");
    }
    client[client_number].fd = -1;
    active_clients -= 1;
    reset_client_buffer(client_number);
};

static void broadcast_message(int client_number, ssize_t size, message *msg) {
    for (int i = 1; i < MAX_CLIENTS; i++) {
        if (client[i].fd != -1  && i != client_number) {
            if (write(client[i].fd, msg, size) < 0) {
                perror("writing on stream socket");
            }
        }
    }
}

static void handle_client_messages() {
    size_t len;
    ssize_t rval;
    for (int i = 1; i < MAX_CLIENTS; ++i) {
        if (client[i].fd != -1 && (client[i].revents & (POLLIN | POLLERR | POLLHUP))) {
            if (buffers[i].length_read < 2) {
                rval = read(client[i].fd,
                            (char *)&(buffers[i].msg.len) + buffers[i].length_read,
                            2 - buffers[i].length_read);
                if (rval == 0) {
                    close_client_socket(i);
                    continue;
                } else if (rval > 0) {
                    buffers[i].length_read += rval;
                } else {
                    perror("Reading stream message");
                    close_client_socket(i);
                    continue;
                }
            }
            if (buffers[i].length_read < 2) {
                continue;
            }
            len = ntohs(buffers[i].msg.len);
            if (len > BUF_SIZE) {
                close_client_socket(i);
                continue;
            }
            rval = read(client[i].fd,
                        buffers[i].msg.data + buffers[i].data_read,
                        len - buffers[i].data_read);
            if (rval < 0) {
                perror("Reading stream message");
                close_client_socket(i);
                continue;
            }
            buffers[i].data_read += rval;
            if (buffers[i].data_read < len) {
                continue; // wait for the rest of the message
            }
            if (!is_message_valid(&(buffers[i].msg), len)) {
                close_client_socket(i);
            } else {
                broadcast_message(i, len + sizeof(uint16_t), &(buffers[i].msg));
                reset_client_buffer(i);
            }
        }
    }
}

static void do_poll() {
    int ret = poll(client, MAX_CLIENTS, POLL_REFRESH_TIME);
    if (ret < 0) {
        perror("poll");
    }
    else if (ret > 0) {
        accept_new_client();
        handle_client_messages();
    }
}

int main (int argc, char** argv) {

    validate_arguments_and_set_connection_port(argc, argv, 2,
                                               &connection_port, "[port]");
    initialize_clients_array();
    create_central_socket();
    bind_port_to_socket();
    listen_on_central_socket();

    do {
        reset_revents();
        close_central_socket_if_necessary();
        do_poll();
    } while (finish == false || active_clients > 0);

    close_central_socket_if_necessary();
    exit(EXIT_SUCCESS);
}
