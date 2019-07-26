#ifndef DOXYGEN_SKIP
//! \file

/*! Header for colr.h example implementation.

    /details
    This is not required for using colr.h.

    \author Christopher Welborn
    \date 02-05-2017
*/


#ifndef COLR_TOOL_H
#define COLR_TOOL_H
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <getopt.h>
#include "dbug.h"
#include "colr.h"

#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"

#define NAME "ColrC"
#define VERSION COLR_VERSION

// Maximum length for user args (fore, back, style).
#define MAX_ARG_LEN 20
// Maximum length for error messages.
#define MAX_ERR_LEN 255
// Maximum length for TEXT argument.
#define MAX_TEXT_LEN 1024

// Short-hand for (x ? "true" : "false")
#define colr_bool_str(x) (x ? "true" : "false")

// Print ColrToolOptions.
#define print_opts_repr(x) \
    do { \
        char* _p_o_r_s = ColrToolOptions_repr(x); \
        printf("%s\n", _p_o_r_s); \
        free(_p_o_r_s); \
    } while (0)

// Print a representation of a ColorArg to stdout.
#define print_repr(x) \
    do { \
        char* _pcar_valrepr = colr_repr(x); \
        printf("%s\n", _pcar_valrepr); \
        free(_pcar_valrepr); \
    } while(0)

// Print a representation of a ColorArg to stdout, with a colorized example
// using it's value.
#define print_ColorArg_example(x) \
    do { \
        char* _pcae_valrepr = colr_repr(x); \
        char* _pcae_valcode = ColorArg_to_str(x); \
        printf("%s%s%s\n", _pcae_valcode, _pcae_valrepr, CODE_RESET_ALL); \
        free(_pcae_valcode); \
        free(_pcae_valrepr); \
    } while(0)

typedef struct ColrToolOptions_s {
    char* text;
    ColorArg* fore;
    ColorArg* back;
    ColorArg* style;
    bool free_text;
    bool rainbow_fore;
    bool rainbow_back;
    bool print_back;
    bool print_256;
    bool print_basic;
    bool print_rgb;
    bool print_rgb_term;
    bool print_rainbow;
} ColrToolOptions;

ColrToolOptions ColrToolOptions_new(void);
char* ColrToolOptions_repr(ColrToolOptions colropts);

void debug_args(char* text, char* fore, char* back, char* style);
void example_color_build(void);
int parse_args(int argc, char** argv, ColrToolOptions* colropts);
int print_256(bool do_back);
int print_basic(bool do_back);
int print_rainbow(bool do_back);
int print_rgb(bool do_back, bool term_rgb);
void print_unrecognized_arg(const char* userarg);
int print_usage(const char* reason);
int print_usage_full(void);
int print_version(void);
char* read_stdin_arg(void);
bool validate_color_arg(ColorArg carg, const char* name);
#endif // COLR_TOOL_H
#endif // DOXYGEN_SKIP
