# Main makefile
all: 
	+$(MAKE) -C src
	+$(MAKE) -C tests
	+$(MAKE) -C fsm

server:
	+$(MAKE) -C src debug

fsm:
	+$(MAKE) -C fsm

run_valgrind:
	+$(MAKE) -C tests run_valgrind

unit_tests:
	+$(MAKE) -C tests unit_tests

clean:
	+$(MAKE) -C src clean
	+$(MAKE) -C tests clean