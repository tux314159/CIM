#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util.h"

#define CONN_PORT "5929"

int main(int argc, char **argv)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(argv[1], CONN_PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sock;
    PERR((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1, "socket");
    PERR(connect(sock, res->ai_addr, res->ai_addrlen), "connect");

    for (;;) {
        printf("> ");
        size_t bufsz = 0;
        char *buf = NULL;
        getline(&buf, &bufsz, stdin);
        send(sock, buf, strlen(buf) - 1, 0);
        free(buf);
    }

    freeaddrinfo(res);
    close(sock);
}
