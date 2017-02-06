/* colr_tool.h
    Header for colr.h example implementation.
    This is not required for using colr.h.
    -Christopher Welborn 02-05-2017
*/


#ifndef _COLR_TOOL_H
#pragma GCC diagnostic ignored "-Wunused-macros"
#define _COLR_TOOL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "colr.h"

#define NAME "Colr"
#define VERSION COLR_VERSION
#define streq(s1, s2) (!strcmp(s1, s2))
#define argeq(arg, s1, s2) (!strcmp(arg, s1)) || (!strcmp(arg, s2))
#define printferr(...) fprintf(stderr, __VA_ARGS__)

// Maximum length for user args (fore, back, style).
#define MAX_ARG_LEN 20
// Maximum length for error messages.
#define MAX_ERR_LEN 255
// Maximum length for TEXT argument.
#define MAX_TEXT_LEN 1024

#ifdef DEBUG
    // Debug printer.
    #define PRINTD(...) fprintf(stderr, __VA_ARGS__)
#else
    // No-op debug print.
    #define PRINTD(...) ((void)0)
#endif

typedef void (*colorext_func)(char*, unsigned char, char*);
void debug_args(char*, char*, char*, char*);
void print_256(colorext_func);
void print_unrecognized_arg(const char*);
void print_usage(const char*);
void print_usage_full(void);
void read_stdin_arg(char*, size_t);
bool validate_color_arg(char*, Colors, char*);
bool validate_style_arg(Styles, char*);
#endif // _COLR_TOOL_H
