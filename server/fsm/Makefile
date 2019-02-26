# Makefile for finite state machine

DIR = generate
MAIN_FILE = server.def
DOT_FILE = server.dot

fsm: $(MAIN_FILE)
	autogen $<
	python scripts/fsm2dot.py $< > $(DOT_FILE)