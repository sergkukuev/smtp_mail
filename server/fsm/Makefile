# Makefile for finite state machine

DIR = generate
MAIN_FILE = server.def
DOT_FILE = server.dot
TEX_FILE = server.tex

fsm: $(MAIN_FILE)
	autogen $<
	python scripts/fsm2dot.py $< > $(DOT_FILE)
	dot2tex -ftikz --autosize --crop $(DOT_FILE) > $(TEX_FILE)