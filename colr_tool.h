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

void debug_args(char *text, char *fore, char *back, char *style);
void example_color_build(void);
int print_256(bool do_fore);
int print_basic(bool do_fore);
int print_rainbow_fore(void);
int print_rgb(bool do_fore);
void print_unrecognized_arg(const char* userarg);
int print_usage(const char *reason);
int print_usage_full(void);
void read_stdin_arg(char *textarg, size_t length);
bool validate_color_arg(struct ColorArg carg, const char *name);
#endif // COLR_TOOL_H
#endif // DOXYGEN_SKIP
