#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

struct addrinfo hints;
struct addrinfo *servinfo;
int sockfd;
char buf[256];

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <address> <port>\n", argv[0]);
		exit(1);
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(argv[1], argv[2], &hints, &servinfo);

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	printf("CLIENT: connecting to server...\n");
	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		printf("ERROR: unable to establish connection.\n");
		exit(1);
	}
	printf("CLIENT: connected to server.\n");

	scanf("%[^\n]s", buf);
	write(sockfd, buf, strlen(buf));
	memset(&buf, 0, sizeof(buf));
	read(sockfd, buf, 255);
	printf("%s\n", buf);

	/* Clean up */
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}
