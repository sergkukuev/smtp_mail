# Makefile for server
CC = gcc
STD = -std=gnu99

RFLAGS = -Wall -o
DFLAGS = -Wall -g3
# Postfix
PF = -lrt -lpthread -lresolv
# Common
CF = -D_XOPEN_SOURCE

all: server_r

debug: server_d client_d

# Launch server
server_r:
	$(CC) $(STD) $(RFLAGS) sockets.c server.c process.c handlers.c -o server $(PF)

server_d:
	$(CC) $(STD) $(DFLAGS) sockets.c server.c process.c handlers.c -o server $(PF)

# Launch test client
client_d:
	$(CC) $(STD) $(DFLAGS) client.c -o client $(PF)

clean:
	rm -rf *.o server client