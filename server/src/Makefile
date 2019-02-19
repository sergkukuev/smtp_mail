# Makefile for server
C = gcc
STD = -std=c99
RELEASE = -Wall -o
DEBUG = -Wall -g

all: clean server client

# Launch server
server:
	$(C) $(STD) $(DEBUG) server.c -o server

# Launch test client
client:
	$(C) $(STD) $(DEBUG) client.c -o client

# Clean objective files
clean:
	rm -rf *.o server client