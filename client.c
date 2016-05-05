#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include "err.h"
#include "common.h"

static int sock;
static struct addrinfo addr_hints, *addr_result;
static bool finish = false;
static int connection_port = DEFAULT_PORT;

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

static void get_server_address(char **argv) {
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_flags = 0;
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;

    int rc =  getaddrinfo(argv[1], argv[2], &addr_hints, &addr_result);
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

static void send_message() {
    char line[BUF_SIZE];
    message msg;
    printf(">:");
    fgets(line, sizeof line, stdin);
    //TODO: handle lines ending with LF CR etc
    uint16_t line_len = strlen(line) - 1;
    msg.len = htons(line_len);
    strncpy(msg.data, line, line_len);
    printf("Message length: %d\n", line_len);
    printf("Message data: %s\n", msg.data);
    if (write(sock, &msg, sizeof(uint16_t) + line_len) < 0) {
        perror("writing on stream socket");
    }
}

int main (int argc, char **argv) {

    validate_arguments_and_set_connection_port(argc, argv, 3, &connection_port,
                                               "hostname [port]");
    set_sigint_behaviour(catch_int);
    create_socket();
    get_server_address(argv);
    connect_to_server();

    do {
        send_message();
    }
    while (!finish);
    if (close(sock) < 0)
        perror("closing stream socket");

    return 0;
}
