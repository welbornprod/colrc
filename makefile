# Makefile for colr_tool
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
LIBS=-lm

binary=colr
source=colr_tool.c colr.c
headers=colr.h colr_tool.h dbug.h
docsconfig=Doxyfile
docsdir=docs
docsmainfile=$(docsdir)/html/index.html
objects:=$(source:.c=.o)

.PHONY: all, debug, release
all: debug

debug: tags
debug: CFLAGS+=-g3 -DDEBUG
debug: $(binary)
debug: docs

release: CFLAGS+=-O3 -DNDEBUG
release: $(binary)
release: strip

$(binary): $(objects)
	$(CC) -o $(binary) $(CFLAGS) $(objects) $(LIBS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)



$(docsmainfile): $(source) $(headers) $(docsconfig)
docs: $(source) $(headers) $(docsconfig) $(docsmainfile)
	doxygen $(docsconfig);

tags: $(source) $(headers)
	-@printf "Building ctags...\n";
	ctags $(source) $(headers);

.PHONY: clang, clangrelease
clang: CC=clang
clang: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clang: debug

clangrelease: CC=clang
clangrelease: CFLAGS+=-Wno-unknown-warning-option -Wliblto
clangrelease: release

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

.PHONY: cleandocs
cleandocs:
	-@if [[ "$(docsdir)x" != "x" ]] && [[ -e "$(docsmainfile)" ]]; then\
		if rm -r $(docsdir)/*; then\
			printf "Docs cleaned:\n";\
			printf "    %s\n" "$(docsdir)/*";\
		fi;\
	else\
		printf "Docs already clean:\n";\
		printf "    %s\n" "$(docsdir)/*";\
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
    all          : Build with no optimization or debug symbols.\n\
    clang        : Use \`clang\` to build the default target.\n\
    clangrelease : Use \`clang\` to build the release target.\n\
    clean        : Delete previous build files.\n\
    cleandocs    : Delete Doxygen docs from ./$(docsdir).\n\
    cleantest    : Delete previous build files, build the binary and the \n\
                   test binary, and run the tests.\n\
    debug        : Build the executable with debug symbols.\n\
    docs         : Build the Doxygen docs.\n\
    release      : Build the executable with optimization, and strip it.\n\
    strip        : Run \`strip\` on the executable.\n\
    tags         : Build tags for this project using \`ctags\`.\n\
    test         : Build debug (if needed), build the test debug (if needed),\n\
                   and run the tests.\n\
	";

.PHONY: cleantest, test
cleantest:
	-@make --no-print-directory clean debug && { cd test; make --no-print-directory cleantest; };

test:
	-@make --no-print-directory debug && { cd test; make --no-print-directory test; };
