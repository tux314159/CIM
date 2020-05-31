#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFMAX 256
#define STACKMAX 100
#define PROT  PROT_READ | PROT_WRITE
#define FLAGS MAP_SHARED | MAP_ANONYMOUS

#define STREQ(x, y) (!strcmp(x, y))

struct srv_status {
	size_t stacksz;
	char **msgstack;
	bool term;
};

struct sockaddr_storage cliaddr;
socklen_t clilen = sizeof(cliaddr);
struct addrinfo hints;
struct addrinfo *servinfo;
int sockfd, clifd, temp;

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "ERROR: Not enough arguments!\n");
		exit(1);
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, argv[1], &hints, &servinfo);

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	printf("SERVER: created socket.\n");
	bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	printf("SERVER: bound socket.\n");

	listen(sockfd, 5);

	/* Allocate memory */
	struct srv_status *stat = mmap(NULL, sizeof(struct srv_status), PROT, FLAGS, -1, 0);
	/* stat->msgstack = mmap(NULL, sizeof(char*) * STACKMAX, PROT, FLAGS, -1, 0); */
	stat->term = 0;
	stat->stacksz = 0;
	stat->msgstack = malloc(sizeof(char*) * STACKMAX);
	for (int i = 0; i < STACKMAX; i++)
		stat->msgstack[i] = calloc(BUFMAX, sizeof(char));
		/* stat->msgstack[i] = mmap(NULL, sizeof(char) * BUFMAX, PROT, FLAGS, -1, 0); */

	while (!stat->term) {
		printf("SERVER: waiting for connection...\n");
		clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		printf("SERVER: connection found.\n");
		printf("SERVER: forking instance to handle new connection.\n");

		/* Here we go! */
		pid_t pid = fork();

		if (pid > 0) {
			close(clifd);
			int status; pid_t pid = waitpid(pid, &status, WNOHANG);
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == 1)
					stat->term = true;
			}
			continue;
		}

		else if (pid == 0) {
			char cmdbuf[5];
			char buf[BUFMAX];
			memset(buf, 0, sizeof(buf));

			read(clifd, cmdbuf, 4);
			if (false) {
			}else if (STREQ(cmdbuf, "SEND")) {
				recv(clifd, buf, BUFMAX - 1, 0);
				strcpy(stat->msgstack[stat->stacksz++], buf);
				send(clifd, "RECV", 4, 0);

			} else if (STREQ(cmdbuf, "GETN")) { /* GET Number */
				recv(clifd, buf, sizeof(buf), 0);
				temp = atoi(buf);
				if (temp >= stat->stacksz)
					write(clifd, "INDO", 4); /* INDex Over */
				else if (temp < 0)
					write(clifd, "INDU", 4); /* INDex Under */
				else {
					strcat(buf, "RMSG"); /* Return MeSsaGe */
					strcat(buf, stat->msgstack[temp]);
					send(clifd, buf + 1, sizeof(buf) - 1, 0);
				}

			} else if (STREQ(cmdbuf, "GETL")) { /* GET Last */
				if (stat->stacksz == 0)
					write(clifd, "INDO", 4);
				else {
					strcat(buf, "RMSG");
					strcat(buf, stat->msgstack[stat->stacksz - 1]);
					send(clifd, buf, sizeof(buf), 0);
				}
			} else if (STREQ(cmdbuf, "GSSZ")) { /* Get Stack SiZe */
				char t[4 + 4];
				sprintf(t, "RSSZ%lu", stat->stacksz); /* Return Stack SiZe */
				send(clifd, t, sizeof(t), 0);

			} else if (STREQ(cmdbuf, "TERM")) {
				printf("SERVER: CHILD: Got TERM, terminating...\n");
				send(clifd, "TERM", 4, 0);
				return 0;

			} else {
				send(clifd, "WHAT", 4, 0);
			}

			printf("SERVER: closing connection...\n");
			close(clifd);
			printf("SERVER: connection closed.\n");
			return 0;
		}
	}

	/* Clean up */
	close(sockfd);
	freeaddrinfo(servinfo);
	for (int i = 0; i < STACKMAX; i++)
		free(stat->msgstack[i]);
	free(stat->msgstack);
	munmap(stat, sizeof(struct srv_status));
	return 0;
}
