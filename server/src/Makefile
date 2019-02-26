# Makefile for server
CC = gcc
STD = -std=gnu99

RFLAGS = -Wall -o
DFLAGS = -Wall -g3
# Postfix
PF = -lrt -lpthread -lresolv
# Common
CF = -D_XOPEN_SOURCE
# All files
FILES = sockets.c smtp.c logger.c common.c process.c handlers.c

all: clean release

release: server_r
debug: server_d

# Launch server
server_r:
	$(CC) $(STD) $(RFLAGS) $(FILES) server.c -o server.out $(PF)

server_d:
	$(CC) $(STD) $(DFLAGS) $(FILES) server.c -o server.out $(PF)

clean:
	rm -rf *.o server.out