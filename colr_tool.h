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
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "colr.h"

#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"

#define NAME "ColrC"
#define VERSION COLR_VERSION

//! Default frequency for rainbowize.
#define CT_DEFAULT_FREQ 0.1l
//! Minimum frequency allowed for rainbowize.
#define CT_MIN_FREQ 0.0l
//! Maximum frequency allowed for rainbowize.
#define CT_MAX_FREQ 1.0l
//! Default starting offset for rainbowize.
#define CT_DEFAULT_OFFSET 3
//! Default 'spread' for rainbowize (width for each rainbow color).
#define CT_DEFAULT_SPREAD 1

//! Short-hand for (x ? "true" : "false")
#define bool_str(x) (x ? "true" : "false")

/*! \def just_arg_str
    Like ColorJustifyMethod_repr(), but for colr tool arguments.
    \pi x A ColorJustify.
*/
#define just_arg_str(x) ( \
        x.method == JUST_LEFT ? "--ljust": \
        x.method == JUST_RIGHT ? "--rjust": \
        x.method == JUST_CENTER ? "--center" : \
        "<none>" \
    )

/*! \def print_opts_repr
    Print ColrOpts.
    \pi x A ColrOpts to print the representation for.
*/
#define print_opts_repr(x) \
    do { \
        char* _p_o_r_s = ColrOpts_repr(x); \
        printf("%s\n", _p_o_r_s); \
        free(_p_o_r_s); \
    } while (0)

/*! \def colr_print_repr
    Print a representation of a ColorArg to stdout.
    \pi x Any value with a type supported by colr_repr().
*/
#define colr_print_repr(x) \
    do { \
        char* _pcar_valrepr = colr_repr(x); \
        printf("%s\n", _pcar_valrepr); \
        free(_pcar_valrepr); \
    } while(0)

/*! \def print_usage_errmsg
    Call print_usage with a printf-style message.

    \details
    This will `return EXIT_FAILURE` if allocation fails.

    \pi ... Arguments for `asprintf`, minus the `char` pointer.
*/
#define print_usage_errmsg(...) \
    do { \
        char* _p_u_m_s = NULL; \
        asprintf_or_return(EXIT_FAILURE, &_p_u_m_s, __VA_ARGS__); \
        print_usage(_p_u_m_s); \
        free(_p_u_m_s); \
    } while (0)


//! Convenience macro for `fprintf(stderr, ...)`.
#define printferr(...) fprintf(stderr, __VA_ARGS__)

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

/*! \def stream_name
    Return a string representation for a common file stream like `stdin`,
    `stdout`, and `stderr`.

    \details
    Arbritrary stream pointers are not supported.

    \pi filep A FILE pointer to get the name for.
    \return   A constant \string containing the name, possibly "unknown".
*/
#define stream_name(filep) ( \
        (filep == stdin) ? "stdin" : \
        (filep == stdout) ? "stdout" : \
        (filep == stderr) ? "stdout" : "<unknown>" \
    )


typedef struct ColrOpts_s {
    // Options that are used to colorize text.
    char* text;
    ColorArg* fore;
    ColorArg* back;
    ColorArg* style;
    ColorJustify just;
    // A file path to read text from.
    const char* filepath;
    // Whether to free the ColrOpts.text member when exiting.
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
    size_t rainbow_spread;
    // Non-colorizing options.
    bool auto_disable;
    bool is_disabled;
    FILE* out_stream;
    bool list_codes;
    bool list_unique_codes;
    bool strip_codes;
    bool translate_code;
} ColrOpts;

typedef int (*colr_tool_cmd)(ColrOpts* opts);

void ColrOpts_cleanup(ColrOpts* opts);
void ColrOpts_free_args(ColrOpts* opts);
void ColrOpts_free_text(ColrOpts* opts);
ColrOpts ColrOpts_new(void);
char* ColrOpts_repr(ColrOpts opts);
bool ColrOpts_set_disabled(ColrOpts* opts);
bool ColrOpts_set_text(ColrOpts* opts);

bool dir_exists(const char* dirpath);
bool file_exists(const char* filepath);
int list_codes(ColrOpts* opts);
int parse_arg_char(char** argv, const char* long_name, const char c, ColrOpts* opts);
int parse_arg_long(const char* long_name, ColrOpts* opts);
int parse_args(int argc, char** argv, ColrOpts* opts);
bool parse_double_arg(const char* s, double* value);
bool parse_int_arg(const char* s, int* value);
bool parse_size_arg(const char* s, size_t* value);
int print_256(ColrOpts* opts, bool do_back);
int print_basic(ColrOpts* opts, bool do_back);
void print_name(ColrOpts* opts, size_t index, bool do_rgb);
int print_names(ColrOpts* opts, bool do_rgb);
int print_plain(ColrOpts* opts);
int print_rainbow(ColrOpts* opts, bool do_back);
int print_rgb(ColrOpts* opts, bool do_back, bool term_rgb);
int print_usage(const char* reason);
int print_usage_full(void);
int print_version(void);
ColorText* rainbowize(ColrOpts* opts);
char* read_file(FILE* fp);
char* read_file_arg(const char* filepath);
char* read_stdin_arg(void);
int run_colr_cmd(colr_tool_cmd func, ColrOpts* opts);
int strip_codes(ColrOpts* opts);
void strip_nl(char* s);
int translate_code(ColrOpts* opts);
bool validate_color_arg(ColorArg carg, const char* name);
#endif // COLR_TOOL_H
#endif // DOXYGEN_SKIP
