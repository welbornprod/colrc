/* colr_tool.h
    Header for colr.h example implementation.
    This is not required for using colr.h.
    -Christopher Welborn 02-05-2017
*/


#ifndef COLR_TOOL_H
#pragma GCC diagnostic ignored "-Wunused-macros"
#define COLR_TOOL_H

#include <stdbool.h>
#include <stdlib.h>
#include "colr.h"

#define NAME "Colr"
#define VERSION COLR_VERSION

// Maximum length for user args (fore, back, style).
#define MAX_ARG_LEN 20
// Maximum length for error messages.
#define MAX_ERR_LEN 255
// Maximum length for TEXT argument.
#define MAX_TEXT_LEN 1024

#ifndef debug
    #ifdef DEBUG
        // Debug printer.
        #define debug(...) fprintf(stderr, __VA_ARGS__)
    #else
        // No-op debug print.
        #define debug(...) ((void)0)
    #endif
#endif

// Assumes memory allocated in a variable named `name`.
#define print_fore_color(codename) \
    colrfore(name, #codename, codename); \
    printf("%s ", name);

typedef void (*colorext_func)(char*, const char*, unsigned char);
typedef void (*colorrgb_func)(char*, const char*, RGB*);
void debug_args(char*, char*, char*, char*);
void example_color_build(void);
void print_256(colorext_func);
void print_basic(void);
void print_rainbow_fore(void);
void print_rgb(colorrgb_func);
void print_unrecognized_arg(const char*);
void print_usage(const char*);
void print_usage_full(void);
void read_stdin_arg(char*, size_t);
bool validate_color_arg_OLD(char*, BasicValue, char*);
bool validate_color_arg(const char*, ColorNameType, const char*);
bool validate_style_arg(StyleValue, char*);
#endif // COLR_TOOL_H
