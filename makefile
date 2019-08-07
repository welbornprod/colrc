# Makefile for colr_tool
# -Christopher Welborn  06-10-2019
# vim: set ts=4 sw=4
SHELL=bash
# This is only for testing standard differences. ColrC must use at least c11/gnu11.
STD=c11
# The flags will be different for clang, so just use the `clang` target (`make clang`).
CC=gcc
# More warnings may be added, but none should be taken away just to get a clean compile.
CFLAGS=-Wall -Wextra -Wfloat-equal -Wenum-compare -Winline -Wlogical-op \
       -Wimplicit-fallthrough -Wlogical-not-parentheses \
       -Wmissing-include-dirs -Wnull-dereference -Wpedantic -Wshadow \
       -Wstrict-prototypes -Wunused \
       -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
       -D_GNU_SOURCE \
       -std=$(STD)
# ColrC uses libm right now, but it's pretty standard.
LIBS=-lm

binary=colrc
colr_source=colr.c
source=colr_tool.c $(colr_source)
objects:=$(source:.c=.o)
colr_headers=colr.h
headers=colr_tool.h $(colr_headers)
optional_headers=dbug.h
optional_flags=$(foreach header, $(optional_headers), -include $(header))
cov_dir=coverage
custom_dir=doc_style
docs_config=Doxyfile_common
docs_html_config=Doxyfile_html
docs_latex_config=Doxyfile_latex
docs_dir=docs
docs_readme=README.md
docs_main_file=$(docs_dir)/html/index.html
docs_css=$(custom_dir)/customdoxygen.css
docs_examples=$(wildcard examples/*.c)
docs_deps=$(docs_config) $(docs_html_config) $(docs_readme) $(docs_examples) $(docs_css)
docs_pdf=$(docs_dir)/ColrC-manual.pdf
latex_dir=$(docs_dir)/latex
latex_header=$(custom_dir)/header.tex
latex_style=$(custom_dir)/doxygen.sty
latex_deps=$(docs_config) $(docs_latex_config) $(docs_readme) $(latex_header) $(latex_style)
latex_tex=$(latex_dir)/refman.tex
latex_pdf=$(latex_dir)/refman.pdf
doxy_latex_files=\
	$(wildcard $(latex_dir)/*.tex) $(wildcard $(latex_dir)/*.md5) \
	$(wildcard $(latex_dir)/*.pdf) $(wildcard $(latex_dir)/*.sty) \
	$(latex_dir)/Makefile
latex_files=\
	$(wildcard $(latex_dir)/*.ps) $(wildcard $(latex_dir)/*.dvi) \
	$(wildcard $(latex_dir)/*.aux) $(wildcard $(latex_dir)/*.toc) \
	$(wildcard $(latex_dir)/*.idx) $(wildcard $(latex_dir)/*.ind) \
	$(wildcard $(latex_dir)/*.ilg) $(wildcard $(latex_dir)/*.log) \
	$(wildcard $(latex_dir)/*.out) $(wildcard $(latex_dir)/*.brf) \
	$(wildcard $(latex_dir)/*.blg) $(wildcard $(latex_dir)/*.bbl) \
	$(wildcard $(latex_dir)/*.pyg) $(latex_dir)/refman.pdf
examples_dir=examples
examples_source=$(wildcard $(examples_dir)/*.c)
valgrind_cmd=bash tools/valgrind_run.sh

.PHONY: all, coverage, debug, release, release2
all: debug

coverage: clean
coverage: coverageclean
coverage: CFLAGS+=-O0 -DDEBUG
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coverage: $(binary)
coverage:
	@./tools/gen_coverage_html.sh "$(realpath $(binary))" "$(realpath $(cov_dir))" $(COLR_ARGS)

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)

release2: CFLAGS+=-O2 -DNDEBUG
release2: $(binary)

$(binary): $(objects)
	@printf "\nCompiling $(binary) executable...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(objects) $(LIBS)

%.o: %.c %.h
	@printf "\nCompiling $<...\n    ";
	$(CC) -c $< $(CFLAGS)

# Build all docs (html and pdf) if needed.
docs: $(docs_main_file)
docs: $(docs_pdf)

# Build the html docs, with example code included.
$(docs_main_file): $(source) $(headers) $(docs_deps)
	@printf "\nBuilding html doxygen docs...\n    Target: $@\n    For: $?\n    "
	doxygen $(docs_html_config)

# Build the doxygen latex docs, without example code (latex_pdf and docs_pdf need this).
# The example code (custom html-wrapper aliases) cause latex to fail.
$(latex_tex): $(source) $(headers) $(latex_deps)
	@printf "\nBuilding latex doxygen docs...\n    Target: $@\n    For: $?\n    "
	doxygen $(docs_latex_config)

$(latex_pdf): $(latex_tex)
	@./tools/gen_latex_pdf.sh --reference && \
		[[ -e "$(latex_pdf)" ]] && \
			printf "\nPDF reference: $(latex_pdf)\n"

$(docs_pdf): $(latex_pdf)
	@./tools/gen_latex_pdf.sh && \
		[[ -e "$(docs_pdf)" ]] && \
			printf "\nPDF manual: $(docs_pdf)\n"

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
	@./tools/clean.sh "$(binary)"

.PHONY: cleandebug
cleandebug: clean
cleandebug: debug

.PHONY: cleandocs, cleanhtml, cleanlatex, cleanexamples, cleanpdf
cleandocs: cleanhtml
cleandocs: cleanlatex
cleandocs: cleanpdf

cleanhtml:
	@./tools/clean.sh -d "$(docs_dir)" "$(docs_dir)/html" "$(docs_dir)/man"

cleanlatex:
	@./tools/clean.sh -m "Latex" $(doxy_latex_files)

cleanexamples:
	@$(MAKE) $(MAKEFLAGS) clean && \
		cd examples && \
			$(MAKE) $(MAKEFLAGS) --no-print-directory clean

cleanpdf:
	@./tools/clean.sh -m "PDF" $(docs_pdf) $(latex_files)

.PHONY: coveragesummary
coveragesummary:
	@./tools/gen_coverage_html.sh "$(binary)" "$(cov_dir)" --summary

.PHONY: coverageview
coverageview:
	@./tools/gen_coverage_html.sh "$(binary)" "$(cov_dir)" --view

.PHONY: docshtml, docslatex, docspdf, docsrebuild
docshtml: $(docs_main_file)

docslatex: $(latex_idx)

docspdf: $(docs_pdf)

docsrebuild: cleandocs
docsrebuild: docs

.PHONY: examples
examples: $(examples_source)
	@cd examples && $(MAKE) $(MAKEFLAGS) --no-print-directory $(COLR_ARGS)

.PHONY: memcheck, memcheckquiet
memcheck:
	@$(valgrind_cmd) -- $(COLR_ARGS)

memcheckquiet:
	@$(valgrind_cmd) -q -- $(COLR_ARGS)

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
    cleandocs       : Delete Doxygen docs from ./$(docs_dir).\n\
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
    release2        : Same as \`release\` target, but with -O2 instead of -O3.\n\
    run             : Run the executable. Args are set with COLR_ARGS.\n\
    runexamples     : Run the example executables in $(examples_dir).\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests.\n\
    testcoverage    : Delete previous test build files, and build the tests for coverage.\n\
    testgdb         : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests through GDB.\n\
    testkdbg        : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests through KDbg.\n\
    testmemcheck    : Build  debug tests (if needed), and run them through valgrind.\n\
    testquiet       : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests with --quiet.\n\
    testsummary     : View a summary of previously built test coverage reports.\n\
    testview        : View previously generated html test coverage reports.\n\
    memcheck        : Run valgrind's memcheck on the executable.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory clean debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory cleantest; \
	};

test:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory test; \
	};

.PHONY: testcoverage, testmemcheck, testsummary, testview
testcoverage:
	-@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory clean coverage

testgdb:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory debug testgdb; \
		};

testkdbg:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory debug testkdbg; \
		};

testmemcheck:
	-@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory debug memcheck

testquiet:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { \
		cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) $(MAKEFLAGS) --no-print-directory debug testquiet; \
	};

testsummary:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coveragesummary

testview:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coverageview

