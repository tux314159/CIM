#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <semaphore.h>
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
	sem_t sem;
};

struct sockaddr_storage cliaddr;
socklen_t clilen = sizeof(cliaddr);
struct addrinfo hints;
struct addrinfo *servinfo;
int sockfd, clifd, temp;
static volatile bool term = false;
pid_t pid = 1;

void int_handler(int _) {
	term = true;
	shutdown(sockfd, SHUT_RDWR);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "ERROR: Not enough arguments!\n");
		exit(1);
	}

	/* Trap interrupts */
	struct sigaction act;
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, argv[1], &hints, &servinfo);

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	printf("SERVER: created socket.\n");
	if(!bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen))
		printf("SERVER: bound socket.\n");

	listen(sockfd, 5);

	/* Allocate memory */
	struct srv_status *stat = mmap(NULL, sizeof(struct srv_status), PROT, FLAGS, -1, 0);
	stat->stacksz = 0;
	stat->msgstack = mmap(NULL, sizeof(char*) * STACKMAX, PROT, FLAGS, -1, 0);
	for (int i = 0; i < STACKMAX; i++)
		stat->msgstack[i] = mmap(NULL, sizeof(char) * BUFMAX, PROT, FLAGS, -1, 0);

	sem_init(&stat->sem, true, 1);

	while (!term) {
		printf("SERVER: waiting for connection...\n");
		clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		if (term) {
			printf("SERVER: Got SIGINT, shutting down...\n");
			continue;
		}

		printf("SERVER: connection found.\n");
		printf("SERVER: forking instance to handle new connection.\n");

		/* Here we go! */
		pid = fork();

		if (pid > 0) {
			close(clifd);
			continue;
		}

		else if (pid == 0) {
			close(sockfd);
			char cmdbuf[5];
			char buf[BUFMAX];
			memset(buf, 0, sizeof(buf));

			read(clifd, cmdbuf, 4);
			if (STREQ(cmdbuf, "SEND"))
				goto SEND;
			else if (STREQ(cmdbuf, "GETN")) /* GET Number */
				goto GETN;
			else if (STREQ(cmdbuf, "GETL")) /* GET Last */
				goto GETL;
			else if (STREQ(cmdbuf, "GSSZ")) /* Get Stack SiZe */
				goto GSSZ;
			else
				goto WHAT;

#include "routines.c" /* THAT HORRIBLE HACK */

			printf("SERVER: closing connection...\n");
			close(clifd);
			printf("SERVER: connection closed.\n");
			return 0;
		}
	}

	/* Clean up */
	close(sockfd);
	close(clifd);
	freeaddrinfo(servinfo);
	for (int i = 0; i < STACKMAX; i++)
		munmap(stat->msgstack[i], sizeof(char) * BUFMAX);
	munmap(stat->msgstack, sizeof(char*) * STACKMAX);
	munmap(stat, sizeof(struct srv_status));
	return 0;

}
