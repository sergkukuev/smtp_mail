# Makefile for server
CC = gcc
STD = -std=gnu99

RFLAGS = -Wall -o
DFLAGS = -Wall -g3
# Postfix
PF = -lrt -lpthread -lresolv
# Common
CF = -D_XOPEN_SOURCE

all: clean release

release: server_r
debug: server_d

# Launch server
server_r:
	$(CC) $(STD) $(RFLAGS) sockets.c smtp.c server.c logger.c common.c process.c handlers.c -o server $(PF)

server_d:
	$(CC) $(STD) $(DFLAGS) sockets.c smtp.c server.c logger.c common.c process.c handlers.c -o server $(PF)

# Launch tests
valgrind_test:
	valgrind --leak-check=full --leak-resolution=med --quiet --log-file=valgrind_log ./server

clean:
	rm -rf *.o server client