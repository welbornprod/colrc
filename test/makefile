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
cov_cmd=bash ../gen_coverage_html.sh --
cov_dir=coverage
valgrind_cmd=bash ../run_valgrind.sh --
test_source:=$(wildcard *.c)
source:=$(test_source) ../colr.c
test_headers:=$(wildcard *.h)
headers:=$(test_headers) ../colr.h
test_objects=$(test_source:.c=.o)
objects:=$(source:.c=.o)
binobjects:=$(notdir $(objects))

test_runner=run_tests.py

.PHONY: all, coverage, debug, debugnotags, release
all: debug

coverage: CFLAGS+=-O0 -DDEBUG
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coverage: $(binary)
coverage:
	@$(cov_cmd) "$(realpath $(binary))" "$(cov_dir)"

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

$(binary): $(binobjects)
	@printf "\nCompiling test executable, $(binary)...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(binobjects) $(LIBS) $(INCLUDES)

colr.o: ../colr.c ../colr.h
	@printf "\nCompiling colr.c from parent dir for tests...\n    "
	$(CC) -c $< -o colr.o $(CFLAGS) $(LIBS) $(INCLUDES)

$(test_objects): $(test_source) $(test_headers)
	@objpath="$@"; \
	cfile="$${objpath%.*}.c"; \
	printf "\nCompiling $$cfile for tests (caused by $@)...\n    "; \
	$(CC) -c $$cfile -o $@ $(CFLAGS) $(LIBS) $(INCLUDES);

tags: $(source) $(headers)
	@printf "Building ctags...\n    ";
	ctags $(source) $(headers);

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

.PHONY: clean
clean:
	-@bash ../clean.sh "$(binary)"

.PHONY: coveragesummary
coveragesummary:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --summary

.PHONY: coverageview
coverageview:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --view

.PHONY: memcheck
memcheck:
	@$(valgrind_cmd) -a $(TEST_ARGS)

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
    all             : Build with no optimization or debug symbols.\n\
    clang           : Use \`clang\` to build the default target.\n\
    clangrelease    : Use \`clang\` to build the release target.\n\
    clean           : Delete previous build files.\n\
    cleantest       : Delete previous build files, then build and run the tests.\n\
    coverage        : Compile the debug build and generate coverage reports.\n\
    coveragesummary : View a summary of previously generated coverage reports.\n\
    coverageview    : View previously generated html coverage reports.\n\
    debug           : Build the executable with debug symbols.\n\
    memcheck        : Run valgrind's memcheck on the executable.\n\
    release         : Build the executable with optimization, and strip it.\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Make debug (if needed) and then run the tests.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@$(MAKE) --no-print-directory clean debug && \
		printf "\nRunning $(binary) (via $(test_runner))...\n    "; \
		./$(test_runner);

test:
	-@$(MAKE) --no-print-directory debug; \
		printf "\nRunning $(binary) (via $(test_runner))...\n    "; \
		./$(test_runner);

