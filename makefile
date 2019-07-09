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

binary=colr
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
latex_idx=$(latex_dir)/refman.idx
latex_pdf=$(latex_dir)/refman.pdf
latex_files=\
	$(wildcard $(latex_dir)/*.ps) $(wildcard $(latex_dir)/*.dvi) \
	$(wildcard $(latex_dir)/*.aux) $(wildcard $(latex_dir)/*.toc) \
	$(wildcard $(latex_dir)/*.idx) $(wildcard $(latex_dir)/*.ind) \
	$(wildcard $(latex_dir)/*.ilg) $(wildcard $(latex_dir)/*.log) \
	$(wildcard $(latex_dir)/*.out) $(wildcard $(latex_dir)/*.brf) \
	$(wildcard $(latex_dir)/*.blg) $(wildcard $(latex_dir)/*.bbl) \
	$(wildcard $(latex_dir)/*.tex) $(wildcard $(latex_dir)/*.md5) \
	$(wildcard $(latex_dir)/*.pdf) $(wildcard $(latex_dir)/*.sty) \
	$(latex_ref) $(docs_pdf) $(latex_dir)/Makefile _minted-refman
examples_dir=examples
examples_source=$(wildcard $(examples_dir)/*.c)
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

# Build all docs (html and pdf) if needed.
docs: $(docs_main_file)
docs: $(docs_pdf)

# Build the html docs, with example code included.
$(docs_main_file): $(source) $(headers) $(docs_deps)
	@printf "\nBuilding html doxygen docs (for $@)...\n    "
	doxygen $(docs_html_config) && printf "    Doxygen html built.\n";

# Build the doxygen latex docs, without example code (latex_pdf and docs_pdf need this).
$(latex_idx): $(source) $(headers) $(latex_deps)
	@printf "\nBuilding latex doxygen docs (for $@)...\n    ";
	doxygen $(docs_latex_config) && printf "    Doxygen latex built.\n";

# Builds $(latex_pdf) if needed.
# Must have these packages installed:
# 	   doxygen-latex, texlive-lang-cyrillic, and texlive-fonts-extra
$(latex_pdf): $(latex_idx)
	@if cd $(latex_dir); then \
		declare -a pdf_args=("-halt-on-error" "-shell-escape");\
		printf "\nBuilding $(latex_pdf) for ($@)...\n"; \
		pdflatex "$${pdf_args[@]}" refman; \
		makeindex refman.idx; \
		pdflatex "$${pdf_args[@]}" refman; \
		latex_count=8 ; \
		while egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\
		  do \
		    printf "\nRerunning latex....\n" ;\
		    pdflatex "$${pdf_args[@]}" refman;\
		    latex_count=`expr $$latex_count - 1` ;\
		  done; \
		makeindex refman.idx; \
		pdflatex "$${pdf_args[@]}" refman; \
	else \
		printf "\nUnable to cd into latex dir: %s\n" "$(latex_dir)" 1>&2; \
	fi;

$(docs_pdf): $(latex_pdf)
	@# Move the refman.pdf into docs/ColrC-manual.pdf.
	@if [[ -e "$(latex_pdf)" ]]; then \
		if cp $(latex_pdf) $(docs_pdf); then \
			printf "\nCopied docs pdf: %s\n" "$(docs_pdf)"; \
		else \
			printf "\nUnable to copy docs pdf: %s\n" "$(docs_pdf)" 1>&2; \
		fi; \
	else \
		printf "\nSource pdf not found: %s\n" "$(latex_pdf)" 1>&2; \
	fi;

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
	@./clean.sh -d "$(docs_dir)" "$(docs_main_file)"

.PHONY: cleanlatex
cleanlatex:
	@./clean.sh -m "Latex" $(latex_files)

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

.PHONY: pdf
pdf: $(docs_pdf)

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
    run             : Run the executable. Args are set with COLR_ARGS.\n\
    runexamples     : Run the example executables in $(examples_dir).\n\
    strip           : Run \`strip\` on the executable.\n\
    tags            : Build tags for this project using \`ctags\`.\n\
    test            : Build debug (if needed), build the test debug (if needed),\n\
                      and run the tests.\n\
    testcoverage    : Delete previous test build files, and build the tests for coverage.\n\
    testmemcheck    : Build  debug tests (if needed), and run them through valgrind.\n\
    testsummary     : View a summary of previously built test coverage reports.\n\
    testview        : View previously generated html test coverage reports.\n\
    memcheck        : Run valgrind's memcheck on the executable.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory clean debug && { cd test; $(MAKE) $(MAKEFLAGS) --no-print-directory cleantest; };

test:
	-@$(MAKE) $(MAKEFLAGS) --no-print-directory debug && { cd test; $(MAKE) $(MAKEFLAGS) --no-print-directory test; };

.PHONY: testcoverage, testmemcheck, testsummary, testview
testcoverage:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory clean coverage

testmemcheck:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory debug memcheck

testsummary:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coveragesummary

testview:
	-@cd test && $(MAKE) $(MAKEFLAGS) --no-print-directory coverageview

