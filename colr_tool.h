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
#include <sys/types.h>
#include <sys/stat.h>
#include "dbug.h"
// colr.h already includes many headers that are used in the colr tool.
#include "colr.h"

#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"

#define NAME "ColrC"
#define VERSION COLR_VERSION

//! Short-hand for (x ? "true" : "false")
#define ct_bool_str(x) (x ? "true" : "false")

/*! \def ct_just_arg_str
    Like ColorJustifyMethod_repr(), but for colr tool arguments.
    \pi x A ColorJustify.
*/
#define ct_just_arg_str(x) ( \
        x.method == JUST_LEFT ? "--ljust": \
        x.method == JUST_RIGHT ? "--rjust": \
        x.method == JUST_CENTER ? "--center" : \
        "<none>" \
    )

/*! \def print_opts_repr
    Print ColrToolOptions.
    \pi x A ColrToolOptions to print the representation for.
*/
#define print_opts_repr(x) \
    do { \
        char* _p_o_r_s = ColrToolOptions_repr(x); \
        printf("%s\n", _p_o_r_s); \
        free(_p_o_r_s); \
    } while (0)

/*! \def print_repr
    Print a representation of a ColorArg to stdout.
    \pi x Any value with a type supported by colr_repr().
*/
#define print_repr(x) \
    do { \
        char* _pcar_valrepr = colr_repr(x); \
        printf("%s\n", _pcar_valrepr); \
        free(_pcar_valrepr); \
    } while(0)

/*! \def return_error
    Print a message before returning `EXIT_FAILURE`.
    \pi ... Arguments for `fprintf(stderr, ...)`.
*/
#define return_error(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        return EXIT_FAILURE; \
    } while (0)

/*! \def return_error_if_null
    Prints a message before returning `EXIT_FAILURE` if `x` is `NULL`/"falsey".
    \pi x   The pointer to check.
    \pi ... Arguments for `fprintf(stderr, ...)`.
*/
#define return_error_if_null(x, ...) if (!x) return_error(__VA_ARGS__)

typedef struct ColrToolOptions_s {
    // Options that are used to colorize text.
    char* text;
    ColorArg* fore;
    ColorArg* back;
    ColorArg* style;
    ColorJustify just;
    // A file path to read text from.
    const char* filepath;
    // Whether to free the ColrToolOptions.text member when exiting.
    // It would need to be cleaned up if a file or stdin was read.
    bool free_text;
    // Whether to free the resulting ColorText.text member when exiting.
    // It would need to be cleaned up if the text was rainbowized.
    bool free_colr_text;
    // Rainbow opts.
    bool rainbow_fore;
    bool rainbow_back;
    bool rainbow_term;
    double rainbow_freq;
    size_t rainbow_offset;
    // Non-colorizing options.
    bool strip_codes;
    // Commands.
    bool print_back;
    bool print_256;
    bool print_basic;
    bool print_rgb;
    bool print_rgb_term;
    bool print_rainbow;
} ColrToolOptions;

ColrToolOptions ColrToolOptions_new(void);
char* ColrToolOptions_repr(ColrToolOptions opts);

bool dir_exists(const char* dirpath);
bool file_exists(const char* filepath);
int parse_args(int argc, char** argv, ColrToolOptions* opts);
bool parse_double_arg(const char* s, double* value);
bool parse_int_arg(const char* s, int* value);
bool parse_size_arg(const char* s, size_t* value);
int print_256(bool do_back);
int print_basic(bool do_back);
int print_rainbow(bool do_back);
int print_rgb(bool do_back, bool term_rgb);
void print_unrecognized_arg(const char* userarg);
int print_usage(const char* reason);
int print_usage_full(void);
int print_version(void);
ColorText* rainbowize(ColrToolOptions* opts);
char* read_file(FILE* fp);
char* read_file_arg(const char* filepath);
char* read_stdin_arg(void);
int strip_codes(ColrToolOptions* opts);
bool validate_color_arg(ColorArg carg, const char* name);
#endif // COLR_TOOL_H
#endif // DOXYGEN_SKIP
