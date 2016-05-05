#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err.h"
#include "comm.h"

static const char bye_string[] = "BYE";
static int sock;
static struct addrinfo addr_hints, *addr_result;


static void validate_arguments(int argc, char **argv) {
    if (argc > 3 || argc < 2) {
        printf("Usage: %s host [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
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

    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;

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

int main (int argc, char **argv) {
    char line[BUF_SIZE];

    validate_arguments(argc, argv);
    create_socket();
    get_server_address(argv);
    connect_to_server();

    do {
        printf(">:");
        fgets(line, sizeof line, stdin);
        if (write(sock, line, strlen (line)) < 0)
            perror("writing on stream socket");
    }
    while (strncmp(line, bye_string, sizeof bye_string - 1));
    if (close(sock) < 0)
        perror("closing stream socket");

    return 0;
}

/*EOF*/
