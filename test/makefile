# Makefile for test_colr
# -Christopher Welborn  06-10-2019

SHELL=bash
CC=gcc
CFLAGS=-Wall -Wextra -Wfloat-equal -Winline -Wlogical-op \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused-macros \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -std=c11
INCLUDES=-iquote../
LIBS=-lcmocka -lm

binary=test_colr
source=test_colr.c
headers=../colr.h
objects:=$(source:.c=.o)

.PHONY: all, debug, debugnotags, release
all: debug

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

$(binary): $(objects)
	$(CC) -o $(binary) $(CFLAGS) $(objects) $(LIBS) $(INCLUDES)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) $(LIBS) $(INCLUDES)


tags: $(source) $(headers)
	-@printf "Building ctags...\n";
	ctags $(source) $(headers);

.PHONY: clean
clean:
	-@if [[ -e $(binary) ]]; then\
		if rm -f $(binary); then\
			printf "Binaries cleaned:\n    $(binary)\n";\
		fi;\
	else\
		printf "Binaries already clean:\n    $(binary)\n";\
	fi;

	-@if ls $(objects) &>/dev/null; then\
		if rm $(objects); then\
			printf "Objects cleaned:\n";\
			printf "    %s\n" $(objects);\
		fi;\
	else\
		printf "Objects already clean:\n";\
		printf "    %s\n" $(objects);\
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
	-@make --no-print-directory clean debug && ./run_tests.py;

test:
	-@make --no-print-directory debug && ./run_tests.py;

