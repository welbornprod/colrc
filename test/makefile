# Makefile for test_colr
# -Christopher Welborn  06-10-2019

SHELL=bash
CC=gcc
CFLAGS=-Wall -Wextra -Wfloat-equal -Winline -Wlogical-op \
       -Wimplicit-fallthrough -Wlogical-not-parentheses \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused \
       -Wno-variadic-macros \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -DCOLR_TEST \
       -std=gnu11
INCLUDES=-iquote../
LIBS=-lm
# Sanitizers/protectors to optionally enable.
FFLAGS=-fno-omit-frame-pointer -fstack-protector-strong \
    -fsanitize=address -fsanitize=leak -fsanitize=undefined

binary=test_colrc
cov_cmd=bash ../tools/gen_coverage_html.sh --
cov_dir=coverage
cppcheck_cmd=bash ../tools/cppcheck_run.sh -t
cppcheck_html=./cppcheck_report/index.html
is_build_cmd=bash ../tools/is_build.sh $(binary)
make_help_fmt_cmd=python3 ../tools/make_help_fmter.py
valgrind_cmd=bash ../tools/valgrind_run.sh
test_source:=$(wildcard *.c)
source:=$(test_source) ../colr.c
test_headers:=$(wildcard *.h)
headers:=$(test_headers) ../colr.h
test_objects=$(test_source:.c=.o)
objects:=$(source:.c=.o)
binobjects:=$(notdir $(objects))

.PHONY: all
all: sanitize

.PHONY: coveragecompile
coveragecompile: clean
coveragecompile: cleancoverage
coveragecompile: CFLAGS+=-O0 -DDEBUG
coveragecompile: CFLAGS+=-fprofile-arcs -ftest-coverage
coveragecompile: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coveragecompile: $(binary)

.PHONY: coverage
coverage: coveragecompile
coverage:
	@$(cov_cmd) "$(realpath $(binary))" "$(realpath $(cov_dir))"

.PHONY: coveragegui
coveragegui: coveragecompile
coveragegui:
	@$(cov_cmd) --gui "$(realpath $(binary))" "$(realpath $(cov_dir))"

.PHONY: debug
debug: tags
debug: CFLAGS+=-gdwarf-4 -g3 -DDEBUG -DCOLR_DEBUG
debug: $(binary)

.PHONY: release
release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)

# Build the tests with all of the -fsanitize options. This will make the
# executable slower and bigger, but helps to catch things that valgrind doesn't.
# Valgrind cannot be used with this build, so the `memcheck` target will
# rebuild in `debug` mode if needed.
.PHONY: sanitize
sanitize: CFLAGS+=-g3 -DDEBUG  -DCOLR_DEBUG $(FFLAGS)
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
	$(CC) -c $$cfile -o $@ $(CFLAGS) $(INCLUDES)

tags: $(source) $(headers)
	@printf "Building ctags...\n    ";
	ctags $(source) $(headers);

.PHONY: clang
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

.PHONY: clean
clean:
	@bash ../tools/clean.sh "$(binary)"

.PHONY: cleanquiet
cleanquiet:
	@bash ../tools/clean.sh --quiet "$(binary)"

.PHONY: cleancoverage
cleancoverage:
	@shopt -s nullglob; \
	declare -a covfiles=($(cov_dir)/*.gc{da,no} $(cov_dir)/*.info $(cov_dir)/html $(cov_dir)/*.o); \
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

.PHONY: coveragesummary
coveragesummary:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --summary

.PHONY: coverageview
coverageview:
	@$(cov_cmd) "$(binary)" "$(cov_dir)" --view

.PHONY: cppcheck
cppcheck:
	@$(cppcheck_cmd)

.PHONY: cppcheckreport
cppcheckreport:
	@$(cppcheck_cmd) -r

.PHONY: cppcheckview
cppcheckview:
	@$(cppcheck_cmd) --view

.PHONY: memcheck
memcheck: debug
memcheck:
	@if $(is_build_cmd) "sanitize" -o "coverage" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized/coverage mode for memcheck.\n"; \
		$(MAKE) clean debug; \
	fi;
	@$(valgrind_cmd) -- $(TEST_ARGS)

.PHONY: memcheckquiet
memcheckquiet: debug
memcheckquiet:
	@if $(is_build_cmd) "sanitize" -o "coverage" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized/coverage mode for memcheck.\n"; \
		$(MAKE) clean debug; \
	fi;
	@$(valgrind_cmd) -q -- --quiet $(TEST_ARGS)

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
    cleancoverage   : Delete previous coverage files.\n\
    cleantest       : Delete previous build files, then build and run the tests.\n\
    coverage        : Compile the \`debug\` build, and generate coverage reports.\n\
    coveragecompile : Compile the \`debug\` build with coverage enabled.\n\
    coveragegui     : Compile the \`debug\` build, and view coverage with \`ggcov\`.\n\
    coveragesummary : View a summary of previously generated coverage reports.\n\
    coverageview    : View previously generated html coverage reports.\n\
    cppcheck        : Run \`cppcheck\` on the tests.\n\
    cppcheckreport  : Generate a \`cppcheck\` HTML report.\n\
    cppcheckview    : View previously generated cppcheck HTML report.\n\
    debug           : Build the executable with debug symbols.\n\
    memcheck        : Run \`valgrind --tool=memcheck\` on the executable.\n\
    release         : Build the executable with optimization, and strip it.\n\
    sanitize        : Build debug with \`-fsanitize\` options.\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Make \`sanitize\` (if needed), then run the tests.\n\
    testeverything  : Alias for \`./run_tests.sh --all --quiet\`.\n\
    testfast        : Make \`debug\` (if needed), then run the tests.\n\
                      This is used to build faster and fail faster.\n\
                      If the tests pass, the \`test\` target still needs to run\n\
                      to thoroughly test ColrC.\n\
    testfull        : Delete previous build files, build/run tests in \`debug\`\n\
                      mode, run tests for \`memcheck\`, and finally build\n\
                      \`sanitize\` target and run tests.\n\
    testgdb         : Make \`sanitize\` (if needed), then run the tests through GDB.\n\
    testkdbg        : Make \`sanitize\` (if needed), then run the tests through KDbg.\n\
    testquiet       : Make \`sanitize\` (if needed), then run the tests with --quiet.\n\
" | $(make_help_fmt_cmd);

.PHONY: cleantest
cleantest: clean
cleantest: test

.PHONY: test
test: sanitize
test:
	@./run_tests.sh $(TEST_ARGS)

.PHONY: testeverything
testeverything:
	@./run_tests.sh --all --quiet

.PHONY: testfast
testfast: clean
testfast: debug
testfast:
	@./run_tests.sh $(TEST_ARGS)

.PHONY: testfastquiet
testfastquiet: cleanquiet
testfastquiet: debug
testfastquiet:
	./run_tests.sh --quiet $(TEST_ARGS)

.PHONY: testfull
testfull: testfast memcheck
testfull:
	@$(MAKE) clean sanitize test

.PHONY: testfullquiet
testfullquiet: testfastquiet memcheckquiet
testfullquiet:
	@$(MAKE) cleanquiet sanitize testquiet

.PHONY: testgdb
testgdb: sanitize
testgdb:
	@./run_tests.sh -e gdb $(TEST_ARGS)

.PHONY: testkdbg
testkdbg: sanitize
testkdbg:
	@./run_tests.sh -e kdbg $(TEST_ARGS)

.PHONY: testquiet
testquiet: sanitize
testquiet:
	@./run_tests.sh --quiet $(TEST_ARGS)


