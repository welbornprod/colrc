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
# Sanitizers/protectors to optionally enable.
FFLAGS=-fno-omit-frame-pointer -fstack-protector-strong \
    -fsanitize=address -fsanitize=leak -fsanitize=undefined

binary=colrc
colr_source=colr.c
source=colr_tool.c $(colr_source)
objects:=$(source:.c=.o)
colr_headers=colr.h
headers=colr_tool.h $(colr_headers)
optional_headers=dbug.h
optional_flags=$(foreach header, $(optional_headers), -include $(header))
cov_dir=coverage
cppcheck_cmd=bash ./tools/cppcheck_run.sh
cppcheck_html=./cppcheck_report/index.html
is_build_cmd=bash tools/is_build.sh
make_help_fmt_cmd=python3 tools/make_help_fmter.py
undoxy_md_cmd=python3 tools/undoxy_markdown.py
version_cmd=bash tools/get_version.sh
doc_dep_dir=doc_deps
docs_config=$(doc_dep_dir)/Doxyfile_common
docs_html_config=$(doc_dep_dir)/Doxyfile_html
docs_latex_config=$(doc_dep_dir)/Doxyfile_latex
docs_dir=docs
docs_github_readme=README.md
docs_index_md=$(doc_dep_dir)/index.md
docs_main_file=$(docs_dir)/html/index.html
docs_css=$(doc_dep_dir)/customdoxygen.css
docs_examples=$(wildcard examples/*.c)
docs_deps=$(docs_config) $(docs_html_config) $(docs_index_md) $(docs_examples) $(docs_css)
docs_pdf=$(docs_dir)/ColrC-manual.pdf
docs_cj_dir=$(realpath ../../cjwelborn.github.io/colrc)
latex_dir=$(docs_dir)/latex
latex_header=$(doc_dep_dir)/header.tex
latex_style=$(doc_dep_dir)/doxygen.sty
latex_deps=$(docs_config) $(docs_latex_config) $(docs_index_md) $(latex_header) $(latex_style)
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
# Set `viewing_file` to a filepath, and run $(view_file) to open it with `xdg-open`.
define view_file =
@if [[ -n "$(viewing_file)" ]]; then \
    if [[ -e "$(viewing_file)" ]]; then \
        printf "Viewing: $(viewing_file)\n"; \
        xdg-open "$(viewing_file)" &>/dev/null; \
    else \
        printf "Missing file: $(viewing_file)\n" 1>&2; \
    fi; \
else \
    printf "No filepath provided to make target!\n" 1>&2; \
fi;
endef

.PHONY: all
all: debug

.PHONY: coverage
coverage: clean
coverage: cleancoverage
coverage: CFLAGS+=-O0 -DDEBUG
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: CFLAGS+=-fkeep-inline-functions -fkeep-static-functions
coverage: $(binary)
coverage:
	@./tools/gen_coverage_html.sh "$(realpath $(binary))" "$(realpath $(cov_dir))" $(COLR_ARGS)

.PHONY: debug
debug: tags
debug: CFLAGS+=-g3 -DDEBUG -DCOLR_DEBUG
debug: $(binary)

.PHONY: lib
lib: cleancolrobjects
lib: CFLAGS+=-fpic
lib: colr.o
lib:
	$(CC) -shared -o libcolr.so colr.o

.PHONY: libdebug
libdebug: CFLAGS+=-g3 -DDEBUG -DCOLR_DEBUG
libdebug: lib

.PHONY: librelease
librelease: CFLAGS+=-O3 -DNDEBUG
librelease: lib

.PHONY: release
release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)

.PHONY: release2
release2: CFLAGS+=-O2 -DNDEBUG
release2: $(binary)

# Build the tests with all of the -fsanitize options. This will make the
# executable slower and bigger, but helps to catch things that valgrind doesn't.
# This should never be used as a release build.
.PHONY: sanitize
sanitize: CFLAGS+=-g3 -DDEBUG $(FFLAGS)
sanitize: LIBS+=-lasan
sanitize: $(binary)

$(binary): $(objects)
	@printf "\nCompiling $(binary) executable...\n    "
	$(CC) -o $(binary) $(CFLAGS) $(objects) $(LIBS)

%.o: %.c %.h
	@printf "\nCompiling $<...\n    ";
	$(CC) -c $< $(CFLAGS)

# Build all docs (html and pdf) if needed.
docs: $(docs_main_file)
docs: $(docs_pdf)
docs: $(docs_github_readme)

# Build the github-friendly README.
$(docs_github_readme): $(docs_index_md)
	@printf "\nGenerating the GitHub README: $(docs_github_readme)\n"
	@$(undoxy_md_cmd) \
		-t "ColrC" \
		-H "For full documentation see [docs/index.html](docs/index.html)" \
		$(docs_index_md) \
		$(docs_github_readme)

# Build the html docs, with example code included.
$(docs_main_file): $(source) $(headers) $(docs_deps)
	@printf "\nBuilding html doxygen docs...\n    Target: $@\n    For: $?\n    "
	@(cat $(docs_html_config); $(version_cmd) -p) | doxygen -
	@printf "\n"
# NOT READY FOR THIS YET:
#$(docs_main_file): docscj

# Build the doxygen latex docs, without example code (latex_pdf and docs_pdf need this).
# The example code (custom html-wrapper aliases) cause latex to fail.
$(latex_tex): $(source) $(headers) $(latex_deps)
	@printf "\nBuilding latex doxygen docs...\n    Target: $@\n    For: $?\n    "
	@(cat $(docs_latex_config); $(version_cmd) -p) | doxygen -
	@printf "\n"

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

.PHONY: clang
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

.PHONY: clangrelease
clangrelease: CC=clang
clangrelease: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clangrelease: release

.PHONY: clean
clean:
	@./tools/clean.sh "$(binary)"

.PHONY: cleanquiet
cleanquiet:
	@./tools/clean.sh --quiet "$(binary)"

.PHONY: cleancolrobjects
cleancolrobjects:
	@./tools/clean.sh -m "Colr Objects" colr.o

.PHONY: cleandebug
cleandebug: clean
cleandebug: debug

.PHONY: cleandocs
cleandocs: cleanhtml
cleandocs: cleanlatex
cleandocs: cleanpdf

PHONY: cleanhtml
cleanhtml:
	@./tools/clean.sh -d "$(docs_dir)" "$(docs_dir)/html" "$(docs_dir)/man"

PHONY: cleanlatex
cleanlatex:
	@./tools/clean.sh -m "Latex" $(doxy_latex_files)

PHONY: cleanexamples
cleanexamples:
	@$(MAKE) clean && \
		cd examples && \
			$(MAKE) --no-print-directory clean

PHONY: cleanpdf
cleanpdf:
	@./tools/clean.sh -m "PDF" $(docs_pdf) $(latex_files)

.PHONY: cleancoverage
cleancoverage:
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

.PHONY: coveragesummary
coveragesummary:
	@./tools/gen_coverage_html.sh "$(binary)" "$(cov_dir)" --summary

.PHONY: coverageview
coverageview:
	@./tools/gen_coverage_html.sh "$(binary)" "$(cov_dir)" --view

.PHONY: cppcheck
cppcheck:
	@$(cppcheck_cmd)

.PHONY: cppcheckreport
cppcheckreport:
	@$(cppcheck_cmd) -r

.PHONY: cppcheckreportall
cppcheckreportall:
	@$(cppcheck_cmd) -r && $(cppcheck_cmd) -t -r

.PHONY: cppcheckview
cppcheckview:
	@$(cppcheck_cmd) --view

.PHONY: cppcheckviewall
cppcheckviewall:
	@$(cppcheck_cmd) --view && $(cppcheck_cmd) -t --view

.PHONY: docscj
docscj:
	@if [[ -n "$(docs_cj_dir)" ]] && [[ -d "$(docs_cj_dir)" ]]; then \
		printf "\nCopying docs for cjwelborn.github.io.\n"; \
		cp -r "$(docs_dir)" "$(docs_cj_dir)"; \
	else \
		if [[ -n "$(docs_cj_dir)" ]]; then \
			printf "\nSite dir missing: %s\n" $(docs_cj_dir) 1>&2; \
		else \
			printf "\nSite dir missing, this make target is not for everyone.\n" 1>&2; \
		fi; \
	fi;

.PHONY: docshtml
docshtml: $(docs_main_file)

.PHONY: docslatex
docslatex: $(latex_idx)

.PHONY: docspdf
docspdf: $(docs_pdf)

.PHONY: docsreadme
docsreadme: $(docs_github_readme)

.PHONY: docsrebuild
docsrebuild: cleandocs
docsrebuild: docs

.PHONY: docsview
docsview: viewing_file=$(docs_main_file)
docsview:
	@$(view_file)

.PHONY: docsviewpdf
docsviewpdf: viewing_file=$(docs_pdf)
docsviewpdf:
	@$(view_file)

.PHONY: examples
examples: $(examples_source)
	@cd examples && $(MAKE) --no-print-directory $(COLR_ARGS)

.PHONY: memcheck
memcheck: debug
memcheck:
	@if $(is_build_cmd) "sanitize" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized mode for memcheck.\n"; \
		$(MAKE) clean debug; \
	fi;
	@$(valgrind_cmd) -- $(COLR_ARGS)

.PHONY: memcheckquiet
memcheckquiet: debug
memcheckquiet:
	@if $(is_build_cmd) "sanitize" || ! $(is_build_cmd) "debug"; then \
		printf "\nRebuilding in debug non-sanitized mode for memcheck.\n"; \
		$(MAKE) clean debug; \
	fi;
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
    all               : Build with no optimization or debug symbols.\n\
    clang             : Use \`clang\` to build the default target.\n\
    clangrelease      : Use \`clang\` to build the \`release\` target.\n\
    clean             : Delete previous build files.\n\
    cleancoverage     : Delete previous coverage files.\n\
    cleandebug        : Like running \`make clean debug\`.\n\
    cleandocs         : Delete Doxygen docs from ./$(docs_dir).\n\
    cleanexamples     : Delete previous build files from the examples in $(examples_dir).\n\
    cleantest         : Delete previous build files, build the binary and the \n\
                        test binary, and run the tests.\n\
    coverage          : Compile the \`debug\` build and generate coverage reports.\n\
                        This only checks the main binary, not the tests.\n\
                        See the \`testcoverage\` target.\n\
    coveragesummary   : View a summary of previously generated coverage reports.\n\
                        This is only for the main binary, not the tests.\n\
                        See the \`testsummary\` target.\n\
    coverageview      : View previously generated html coverage reports.\n\
                        This is only for the main binary, not the tests.\n\
                        See the \`testview\` target.\n\
    cppcheck          : Run cppcheck on the tests.\n\
    cppcheckreport    : Generate a cppcheck HTML report.\n\
    cppcheckreportall : Generate a cppcheck HTML report.\n\
                        This will also generate a report for the tests.\n\
    cppcheckview      : View previously generated cppcheck HTML report.\n\
    cppcheckviewall   : View previously generated cppcheck HTML report.\n\
                        This will also view the report for the tests.\n\
    debug             : Build the executable with debug symbols.\n\
    docs              : Build the Doxygen docs.\n\
    docsreadme        : Build the GitHub README.\n\
    docsrebuild       : Like running \`make cleandocs docs\`\n\
    docsview          : View previously generated HTML docs in your browser.\n\
    docsviewpdf       : View previously generated PDF docs in your viewer.\n\
    examples          : Build example executables in $(examples_dir).\n\
    release           : Build the executable with optimization, and strip it.\n\
    release2          : Same as \`release\` target, but with -O2 instead of -O3.\n\
    run               : Run the executable. Args are set with COLR_ARGS.\n\
    runexamples       : Run the example executables in $(examples_dir).\n\
    sanitize          : Build debug with \`-fsanitize\` options.\n\
    strip             : Run \`strip\` on the executable.\n\
    tags              : Build tags for this project using \`ctags\`.\n\
    test              : Build debug (if needed), build the test sanitize (if needed),\n\
                        and run the tests.\n\
    testcoverage      : Delete previous test build files, and build the tests for coverage.\n\
    testeverything    : Alias for \`./test/run_tests.sh --all --quiet\`.\n\
    testfast          : Build the \`test debug\` and run the tests.\n\
                        It's not as thorough as \`test\`, but it catches some\n\
                        errors.\n\
    testfull          : Build/run \`testfast\`, if nothing fails run \`memcheck\`,\n\
                        and if that succeeds, run the tests in \`sanitize\` mode.\n\
                        This will show errors early, and if everything passes\n\
                        then there is a low chance of show-stopping bugs.\n\
    testgdb           : Build \`debug\` (if needed), build the test \`debug\` (if needed),\n\
                        and run the tests through GDB.\n\
    testkdbg          : Build \`debug\` (if needed), build the test \`debug\` (if needed),\n\
                        and run the tests through KDbg.\n\
    testmemcheck      : Build \`debug\` tests (if needed), and run them through \`valgrind\`.\n\
    testquiet         : Build \`debug\` (if needed), build the test \`debug\` (if needed),\n\
                        and run the tests with \`--quiet\`.\n\
    testsummary       : View a summary of previously built test coverage reports.\n\
    testview          : View previously generated html test coverage reports.\n\
    memcheck          : Run \`valgrind --tool=memcheck\` on the executable.\n\
" | $(make_help_fmt_cmd);

.PHONY: cleantest
cleantest:
	-@$(MAKE) --no-print-directory clean debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory cleantest; \
	};

.PHONY: test
test:
	-@$(MAKE) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory test; \
	};

.PHONY: testcppcheckreport
testcppcheckreport:
	@cd test && $(MAKE) cppcheckreport;

.PHONY: testcoverage
testcoverage:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory clean coverage;

# This is the same as `testview`.
.PHONY: testcoverageview
testcoverageview:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory coverageview;

.PHONY: testeverything
testeverything:
	@cd test && \
		$(MAKE) --no-print-directory testeverything;

.PHONY: testfast
testfast:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory testfast;

.PHONY: testfastquiet
testfastquiet:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory testfastquiet;

.PHONY: testfull
testfull:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory testfull;

.PHONY: testfullquiet
testfullquiet:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory testfullquiet;

.PHONY: testgdb
testgdb:
	@$(MAKE) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory debug testgdb; \
		};

.PHONY: testkdbg
testkdbg:
	@$(MAKE) --no-print-directory debug && { \
		cd test && \
			TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory debug testkdbg; \
		};

.PHONY: testmemcheck
testmemcheck:
	@cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory debug memcheck;

.PHONY: testquiet
testquiet:
	@$(MAKE) --no-print-directory debug && { \
		cd test && \
		TEST_ARGS=$(TEST_ARGS) $(MAKE) --no-print-directory debug testquiet; \
	};

.PHONY: testsummary
testsummary:
	@cd test && $(MAKE) --no-print-directory coveragesummary;

.PHONY: testview
testview:
	@cd test && $(MAKE) --no-print-directory coverageview;

