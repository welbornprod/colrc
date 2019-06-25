# Makefile for test_colr
# -Christopher Welborn  06-10-2019

SHELL=bash
CC=gcc
CFLAGS=-Wall -Wextra -Wfloat-equal -Winline -Wlogical-op \
       -Wimplicit-fallthrough -Wlogical-not-parentheses \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -std=c11
INCLUDES=-iquote../
LIBS=-lcmocka -lm

binary=test_colr
source=test_colr.c ../colr.c
headers=../colr.h
objects:=$(source:.c=.o)
binobjects:=$(notdir $(objects))
test_runner=run_tests.py

.PHONY: all, debug, debugnotags, release
all: debug

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

$(binary): $(binobjects)
	@printf "\nCompiling test executable, $(binary)...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(binobjects) $(LIBS) $(INCLUDES)

colr.o: ../colr.c
	@printf "\nCompiling colr.c for tests...\n    "
	$(CC) -c $< -o colr.o $(CFLAGS) $(LIBS) $(INCLUDES)
test_colr.o: test_colr.c
	@printf "\nCompiling test_colr.c for tests...\n    "
	$(CC) -c $< $(CFLAGS) $(LIBS) $(INCLUDES)



tags: $(source) $(headers)
	@printf "Building ctags...\n    ";
	ctags $(source) $(headers);

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

.PHONY: clean
clean:
	-@if [[ -e $(binary) ]]; then\
		if rm -f $(binary); then\
			printf "\nTest binaries cleaned:\n    $(binary)\n";\
		fi;\
	else\
		printf "\nTest binaries already clean:\n    $(binary)\n";\
	fi;

	-@if ls $(binobjects) &>/dev/null; then\
		if rm $(binobjects); then\
			printf "\nTest objects cleaned:\n";\
			printf "    %s\n" $(binobjects);\
		fi;\
	else\
		printf "\nTest objects already clean:\n";\
		printf "    %s\n" $(binobjects);\
	fi;

.PHONY: strip
strip:
	@if strip $(binary); then\
		printf "\n%s was stripped.\n" "$(binary)";\
	else\
		printf "\nError stripping executable: %s\n" "$(binary)" 1>&2;\
	fi;

.PHONY: help, targets
help targets:
	-@printf "Make targets available:\n\
    all       : Build with no optimization or debug symbols.\n\
    clang        : Use \`clang\` to build the default target.\n\
    clangrelease : Use \`clang\` to build the release target.\n\
    clean     : Delete previous build files.\n\
    cleantest : Delete previous build files, then build and run the tests.\n\
    debug     : Build the executable with debug symbols.\n\
    release   : Build the executable with optimization, and strip it.\n\
    strip     : Run \`strip\` on the executable.\n\
    tags      : Build tags for this project using \`ctags\`.\n\
    test      : Make debug (if needed) and then run the tests.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@make --no-print-directory clean debug && \
		printf "\nRunning $(binary) (via $(test_runner))...\n" && \
		./$(test_runner);

test:
	-@make --no-print-directory debug && \
		printf "\nRunning $(binary) (via $(test_runner))...\n" && \
		./$(test_runner);

