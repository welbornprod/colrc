# Makefile for colr_example
# -Christopher Welborn  07-01-2019

SHELL=bash
CC=gcc
CFLAGS=-Wall -Wextra -Wenum-compare -Wfloat-equal -Winline -Wlogical-op \
       -Wimplicit-fallthrough -Wlogical-not-parentheses \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -std=c11
CFLAGS+=-g3 -DDEBUG
LIBS=-lm
INCLUDES=-iquote../
example_source:=$(wildcard *.c)
source:=$(example_source) ../colr.c
example_headers:=$(wildcard *.h)
headers:=$(example_headers) ../colr.h
example_objects=$(example_source:.c=.o)
binobjects=$(example_objects) colr.o
objects:=$(source:.c=.o)
binaries=$(basename $(notdir $(example_objects)))

.PHONY: all
all: $(binaries)

$(binaries): $(binobjects)
	@binname="$(basename $@)"; \
	if [[ "$?" =~ "$${binname}.o" ]]; then \
		printf "\nCompiling example executable $$binname:\n    Caused by $?\n    "; \
		binobjects="$@.o colr.o"; \
		$(CC) -o $(basename $@) $(CFLAGS) $$binobjects $(LIBS) $(INCLUDES); \
	else \
		: # printf "\nNot compiling executable $$binname ($? changed).\n"; \
	fi;

colr.o: ../colr.c ../colr.h
	@printf "\nCompiling colr.c from parent dir for examples...\n    "; \
	$(CC) -c $< -o colr.o $(CFLAGS) $(INCLUDES)

$(example_objects): $(example_source) $(example_headers)
	@objpath="$@"; \
	cfile="$${objpath%.*}.c"; \
	if [[ "$?" =~ "$$cfile" ]]; then \
		printf "\nCompiling $$cfile for examples:\n    Caused by $?\n    "; \
		$(CC) -c $$cfile -o $@ $(CFLAGS) $(INCLUDES); \
	else \
		: # printf "\nNot compiling $$cfile ($? changed).\n"; \
	fi;

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: all

.PHONY: clean
clean:
	-@../clean.sh - $(binaries) $(example_objects) colr.o

.PHONY: help, targets
help targets:
	-@printf "Make targets available:\n\
	all       : Compile all examples.\n\
	clean     : Delete previous build files.\n\
	";
