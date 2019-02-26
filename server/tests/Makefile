#Makefile for server tests
CC = gcc
STD = -std=gnu99

RFLAGS = -Wall -o
DFLAGS = -Wall -g3
# Postfix
PF = -lrt -lpthread -lresolv -lcunit
# Common
CF = -D_XOPEN_SOURCE
# All files 
FILES = ../src/sockets.c ../src/smtp.c ../src/logger.c ../src/common.c ../src/process.c ../src/handlers.c
TESTS = unit/test_main.c unit/test_handlers.c

all: clean release

release: server_r
debug: server_d


# Launch tests
run_valgrind:
	valgrind --leak-check=full --leak-resolution=med --quiet --log-file=valgrind_log ../src/server.out

unit_tests:
	$(CC) $(STD) $(DFLAGS) $(FILES) $(TESTS) unit_tests.c -o unit_tests.out $(PF)

clean:
	rm -rf *.o unit_tests.out