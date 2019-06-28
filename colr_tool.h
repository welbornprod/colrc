#ifndef DOXYGEN_SKIP
//! \file

/*! Header for colr.h example implementation.

    /details
    This is not required for using colr.h.

    \author Christopher Welborn
    \date 02-05-2017
*/


#ifndef COLR_TOOL_H
#pragma GCC diagnostic ignored "-Wunused-macros"
#define COLR_TOOL_H
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

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

// Print a representation of a ColorArg to stdout.
#define print_ColorArg_repr(x) \
    do { \
        char *_pcar_valrepr = force_repr(x); \
        printf("%s\n", _pcar_valrepr); \
        free(_pcar_valrepr); \
    } while(0)

// Print a representation of a ColorArg to stdout, with a colorized example
// using it's value.
#define print_ColorArg_example(x) \
    do { \
        char *_pcae_valrepr = force_repr(x); \
        char *_pcae_valcode = ColorArg_to_str(x); \
        printf("%s%s%s\n", _pcae_valcode, _pcae_valrepr, CODE_RESET_ALL); \
        free(_pcae_valcode); \
        free(_pcae_valrepr); \
    } while(0)

// Assumes memory allocated in a variable named `name`.
#define print_fore_color(codename) \
    do { \
        colrfg(name, #codename, codename); \
        printf("%s ", name); \
    } while(0)

typedef void (*colorext_func)(char*, const char*, unsigned char);
typedef void (*colorrgb_func)(char*, const char*, struct RGB);
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
bool validate_color_arg(const char*, ColorType, const char*);
bool validate_style_arg(StyleValue, char*);
#endif // COLR_TOOL_H
#endif // DOXYGEN_SKIP
