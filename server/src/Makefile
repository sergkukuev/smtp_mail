# Makefile for server
C = gcc
STD = -std=c99
RELEASE = -Wall -o
DEBUG = -Wall -g

all: clean server

# Launch server
server:
	$(C) $(STD) $(DEBUG) server.c -o server

# Clean objective files
clean:
	rm -rf *.o server