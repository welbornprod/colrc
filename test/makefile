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
       -std=gnu11
INCLUDES=-iquote../
LIBS=-lm
# Sanitizers/protectors to optionally enable.
FFLAGS=-fno-omit-frame-pointer -fstack-protector-strong \
    -fsanitize=address -fsanitize=leak -fsanitize=undefined

binary=test_colrc
cov_cmd=bash ../tools/gen_coverage_html.sh --
cov_dir=coverage
is_build_cmd=bash ../tools/is_build.sh
valgrind_cmd=bash ../tools/valgrind_run.sh
test_source:=$(wildcard *.c)
source:=$(test_source) ../colr.c
test_headers:=$(wildcard *.h)
headers:=$(test_headers) ../colr.h
test_objects=$(test_source:.c=.o)
objects:=$(source:.c=.o)
binobjects:=$(notdir $(objects))

.PHONY: all, coverage, debug, debugnotags, release, sanitize
all: sanitize

coverage: clean
coverage: coverageclean
coverage: CFLAGS+=-O0 -DDEBUG
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coverage: $(binary)
coverage:
	@$(cov_cmd) "$(realpath $(binary))" "$(realpath $(cov_dir))"

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

sanitize: CFLAGS+=-g3 -DDEBUG $(FFLAGS)
sanitize: LIBS+=-lasan
sanitize: $(binary)

$(binary): $(binobjects)
	@printf "\nCompiling test executable, $(binary)...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(binobjects) $(LIBS) $(INCLUDES)

colr.o: ../colr.c ../colr.h
	@printf "\nCompiling colr.c from parent dir for tests...\n    "
	$(CC) -c $< -o colr.o $(CFLAGS) $(INCLUDES)

$(test_objects): $(test_source) $(test_headers)
	@objpath="$@"; \
	cfile="$${objpath%.*}.c"; \
	printf "\nCompiling $$cfile for tests (caused by $@)...\n    "; \
	$(CC) -c $$cfile -o $@ $(CFLAGS) $(INCLUDES);

tags: $(source) $(headers)
	@printf "Building ctags...\n    ";
	ctags $(source) $(headers);

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

.PHONY: clean
clean:
	-@bash ../tools/clean.sh "$(binary)"

.PHONY: coverageclean, coveragesummary, coverageview
coverageclean:
	@shopt -s nullglob; \
	declare -a covfiles=($(cov_dir)/*.gc{da,no} $(cov_dir)/*.info $(cov_dir)/html); \
	if (($${#covfiles[@]})) && [[ "$${covfiles[*]}" != $(cov_dir)/html ]]; then \
		printf "Removing coverage files...\n"; \
		for covfile in "$${covfiles[@]}"; do \
			if rm -r "$$covfile" &>/dev/null; then \
				printf "    Cleaned: $$covfile\n"; \
			elif [[ -e "$$covfile" ]]; then \
				printf "     Failed: $$covfile\n"; \
			fi; \
		done; \
	else \
		printf "Coverage files already clean.\n"; \
	fi;
coveragesummary:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --summary

coverageview:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --view

.PHONY: memcheck, memcheckquiet

memcheck: debug
memcheck:
	@if $(is_build_cmd) "sanitize" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized mode for memcheck.\n"; \
		$(MAKE) $(MAKEFLAGS) clean debug; \
	fi;
	@$(valgrind_cmd) -- $(TEST_ARGS)

memcheckquiet: debug
memcheckquiet:
	@if $(is_build_cmd) "sanitize" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized mode for memcheck.\n"; \
		$(MAKE) $(MAKEFLAGS) clean debug; \
	fi;
	@$(valgrind_cmd) -q -- $(TEST_ARGS)

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
    clean           : Delete previous build files.\n\
    cleantest       : Delete previous build files, then build and run the tests.\n\
    coverage        : Compile the debug build and generate coverage reports.\n\
    coveragesummary : View a summary of previously generated coverage reports.\n\
    coverageview    : View previously generated html coverage reports.\n\
    debug           : Build the executable with debug symbols.\n\
    memcheck        : Run valgrind's memcheck on the executable.\n\
    release         : Build the executable with optimization, and strip it.\n\
    sanitize        : Build debug with \`-fsanitize\` options.\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Make debug (if needed), then run the tests.\n\
    testgdb         : Make debug (if needed), then run the tests through GDB.\n\
    testkdbg        : Make debug (if needed), then run the tests through KDbg.\n\
    testquiet       : Make debug (if needed), then run the tests with --quiet.\n\
	";

.PHONY: cleantest, testgdb, testkdbg, testquiet, test
cleantest: clean
cleantest: test

test: sanitize
test:
	-@./run_tests.sh $(TEST_ARGS)

testgdb: sanitize
testgdb:
	-@./run_tests.sh -e gdb $(TEST_ARGS)

testkdbg: sanitize
testkdbg:
	-@./run_tests.sh -e kdbg $(TEST_ARGS)

testquiet: sanitize
testquiet:
	-@./run_tests.sh $(TEST_ARGS) --quiet


