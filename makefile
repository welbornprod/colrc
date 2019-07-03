# Makefile for colr_tool
# -Christopher Welborn  06-10-2019

SHELL=bash
CC=gcc
CFLAGS=-Wall -Wextra -Wfloat-equal -Wenum-compare -Winline -Wlogical-op \
       -Wimplicit-fallthrough -Wlogical-not-parentheses \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -std=c11

LIBS=-lm

binary=colr
colr_source=colr.c
source=colr_tool.c $(colr_source)
colr_headers=colr.h
headers=colr_tool.h $(colr_headers)
optional_headers=dbug.h
optional_flags=$(foreach header, $(optional_headers), -include $(header))
cov_dir=coverage
docsconfig=Doxyfile
docsdir=docs
docsreadme=README.md
docsmainfile=$(docsdir)/html/index.html
docscss=doc_style/customdoxygen.css
docsexamples=$(wildcard examples/*.doc) $(wildcard examples/*.c)
docsdepfiles=$(docsconfig) $(docsreadme) $(docsexamples) $(docscss)
examples_dir=examples
examples_source=$(wildcard $(examples_dir)/*.c)
objects:=$(source:.c=.o)

.PHONY: all, coverage, debug, release
all: debug

coverage: CFLAGS+=-O0 -DDEBUG
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coverage: $(binary)
coverage:
	@./gen_coverage_html.sh "$(realpath $(binary))" "$(realpath $(cov_dir))" $(COLR_ARGS)

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

$(binary): $(objects)
	@printf "\nCompiling $(binary) executable...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(objects) $(LIBS)

%.o: %.c %.h
	@printf "\nCompiling $<...\n    ";
	$(CC) -c $< $(CFLAGS)

$(docsmainfile): $(source) $(headers) $(docsdepfiles)
docs: $(source) $(headers) $(docsmainfile) $(docsdepfiles)
	@printf "\nBuilding doxygen docs (for $?)...\n    "
	doxygen $(docsconfig);

tags: $(source) $(headers)
	@printf "Building ctags...\n    "
	ctags $(source) $(headers)

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

clangrelease: CC=clang
clangrelease: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clangrelease: release

.PHONY: clean
clean:
	@./clean.sh "$(binary)"

.PHONY: cleandebug
cleandebug: clean
cleandebug: debug

.PHONY: cleandocs
cleandocs:
	@./clean.sh -d "$(docsdir)" "$(docsmainfile)"

.PHONY: cleanexamples
cleanexamples:
	@cd examples && $(MAKE) $(MAKEFLAGS) --no-print-directory clean

.PHONY: coveragesummary
coveragesummary:
	@./gen_coverage_html.sh "$(binary)" "$(cov_dir)" --summary

.PHONY: coverageview
coverageview:
	@./gen_coverage_html.sh "$(binary)" "$(cov_dir)" --view

.PHONY: docsrebuild
docsrebuild: cleandocs
docsrebuild: docs

.PHONY: examples
examples: $(examples_source)
	@cd examples && $(MAKE) $(MAKEFLAGS) --no-print-directory $(COLR_ARGS)

.PHONY: memcheck
memcheck:
	@./run_valgrind.sh -a $(COLR_ARGS)

.PHONY: run
run:
	-@if [[ -e $(binary) ]]; then \
		./$(binary) $$COLR_ARGS; \
	else \
		printf "No binary built yet: $(binary)" 1>&2; \
	fi; \

.PHONY: runexamples
runexamples:
	@cd examples && ./run_examples.sh $(COLR_ARGS)

.PHONY: strip
strip:
	@if strip $(binary); then \
		printf "\n%s was stripped.\n" "$(binary)"; \
	else \
		printf "\nError stripping executable: %s\n" "$(binary)" 1>&2; \
	fi;

.PHONY: help, targets
help targets:
	-@printf "Make targets available:\n\
    all             : Build with no optimization or debug symbols.\n\
    clang           : Use \`clang\` to build the default target.\n\
    clangrelease    : Use \`clang\` to build the release target.\n\
    clean           : Delete previous build files.\n\
    cleandebug      : Like running \`make clean debug\`.\n\
    cleandocs       : Delete Doxygen docs from ./$(docsdir).\n\
    cleanexamples   : Delete previous build files from the examples in $(examples_dir).\n\
    cleantest       : Delete previous build files, build the binary and the \n\
                      test binary, and run the tests.\n\
    coverage        : Compile the debug build and generate coverage reports.\n\
                      This only checks the main binary, not the tests.\n\
                      See the \`testcoverage\` target.\n\
    coveragesummary : View a summary of previously generated coverage reports.\n\
                      This is only for the main binary, not the tests.\n\
                      See the \`testsummary\` target.\n\
    coverageview    : View previously generated html coverage reports.\n\
                      This is only for the main binary, not the tests.\n\
                      See the \`testview\` target.\n\
    debug           : Build the executable with debug symbols.\n\
    docs            : Build the Doxygen docs.\n\
    docsrebuild     : Like running \`make cleandocs docs\`\n\
    examples        : Build example executables in $(examples_dir).\n\
    release         : Build the executable with optimization, and strip it.\n\
    run             : Run the executable. Args are set with COLR_ARGS.\n\
    runexamples     : Run the example executables in $(examples_dir).\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests.\n\
    memcheck        : Run valgrind's memcheck on the executable.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory clean debug && { cd test; $(MAKE) $(MAKEFLAGS) --no-print-directory cleantest; };

test:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { cd test; $(MAKE) $(MAKEFLAGS) --no-print-directory test; };

.PHONY: testcoverage, testsummary, testview
testcoverage:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory clean coverage

testsummary:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coveragesummary

testview:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coverageview

