CC=gcc
DEBUG=-g
CFLAGS=$(DEBUG) -o $@

all : server client

server : server.c
	$(CC) $(CFLAGS) $<

client : client.c
	$(CC) $(CFLAGS) $<
