#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util.h"

#define LISTEN_PORT "5929"
#define BACKLOG 10

int main(void)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, LISTEN_PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int mainsock;
    PERR((mainsock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1, "sock");
    PERR(bind(mainsock, res->ai_addr, res->ai_addrlen), "bind");
    PERR(listen(mainsock, BACKLOG), "listen");

    int childsock;
    struct sockaddr_storage childconn_info;
    socklen_t childconn_sz;
    PERR(
        (childsock = accept(mainsock, (struct sockaddr *)&childconn_info, &childconn_sz)) == -1,
        "listen"
    );

    char buf[256];
    for (;;) {
        memset(buf, 0, sizeof(buf));
        recv(childsock, &buf, sizeof(buf), 0);
        printf("%s\n", buf);
    }

    freeaddrinfo(res);
    close(mainsock);
}
