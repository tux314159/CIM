CC=gcc
DEBUG=-g -Wall
CFLAGS=$(DEBUG) -pthread -o $@

all : server client

server : server.c
	$(CC) $(CFLAGS) $<

client : client.c
	$(CC) $(CFLAGS) $<

clean :
	rm -f server client
