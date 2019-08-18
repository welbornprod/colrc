/*! \file colr.h
    Declarations for ColrC functions, enums, structs, etc.

    \internal
    \author Christopher Welborn
    \date 06-22-2019

    \details
    To use ColrC in your project, you will need to include colr.h
    and compile colr.c with the rest of your files.

    \examplecodefor{colr_h,.c}
    #include "colr.h"
    // Use ColrC functions/macros/etc.

    int main(void) {
        char *colorized = colr(
            "This is ",
            Colr("ColrC", fore(BLUE), style(BRIGHT)),
            " and it tries to make things ",
            Colr("easy", fore("green"), style("underline")),
            "."
        );
        printf("%s\n", colorized);
        free(colorized);
    }
    \endexamplecode

    <em>Don't forget to compile with `colr.c` and `-lm`</em>.
    \code{.sh}
    gcc -std=c11 -c your_program.c colr.c -lm
    \endcode
*/
#ifndef COLR_H
#define COLR_H
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

//! Current version for ColrC.
#define COLR_VERSION "0.3.0"

#ifndef DOXYGEN_SKIP
/*! \def IS_C11
    Pragmas to check for _Generic support.

    \details
    There's a reason for jumping through all of these hoops.
    See: https://mort.coffee/home/c-compiler-quirks/
*/
#if (__STDC_VERSION__ >= 201112L)
    #if defined(__GNUC__) && !defined(__clang__)
        #if (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
            #define IS_C11
        #endif
    #else
        #define IS_C11
    #endif
#endif
#endif // DOXYGEN_SKIP

// ColrC uses GNU extensions.
#ifndef __GNUC__
    #error "ColrC uses GNU extensions that your compiler doesn't support.")
#endif
// Without _Generic, ColrC is useless.
#ifndef IS_C11
    #error "ColrC cannot compile without C11+ generic selections (_Generic).")
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <math.h>  /* Must include `-lm` in compiler args or Makefile LIBS! */
#include <limits.h>
#include <locale.h>
#include <search.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wchar.h>
#include <ttyent.h>

// #include "dbug.h"

/* Tell gcc to ignore unused macros. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* Tell gcc to ignore clang pragmas, for linting. */
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

/* Tell clang to ignore unused macros. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"

//! Convenience definition, because this is used a lot.
#define CODE_RESET_ALL "\x1b[0m"
//! Convenience definition for wide chars.
#define WCODE_RESET_ALL L"\x1b[0m"
//! Short-hand for CODE_RESET_ALL, stands for "No Color".
#define NC CODE_RESET_ALL
//! Short-hand for WCODE_RESET_ALL, stands for "Wide No Color".
#define WNC WCODE_RESET_ALL
//! Length of CODE_RESET_ALL, including `'\0'`.
#define CODE_RESET_LEN 5
/*! Minimum length for the shortest basic fore/back escape code, including `'\0'`.

    \details
    Use CODE_LEN for allocation.
*/
#define CODE_LEN_MIN 5
/*! Maximum length for a basic fore/back escape code, including `'\0'`.
    Keep in mind that BasicValue actually has some "light" colors (104).
*/
#define CODE_LEN 14
/*! Minimum length for the shortest extended fore/back escape code, including `'\0'`.

    \details
    Use CODEX_LEN for allocation.
*/
#define CODEX_LEN_MIN 10
//! Maximum length for an extended fore/back escape code, including `'\0'`.
#define CODEX_LEN 12
/*! Minimum length for the shortest style escape code, including `'\0'`.

    \details
    Use STYLE_LEN for allocation.
*/
#define STYLE_LEN_MIN 5
//! Maximum length for a style escape code, including `'\0'`.
#define STYLE_LEN 6
/*! Maximum length in chars for any combination of basic/extended escape codes.

    Should be `(CODEX_LEN * 2) + STYLE_LEN`.
    Allocating for a string that will be colorized must account for this.
*/
#define COLOR_LEN 30
/*! Minimum length for the shortest RGB fore/back escape code, including `'\0'`.

    \details
    Use CODE_RGB_LEN for allocation.
*/
#define CODE_RGB_LEN_MIN 14
//! Maximum length in chars for an RGB fore/back escape code, including `'\0'`.
#define CODE_RGB_LEN 20
/*! Maximum length in chars added to a rgb colorized string.

    Should be `CODE_RGB_LEN + STYLE_LEN`
    Allocating for a string that will be colorized with rgb values must account
    for this.
*/
#define COLOR_RGB_LEN 26

/*! Maximum length in chars for any possible escape code mixture.

    (basically `(CODE_RGB_LEN * 2) + STYLE_LEN` since rgb codes are the longest).
*/
#define CODE_ANY_LEN 46

/*! Marker for the ColorArg struct, for identifying a void pointer as a
    ColorArg.
*/
#define COLORARG_MARKER UINT_MAX

/*! Marker for the _ColrLastArg_s struct, for identifying a void pointer as a
    _ColrLastArg_s.
*/
#define COLORLASTARG_MARKER (UINT_MAX - 20)

/*! Marker for the ColorJustify struct, for identifying a void pointer as a
    ColorJustify.
*/
#define COLORJUSTIFY_MARKER (UINT_MAX - 30)

/*! Marker for the ColorText struct, for identifying a void pointer as a
    ColorText.
*/
#define COLORTEXT_MARKER (UINT_MAX - 40)

/*! Possible error return value for BasicValue_from_str(), ExtendedValue_from_str(),
    and colorname_to_rgb().
*/
#define COLOR_INVALID (-2)
//! Possible error return value for RGB_from_str().
#define COLOR_INVALID_RANGE (-3)
/*! Alias for COLOR_INVALID.
    \details
    All color values share an _INVALID member with the same value, so:
    \code
    COLOR_INVALID == BASIC_INVALID == EXT_INVALID == STYLE_INVALID
    \endcode
*/
#define EXT_INVALID COLOR_INVALID
/*! Possible error return value for ExtendedValue_from_str() or ExtendedValue_from_esc().
    \details
    This is just an alias for COLOR_INVALID_RANGE.
    \code
    COLOR_INVALID_RANGE == BASIC_INVALID_RANGE == EXT_INVALID_RANGE == STYLE_INVALID_RANGE
    \endcode
*/
#define EXT_INVALID_RANGE COLOR_INVALID_RANGE

/*! \def alloc_basic
    Allocate enough for a basic code.

    \return Pointer to the allocated string, or NULL on error.\n
            \mustfree
*/
#define alloc_basic() calloc(CODE_LEN, sizeof(char))

/*! \def alloc_extended
    Allocate enough for a extended code.

    \return Pointer to the allocated string, or NULL on error.\n
            \mustfree
*/
#define alloc_extended() calloc(CODEX_LEN, sizeof(char))


/*! \def alloc_rgb
    Allocate enough for an rgb code.

    \return Pointer to the allocated string, or NULL on error.\n
            \mustfree
*/
#define alloc_rgb() calloc(CODE_RGB_LEN, sizeof(char))

/*! \def alloc_style
    Allocate enough for a style code.

    \return Pointer to the allocated string, or NULL on error.\n
            \mustfree
*/
#define alloc_style() calloc(STYLE_LEN, sizeof(char))

/*! \def argeq
    Convenience macro for `!strcmp(arg, s1) || !strcmp(arg, s2)`

    \pi arg String to check.
    \pi s1  First string to compare against.
    \pi s2  Second string to compare against.

    \return Non-zero if \p arg matches either \p s1 or \p s2, otherwise `0`.
*/
#define argeq(arg, s1, s2) (!strcmp(arg, s1)) || (!strcmp(arg, s2))

/*! \def asprintf_or_return
    Convenience macro for bailing out of a function when asprintf fails.

    \pi retval Value to return if the asprintf fails.
    \pi ...    Arguments for asprintf.
*/
#define asprintf_or_return(retval, ...) if_not_asprintf(__VA_ARGS__) return retval

/*! \def back
    Create a back color suitable for use with the colr() and Colr() macros.


    \details
    Technically, this macro accepts BasicValues, ExtendedValues, or RGB structs.
    However, for some of these you should be using the
    macros that create those things.

    \details
    BasicValues can be used by their names (RED, YELLOW, etc.).

    \details
    ExtendedValues can be created on the fly with ext().

    \details
    RGB structs can be easily created with rgb().

    \details
    Color names (`char* `) can be passed to generate the appropriate color value.


    \pi x   A BasicValue, ExtendedValue, or RGB struct to use for the color value.
    \return A pointer to a heap-allocated ColorArg struct.\n
            \colrmightfree

    \sa back_arg back_str colr Colr

    \example back_example.c
*/
#define back(x) ColorArg_to_ptr(back_arg(x))

/*! \def back_arg
    Uses ColorArg_from_<type> to build a ColorArg with the appropriate color
    type, based on the type of it's argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorArg.
    This is used by the back() macro.

    \pi x   `BasicValue`, `Extended` (`unsigned char`), `RGB` struct,
            or string (color name) for back color.
    \return A ColorArg with the BACK type set, and it's `.value.type` set
            for the appropriate color type/value.\n
            For invalid values the `.value.type` may be set to TYPE_INVALID.\n
            \mustfree

    \sa back back_str

*/
#define back_arg(x) \
    _Generic( \
        (x), \
        char* : ColorArg_from_str, \
        RGB: ColorArg_from_RGB, \
        BasicValue: ColorArg_from_BasicValue, \
        ExtendedValue: ColorArg_from_ExtendedValue, \
        StyleValue: ColorArg_from_StyleValue \
    )(BACK, x)

/*! \def back_str
    Retrieve just the escape code string for a back color.

    \pi x A BasicValue, ExtendedValue, or RGB struct.
    \return An allocated string.\n
            \mustfree

    \sa back back_arg
*/
#define back_str(x) ColorArg_to_str(back_arg(x))

/*! \def basic
    Casts to BasicValue.

    \pi x   Value to case to `BasicValue`.
    \return A BasicValue.

    \sa fore back colr Colr
*/
#define basic(x) ((BasicValue)(x))


/*! \def bool_colr_enum
    Returns the "truthiness" of the enums used in ColrC
    (BasicValue, ExtendedValue, StyleValue, ColorType, ArgType).

    \details
    Any value less than `0` is considered false.

    \pi x An enum to convert to boolean.
    \retval true if the value is considered valid, or non-empty.
    \retval false if the value is considered invalid, or empty.
*/
#define bool_colr_enum(x) (x < 0 ? false: true)

/*! \def color_arg
    Builds a correct ColorArg struct according to the type of it's second
    argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorArg.

    \pi type `ArgType` (`FORE`, `BACK`, `STYLE`) to build the ColorArg.
    \pi x    `BasicValue`, `Extended` (`unsigned char`). or `RGB` value.
    \return  ColorArg_from_value(type, [appropriate type], x)
*/
#define color_arg(type, x) \
    _Generic( \
        (x), \
        BasicValue: ColorArg_from_value(type, TYPE_BASIC, &x), \
        ExtendedValue: ColorArg_from_value(type, TYPE_EXTENDED, &x), \
        StyleValue: ColorArg_from_value(type, TYPE_STYLE, &x), \
        RGB: ColorArg_from_value(type, TYPE_RGB, &x) \
    )

/*! \def color_name_is_basic
    Convenience macro for checking if a color name is valid, and maps to a
    BasicValue.

    \pi x   Color name (`char*`) to check.
    \return `true` if the name is a valid BasicValue name, otherwise `false`.

    \sa color_name_is_valid color_name_is_invalid
*/
#define color_name_is_basic(x) (ColorType_from_str(x) == TYPE_BASIC)


/*! \def color_name_is_invalid
    Convenience macro for checking if a color name is invalid.

    \pi x Color name (`char*`) to check.
    \return `true` if the name is an invalid color name, otherwise `false`.

    \sa color_name_is_valid color_name_is_basic

    \examplecodefor{color_name_is_invalid,.c}
    char* names[] = {
        "red",
        "lightblue",
        "127",
        "123,54,67",
        "NOTACOLOR",
        "345",
        "1;",
        "1;2;"
    };
    size_t names_len = sizeof(names) / sizeof(names[0]);
    for (size_t i = 0; i < names_len; i++) {
        if (color_name_is_invalid(names[i])) {
            printf("Invalid name: %s\n", names[i]);
        } else  {
            printf("  Valid name: %s\n", names[i]);
        }
    }
    \endexamplecode
*/
#define color_name_is_invalid(x) ColorType_is_invalid(ColorType_from_str(x))

/*! \def color_name_is_valid
    Convenience macro for checking if a color name is valid.

    \pi x   Color name (`char*`) to check.
    \return `true` if the name is a valid color name, otherwise `false`.

    \sa color_name_is_invalid color_name_is_basic

    \examplecodefor{color_name_is_valid,.c}
    char* names[] = {
        "red",
        "lightblue",
        "127",
        "123,54,67",
        "NOTACOLOR",
        "345",
        "1;",
        "1;2;"
    };
    size_t names_len = sizeof(names) / sizeof(names[0]);
    for (size_t i = 0; i < names_len; i++) {
        if (color_name_is_valid(names[i])) {
            printf("  Valid name: %s\n", names[i]);
        } else  {
            printf("Invalid name: %s\n", names[i]);
        }
    }
    \endexamplecode
*/
#define color_name_is_valid(x) ColorType_is_valid(ColorType_from_str(x))

/*! \def color_val
    Builds a correct ColorValue struct according to the type of it's first
    argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorValue.

    \pi x `BasicValue`, `Extended` (`unsigned char`). or `RGB` value.
    \return  ColorValue_from_value([appropriate type], x)
*/
#define color_val(x) \
    _Generic( \
        (x), \
        BasicValue: ColorValue_from_value(TYPE_BASIC, &x), \
        ExtendedValue: ColorValue_from_value(TYPE_EXTENDED, &x), \
        StyleValue: ColorValue_from_value(TYPE_STYLE, &x), \
        RGB: ColorValue_from_value(TYPE_RGB, &x) \
    )

/*! Call the current ColorValue_has_\<type\> function for the given value.

    \details
    Given the correct type of value, this will check to see if a ColorValue
    has the correct `.type` set for the value, and the values match.

    \pi cval The ColorValue to check.
    \pi val  A BasicValue, ExtendedValue, StyleValue, or RGB value.
    \return  `true` if the ColorValue has the correct `.type` and it's value
             matches `val`, otherwise `false`.

    \sa ColorValue
    \sa ColorValue_has_BasicValue ColorValue_has_ExtendedValue
    \sa ColorValue_has_StyleValue ColorValue_has_RGB
*/
#define ColorValue_has(cval, val) \
    _Generic( \
        (val), \
        BasicValue: ColorValue_has_BasicValue, \
        ExtendedValue: ColorValue_has_ExtendedValue, \
        StyleValue: ColorValue_has_StyleValue, \
        RGB: ColorValue_has_RGB \
    )(cval, val)

/*! \def Colr
    Returns a heap-allocated ColorText struct that can be used by itself,
    or with the colr() macro.

    \details
    You must `free()` the resulting ColorText struct using ColorText_free(),
    unless you pass it to colr(), which will `free()` it for you.

    \pi text String to colorize/style.
    \pi ...  No more than 3 ColorArg pointers for fore, back, and style in any order.

    \return An allocated ColorText.\n
            \colrmightfree

    \example Colr_example.c
*/
#define Colr(text, ...) ColorText_to_ptr(ColorText_from_values(text, __VA_ARGS__, _ColrLastArg))

/*! \def Colr_str
    Create an allocated string directly from Colr() arguments.

    \details
    This is a wrapper around `colr(Colr(text, ...))`, which will automatically
    `free()` the ColorText, and return a string that you are responsible for.

    \pi text String to colorize/style.
    \pi ...  No more than 3 ColorArg pointers for fore, back, and style in any order.

    \return An allocated string with the result.\n
            \mustfree
            \maybenullalloc
*/
#define Colr_str(text, ...) colr(Colr(text, __VA_ARGS__))

/*! \def Colr_center
    Sets the JustifyMethod for a ColorText while allocating it.

    \pi text      Text to colorize.
    \pi justwidth Width for justification.
    \pi ...       Fore, back, or style ColorArgs for Colr().

    \return       An allocated ColorText.\n
                  \colrmightfree
*/
#define Colr_center(text, justwidth, ...) ColorText_set_just( \
        Colr(text, __VA_ARGS__), \
        (ColorJustify){.method=JUST_CENTER, .width=justwidth, .padchar=' '} \
    )

/*! \def Colr_ljust
    Sets the JustifyMethod for a ColorText while allocating it.

    \pi text      Text to colorize.
    \pi justwidth Width for justification.
    \pi ...       Fore, back, or style ColorArgs for Colr().

    \return       An allocated ColorText.\n
                  \colrmightfree
*/
#define Colr_ljust(text, justwidth, ...) ColorText_set_just( \
        Colr(text, __VA_ARGS__), \
        (ColorJustify){.method=JUST_LEFT, .width=justwidth, .padchar=' '} \
    )

/*! \def Colr_rjust
    Sets the JustifyMethod for a ColorText while allocating it.

    \pi text      Text to colorize.
    \pi justwidth Width for justification.
    \pi ...       Fore, back, or style ColorArgs for Colr().

    \return       An allocated ColorText.\n
                  \colrmightfree
*/
#define Colr_rjust(text, justwidth, ...) ColorText_set_just( \
        Colr(text, __VA_ARGS__), \
        (ColorJustify){.method=JUST_RIGHT, .width=justwidth, .padchar=' '} \
    )

/*! \def colr
    Join ColorArg pointers, ColorText pointers, and strings into one long string.

    \details
    To build the ColorArg pointers, it is better to use the fore(), back(),
    and style() macros. The ColorArgs are heap allocated, but colr() will
    free() them for you.

    \details
    To build the ColorText pointers, it is better to use the Colr() macro,
    along with the fore(), back(), and style() macros. The ColorTexts are
    heap allocated, but colr() will free() them for you.

    \pi ... One or more ColorArg pointers, ColorText pointers, or strings to join.
    \return An allocated string result.\n
            \mustfree

    \sa Colr

    \example colr_example.c
*/
#define colr(...) _colr(__VA_ARGS__, _ColrLastArg)

/*! \def colr_eq
    Calls the \<type\>_eq functions for the supported types.

    \details
    The types for \p a and \p b must be the same.

    \pi a First supported type to compare.
    \pi b Second supported type to compare.
    \return `true` if the values are equal, otherwise `false`.
*/
#define colr_eq(a, b) \
    _Generic( \
        (a), \
        ArgType: ArgType_eq, \
        BasicValue: BasicValue_eq, \
        ColorArg: ColorArg_eq, \
        ColorJustify: ColorJustify_eq, \
        ColorType: ColorType_eq, \
        ColorValue: ColorValue_eq, \
        ExtendedValue: ExtendedValue_eq, \
        RGB: RGB_eq, \
        StyleValue: StyleValue_eq \
    )(a, b)

/*! \def colr_example
    Calls the \<type\>_example functions for the supported types.

    \details
    This is used to create a human-friendly representation for ColorArgs or
    ColorValues.

    \pi x   A supported type to get an example string for.
    \return An allocated string with the result.\n
            \mustfree
            \maybenullalloc
*/
#define colr_example(x) \
    _Generic( \
        (x), \
        ColorArg: ColorArg_example, \
        ColorValue: ColorValue_example \
    )(x)

/*! \def colr_free
    Calls the \<type\>_free functions for the supported types.

    \details
    If the type is not supported, a plain `free(x)` is used.

    \pi x A pointer to a supported type to free.
*/
#define colr_free(x) \
    _Generic( \
        (x), \
        ColorArg*: ColorArg_free, \
        ColorText*: ColorText_free, \
        default: free \
    )(x)

/*! \def colr_is_empty
    Calls the \<type\>is_empty functions for the supported types.

    \pi x A supported type to build a string from.
*/
#define colr_is_empty(x) \
    _Generic( \
        (x), \
        ColorArg: ColorArg_is_empty, \
        ColorJustify: ColorJustify_is_empty, \
        ColorText: ColorText_is_empty, \
        ColorValue: ColorValue_is_empty \
    )(x)

/*! \def colr_is_invalid
    Calls the \<type\>is_invalid functions for the supported types.


    \pi x A supported type to build a string from.
*/
#define colr_is_invalid(x) \
    _Generic( \
        (x), \
        BasicValue: BasicValue_is_invalid, \
        ExtendedValue: ExtendedValue_is_invalid, \
        StyleValue: StyleValue_is_invalid, \
        ColorArg: ColorArg_is_invalid, \
        ColorType: ColorType_is_invalid, \
        ColorValue: ColorValue_is_invalid \
    )(x)

/*! \def colr_is_valid
    Calls the \<type\>is_valid functions for the supported types.

    \pi x A supported type to build a string from.
*/
#define colr_is_valid(x) \
    _Generic( \
        (x), \
        BasicValue: BasicValue_is_valid, \
        ExtendedValue: ExtendedValue_is_valid, \
        StyleValue: StyleValue_is_valid, \
        ColorArg: ColorArg_is_valid, \
        ColorType: ColorType_is_valid, \
        ColorValue: ColorValue_is_valid \
    )(x)

/*! \def colr_istr_either
    Convenience macro for `!strcasecmp(s1, s2) || !strcasecmp(s1, s3)`.

    \pi s1 The string to compare against the other two strings.
    \pi s2 The first string to compare with.
    \pi s3 The second string to compare with.

    \return `1` if \p s1 is equal to \p s2 or \p s3, otherwise `0`.
*/
#define colr_istr_either(s1, s2, s3) ((s1 && s2 && s3) ? (colr_istreq(s1, s2) || colr_istreq(s1, s3)) : 0)

/*! \def colr_istreq
    Convenience macro for `!strcasecmp(s1, s2)`.

    \pi s1  The first string to compare.
    \pi s2  The second string to compare.

    \return `1` if \p s1 and \p s2 are equal, otherwise `0`.
*/
#define colr_istreq(s1, s2) ((s1 && s2) ? !strcasecmp(s1, s2) : 0)

/*! \def colr_join
    Join ColorArg pointers, ColorText pointers, and strings by another
    ColorArg pointer, ColorText pointer, or string.

    \details
    To build the ColorArg pointers, it is better to use the fore(), back(),
    and style() macros. The ColorArgs are heap allocated, but colr_join() will
    free() them for you.

    \details
    To build the ColorText pointers, it is better to use the Colr() macro,
    along with the fore(), back(), and style() macros. The ColorTexts are
    heap allocated, but colr_join() will free() them for you.

    \pi joiner What to put between the other arguments.
               ColorArg pointer, ColorText pointer, or string.
    \pi ...    Other arguments to join, with \p joiner between them.
               ColorArg pointers, ColorText pointers, or strings, in any order.
    \return    An allocated string.\n
               \mustfree

    \sa colr Colr

    \example colr_join_example.c
*/
#define colr_join(joiner, ...) _colr_join(joiner, __VA_ARGS__, _ColrLastArg)


/*! \def colr_max
    Macro for `(a > b ? a : b)`.

    \pi a   First value to compare.
    \pi b   Second value to compare.
    \return `a` if `a > b`, otherwise `b`.
*/
#define colr_max(a, b) (a > b ? a : b)

/*! \def colr_replace
    Replace a substring in \p s with another string, ColorArg string, or
    ColorText string.

    \details
    If a string (`char*`) is used as \p repl, this is just a wrapper around
    colr_str_replace().

    \details
    If a ColorArg or ColorText is used as \p repl, the appropriate
    colr_str_replace_\<type\> function is called. The function will create a
    string of escape-codes/text to be used as a replacement.

    \details
    If \p repl is `NULL`, then an empty string (`""`) is used as the replacement,
    which causes the \p target string to be removed.

    \pi s      The string to operate on.
               \mustnull
    \pi target A target string to replace in \p s.
               \mustnull
    \pi repl   A string, ColorArg, or ColorText to replace the target string with.
               If this is `NULL`, then an empty string is used (`""`) as the replacement.
    \return    An allocated string with the result.\n
               \mustfree
*/
#define colr_replace(s, target, repl) \
    _Generic( \
        (repl), \
        char*: colr_str_replace, \
        ColorArg*: colr_str_replace_ColorArg, \
        ColorText*: colr_str_replace_ColorText \
    )(s, target, repl)

/*! \def colr_repr
    Transforms several ColrC objects into their string representations.

    \details
    Uses _Generic (C11 standard) to dynamically dispatch to the proper `*_repr`
    functions.

    \details
    If a regular string is passed in, it will be escaped and you must still
    `free()` the result.

    \details
    Supported Types:
        - ColorArg
        - ColorJustify
        - ColorJustifyMethod
        - ColorText
        - ColorValue
        - ArgType
        - ColorType
        - BasicValue
        - ExtendedValue
        - RGB
        - StyleValue
        - TermSize
        - char*
        - char

    \pi x   A value with one of the supported types to transform into a string.
    \return Stringified representation of what was passed in.\n
            \mustfree

*/
#define colr_repr(x) \
    _Generic( \
        (x), \
        ColorArg: ColorArg_repr, \
        ColorJustify: ColorJustify_repr, \
        ColorJustifyMethod: ColorJustifyMethod_repr, \
        ColorText: ColorText_repr, \
        ColorValue: ColorValue_repr, \
        ArgType: ArgType_repr, \
        ColorType: ColorType_repr, \
        BasicValue: BasicValue_repr, \
        ExtendedValue: ExtendedValue_repr, \
        RGB: RGB_repr, \
        StyleValue: StyleValue_repr, \
        TermSize: TermSize_repr, \
        const char*: colr_str_repr, \
        char*: colr_str_repr, \
        const char: colr_char_repr, \
        char: colr_char_repr \
    )(x)


/*! \def colr_str_eq
    Convenience macro for `!strcmp(s1, s2)`.

    \pi s1  The first string to compare.
    \pi s2  The second string to compare.

    \return `1` if \p s1 and \p s2 are equal, otherwise `0`.
*/
#define colr_str_eq(s1, s2) ((s1 && s2) ? !strcmp(s1, s2) : 0)

/*! \def colr_str_either
    Convenience macro for `!strcmp(s1, s2) || !strcmp(s1, s3)`.

    \pi s1 The string to compare against the other two strings.
    \pi s2 The first string to compare with.
    \pi s3 The second string to compare with.

    \return `1` if \p s1 is equal to \p s2 or \p s3, otherwise `0`.
*/
#define colr_str_either(s1, s2, s3) (colr_str_eq(s1, s2) || colr_str_eq(s1, s3))

/*! \def colr_to_str
    Calls the \<type\>to_str functions for the supported types.

    \pi x A supported type to build a string from.
*/
#define colr_to_str(x) \
    _Generic( \
        (x), \
        ArgType: ArgType_to_str, \
        BasicValue: BasicValue_to_str, \
        ColorArg: ColorArg_to_str, \
        ColorText: ColorText_to_str, \
        ColorType: ColorType_to_str, \
        ColorValue: ColorValue_to_str, \
        ExtendedValue: ExtendedValue_to_str, \
        StyleValue: StyleValue_to_str, \
        RGB: RGB_to_str \
    )(x)

#ifndef DOXYGEN_SKIP
#if defined(DEBUG) && defined(dbug)
    /*! \def dbug_repr
        Uses colr_repr() to build a string representation of a ColrC object,
        dbug prints it, and calls free() when it's done.

        \details
        Without `dbug.h` (defines dbug()) and `DEBUG` these calls will be disabled.
        This is for debugging purposes, and is a no-op when DEBUG is not
        defined.

        \pi lbl Label text for the dbug print.
        \pi x   Any object supported by colr_repr().

    */
    #define dbug_repr(lbl, x) \
        do { \
            char* _dbug_repr_s = colr_repr(x); \
            dbug("%s: %s\n", lbl, _dbug_repr_s); \
            free(_dbug_repr_s); \
        } while(0)
#else
    #if !defined(dbug)
        /*! \def dbug
            This is a no-op macro call. If dbug.h is included before colr.h
            this will be defined as a printf-wrapper to print debug messages
            with function/line info.

            \pi ... Arguments for `fprintf(stderr, ...)`
        */
        #define dbug(...) ((void)0)
    #endif
    /*! \def dbug_repr
        This is a no-op macro call. If dbug.h is included before colr.h this
        will be defined as a printf-wrapper to print a label/colr_repr pair
        with function/line info.

        \pi lbl Label text for the dbug print.
        \pi x   Any object supported by colr_repr().
    */
    #define dbug_repr(lbl, x) ((void)0)
#endif
#endif // DOXYGEN_SKIP

/*! \def ext
    Casts to ExtendedValue (unsigned char).

    \pi x   Value to cast to `unsigned char`/`ExtendedValue`.
    \return An ExtendedValue.

    \sa fore back colr Colr ext_hex ext_hex_or
*/
#define ext(x) ((ExtendedValue)x)

/*! \def ext_hex
    Like hex(), but force a conversion to the closest ExtendedValue (256-colors).

    \pi s   A hex string to convert.
    \return The closest matching ExtendedValue, or 0 for bad hex strings.

    \sa ext ext_hex_or hex hex_or
*/
#define ext_hex(s) ext_hex_or(s, ext(0))

/*! \def ext_hex_or
    Like hex_or(), but force a conversion to the closest ExtendedValue (256-colors).

    \details
    This is a convenience macro for ExtendedValue_from_hex_default().

    \pi s              A hex string to convert.
    \pi default_value  ExtendedValue to use if the hex string is not valid.
    \return            The closest matching ExtendedValue, or `default_value` for bad hex strings.

    \sa ext ext_hex hex hex_or
*/
#define ext_hex_or(s, default_value) ExtendedValue_from_hex_default(s, default_value)

/*! \def ext_rgb
    Creates the closest matching ExtendedValue from an RGB value.

    \details
    This is short-hand for ExtendedValue_from_RGB().

    \pi r   The red value.
    \pi g   The green value.
    \pi b   The blue value.

    \return An ExtendedValue that closely matches the RGB value.

    \sa ExtendedValue_from_RGB RGB_to_term_RGB
*/
#define ext_rgb(r, g, b) ExtendedValue_from_RGB((RGB){.red=r, .green=g, .blue=b})

/*! \def fore
    Create a fore color suitable for use with the colr() and Colr() macros.


    \details
    Technically, this macro accepts BasicValues, ExtendedValues, or RGB structs.
    However, for some of these you should be using the
    macros that create those things.

    \details
    BasicValues can be used by their names (RED, YELLOW, etc.).

    \details
    ExtendedValues can be created on the fly with ext().

    \details
    RGB structs can be easily created with rgb().

    \details
    Color names (`char* `) can be passed to generate the appropriate color value.

    \pi x   A BasicValue, ExtendedValue, or RGB struct to use for the color value.
    \return A pointer to a heap-allocated ColorArg struct.\n
            \colrmightfree

    \sa fore_arg fore_str colr Colr

    \example fore_example.c
*/
#define fore(x) ColorArg_to_ptr(fore_arg(x))

/*! \def fore_arg
    Uses ColorArg_from_<type> to build a ColorArg with the appropriate color
    type, based on the type of it's argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorArg.
    This is used by the fore() macro.

    \pi x   `BasicValue`, `Extended` (`unsigned char`), `RGB` struct,
            or string (color name) for fore color.
    \return A ColorArg with the FORE type set, and it's `.value.type` set
            for the appropriate color type/value.
            For invalid values the `.value.type` may be set to TYPE_INVALID.

    \sa fore fore_str
*/
#define fore_arg(x) \
    _Generic( \
        (x), \
        char* : ColorArg_from_str, \
        RGB: ColorArg_from_RGB, \
        BasicValue: ColorArg_from_BasicValue, \
        ExtendedValue: ColorArg_from_ExtendedValue, \
        StyleValue: ColorArg_from_StyleValue \
    )(FORE, x)

/*! \def fore_str
    Retrieve just the escape code string for a fore color.

    \pi     x A BasicValue, ExtendedValue, or RGB struct.
    \return An allocated ColorArg.\n
            \mustfree

    \sa fore fore_arg
*/
#define fore_str(x) ColorArg_to_str(fore_arg(x))

/*! \def hex
    Use RGB_from_hex_default() to create an RGB value.

    \pi s   A hex string to convert.
    \return A valid RGB value, or `rgb(0, 0, 0)` for bad hex strings.

    \sa hex_or ext_hex ext_hex_or
*/
#define hex(s) hex_or(s, rgb(0, 0, 0))

/*! \def hex_or
    Use RGB_from_hex_default() to create an RGB value.

    \pi s            A hex string to convert.
    \pi default_rgb  Default RGB value to use if the hex string is not valid.
    \return          A valid RGB value, or `default_rgb` for bad hex strings.

    \sa hex ext_hex ext_hex_or
*/
#define hex_or(s, default_rgb) RGB_from_hex_default(s, default_rgb)

/*! \def if_not_asprintf
    Convenience macro for checking asprintf's return value.

    \details
    Should be followed by a block of code.

    \pi ... Arguments for asprintf.
*/
#define if_not_asprintf(...) if (asprintf(__VA_ARGS__) < 1)

//! Convenience macro for `fprintf(stderr, ...)`.
#define printferr(...) fprintf(stderr, __VA_ARGS__)

/*! \def rgb
    Creates an anonymous RGB struct for use in function calls.

    \pi r   `unsigned char` Red value.
    \pi g   `unsigned char` Blue value.
    \pi b   `unsigned char` Green value.
    \return An RGB struct.

    \sa fore back colr Colr
*/
#define rgb(r, g, b) ((RGB){.red=r, .green=g, .blue=b})

/*! Strip a leading character from a string, filling a  `char` array with the
    result.

    \po dest   Destination `char` array. Must have room for `strlen(s) + 1`.
    \pi s      String to strip the character from.
    \pi length Length of \p s, the input string.
    \pi c      Character to strip.
*/
#define inline_str_lstrip_char(dest, s, length, c) \
    do { \
        size_t _st_l_c_dest_i = 0; \
        for (size_t _st_l_c_i = 0; _st_l_c_i < length; _st_l_c_i++) { \
            if (s[_st_l_c_i] == c) continue; \
            dest[_st_l_c_dest_i] = s[_st_l_c_i]; \
            _st_l_c_dest_i++; \
        } \
    } while (0);

/*! \def style
    Create a style suitable for use with the colr() and Colr() macros.

    \details
    This macro accepts strings (style names) and StyleValues.

    \details
    Style names (`char* `) can be passed to generate the appropriate style value.

    \pi x   A StyleValue.
    \return A pointer to a heap-allocated ColorArg struct.\n
            \colrmightfree

    \sa style_arg style_str colr Colr

    \example style_example.c
*/
#define style(x) ColorArg_to_ptr(style_arg(x))

/*! \def style_arg
    Uses ColorArg_from_StyleValue to build a ColorArg with the appropriate color
    type/value.

    \pi x   `StyleValue` for the style.
    \return A ColorArg with the STYLE type set, and it's `.value.type` set
            for the appropriate color type/value.
            For invalid values the `.value.type` may be set to TYPE_INVALID.

    \sa style style_str
*/
#define style_arg(x) \
    _Generic( \
        (x), \
        char* : ColorArg_from_str, \
        StyleValue: ColorArg_from_StyleValue \
    )(STYLE, x)

/*! \def style_str
    Retrieve just the escape code string for a style.

    \pi x   StyleValue to use.
    \return An allocated string.\n
            \mustfree

    \sa style style_arg
*/
#define style_str(x) ColorArg_to_str(style_arg(x))

/*! \def while_colr_va_arg
    Construct a while-loop over a `va_list`, where the last argument is
    expected to be _ColrLastArg, or a pointer to a _ColrLastArg_s with the
    same values as _ColrLastArg.

    \pi ap      The `va_list` to use.
    \pi vartype Expected type of the argument.
    \pi x       The variable to assign to (usually `arg`).
*/
#define while_colr_va_arg(ap, vartype, x) while (x = va_arg(ap, vartype), !_colr_is_last_arg(x))

/*! \def with_rgbs
    Iterate over every possible rgb combination, 0-255 for red, green, and blue.

    \details
    This macro expects a block of code after it, where the values `r`, `g`, and
    `b` are declared as `int` in the range `0`-`255`.

    \examplecodefor{with_rgbs,.c}
    with_rgbs() {
        // You can do whatever you want with r, g, and b.
        if ((r % 100 == 0) && (g % 100 == 0) && (b % 100 == 0)) {
            RGB val = rgb(r, g, b);
            char* repr = colr_repr(val);
            char* s = colr(fore(val), repr);
            printf("Found RGB: %s\n", s);
            free(repr);
            free(s);
        }
    }
    \endexamplecode
*/
#define with_rgbs() \
    for (int r = 0; r < 256; r++) \
        for (int g = 0; g < 256; g++) \
            for (int b = 0; b < 256; b++) \

/*! Basic color values, with a few convenience values for extended colors.
    \internal
    The enum values are immediately defined to be explicit casts to the
    BasicValue_t type, this ensure no confusion between StyleValue and
    BasicValue when they are passed to _Generic-based macros.
    \endinternal
*/
typedef enum BasicValue {
    BASIC_INVALID_RANGE = COLOR_INVALID_RANGE,
    BASIC_INVALID = COLOR_INVALID,
    BASIC_NONE = -1,
    // The actual escape code value for fore colors is BasicValue + 30 == (30-39).
    // The actual escape code value for back colors is BasicValue + 40 == (40-49).
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    UNUSED = 8,
    RESET = 9,
    // The following colors are basic "bright" colors.
    // The actual escape code value for fore colors is BasicValue + 80 == (90-97).
    // The actual escape code value for back colors is BasicValue + 90 == (100-107).
    LIGHTBLACK = 10,
    LIGHTRED = 11,
    LIGHTGREEN = 12,
    LIGHTYELLOW = 13,
    LIGHTBLUE = 14,
    LIGHTMAGENTA = 15,
    LIGHTCYAN = 16,
    LIGHTWHITE = 17,
} BasicValue;

#ifndef DOXYGEN_SKIP
// This makes enum values more friendly to _Generic, by explicitly casting
// from `int` to the enum type.
#define BASIC_INVALID basic(BASIC_INVALID)
#define BASIC_NONE basic(BASIC_NONE)
#define BLACK basic(BLACK)
#define RED basic(RED)
#define GREEN basic(GREEN)
#define YELLOW basic(YELLOW)
#define BLUE basic(BLUE)
#define MAGENTA basic(MAGENTA)
#define CYAN basic(CYAN)
#define WHITE basic(WHITE)
#define UNUSED basic(UNUSED)
#define RESET basic(RESET)
#define LIGHTBLACK basic(LIGHTBLACK)
#define LIGHTRED basic(LIGHTRED)
#define LIGHTGREEN basic(LIGHTGREEN)
#define LIGHTYELLOW basic(LIGHTYELLOW)
#define LIGHTBLUE basic(LIGHTBLUE)
#define LIGHTMAGENTA basic(LIGHTMAGENTA)
#define LIGHTCYAN basic(LIGHTCYAN)
#define LIGHTWHITE basic(LIGHTWHITE)
/*! \internal
    Convenience 256 color names.
    \endinternal
*/
#define XBLACK ((ExtendedValue)0)
#define XRED ((ExtendedValue)1)
#define XGREEN ((ExtendedValue)2)
#define XYELLOW ((ExtendedValue)3)
#define XBLUE ((ExtendedValue)4)
#define XMAGENTA ((ExtendedValue)5)
#define XCYAN ((ExtendedValue)6)
#define XWHITE ((ExtendedValue)7)
#define XLIGHTBLACK ((ExtendedValue)8)
#define XLIGHTRED ((ExtendedValue)9)
#define XLIGHTGREEN ((ExtendedValue)10)
#define XLIGHTYELLOW ((ExtendedValue)11)
#define XLIGHTBLUE ((ExtendedValue)12)
#define XLIGHTMAGENTA ((ExtendedValue)13)
#define XLIGHTCYAN ((ExtendedValue)14)
#define XLIGHTWHITE ((ExtendedValue)15)
#endif // DOXYGEN_SKIP

//! Convenience `typedef` for clarity when dealing with extended (256) colors.
typedef unsigned char ExtendedValue;

//! Container for RGB values.
typedef struct RGB {
    /*! \internal
        The order matters. {1, 2, 3} should mean {r, g, b}.
        \endinternal
    */
    //! Red value for a color.
    unsigned char red;
    //! Green value for a color.
    unsigned char green;
    //! Blue value for a color.
    unsigned char blue;
} RGB;

//! Style values.
typedef enum StyleValue {
    STYLE_INVALID_RANGE = COLOR_INVALID_RANGE,
    STYLE_INVALID = COLOR_INVALID,
    STYLE_NONE = -1,
    RESET_ALL = 0,
    BRIGHT = 1,
    DIM = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    FLASH = 5, // DOS has a "rapid flash" for 6 also.
    HIGHLIGHT = 7,
    STRIKETHRU = 9,
    NORMAL = 22,
    // May not be supported.
    FRAME = 51,
    ENCIRCLE = 52,
    OVERLINE = 53, // Supported in Konsole.
} StyleValue;

//! Maximum value allowed for a StyleValue.
#define STYLE_MAX_VALUE ((StyleValue)OVERLINE)
//! Minimum value allowed for a StyleValue.
#define STYLE_MIN_VALUE ((StyleValue)STYLE_INVALID_RANGE)

#ifndef DOXYGEN_SKIP
// This makes enum values more friendly to _Generic, by explicitly casting
// from `int` to the enum type.
#define STYLE_INVALID_RANGE ((StyleValue)STYLE_INVALID_RANGE)
#define STYLE_INVALID ((StyleValue)STYLE_INVALID)
#define STYLE_NONE ((StyleValue)STYLE_NONE)
#define RESET_ALL ((StyleValue)RESET_ALL)
#define BRIGHT ((StyleValue)BRIGHT)
#define DIM ((StyleValue)DIM)
#define ITALIC ((StyleValue)ITALIC)
#define UNDERLINE ((StyleValue)UNDERLINE)
#define FLASH ((StyleValue)FLASH)
#define HIGHLIGHT ((StyleValue)HIGHLIGHT)
#define NORMAL ((StyleValue)NORMAL)
#define FRAME ((StyleValue)FRAME)
#define ENCIRCLE ((StyleValue)ENCIRCLE)
#define OVERLINE ((StyleValue)OVERLINE)
#endif // DOXYGEN_SKIP

//! Argument types (fore, back, style).
typedef enum ArgType {
    ARGTYPE_NONE = -1,
    FORE = 0,
    BACK = 1,
    STYLE = 2,
} ArgType;

#ifndef DOXYGEN_SKIP
// This makes enum values more friendly to _Generic, by explicitly casting
// from `int` to the enum type.
#define ARGTYPE_NONE ((ArgType)ARGTYPE_NONE)
#define FORE ((ArgType)FORE)
#define BACK ((ArgType)BACK)
#define STYLE ((ArgType)STYLE)
#endif
//! Justification style for ColorTexts.
typedef enum ColorJustifyMethod {
    JUST_NONE = -1,
    JUST_LEFT = 0,
    JUST_RIGHT = 1,
    JUST_CENTER = 2,
} ColorJustifyMethod;

#ifndef DOXYGEN_SKIP
// This makes enum values more friendly to _Generic, by explicitly casting
// from `int` to the enum type.
#define JUST_NONE ((ColorJustifyMethod)JUST_NONE)
#define JUST_LEFT ((ColorJustifyMethod)JUST_LEFT)
#define JUST_RIGHT ((ColorJustifyMethod)JUST_RIGHT)
#define JUST_CENTER ((ColorJustifyMethod)JUST_CENTER)
#endif
//! Color/Style code types. Used with ColorType_from_str() and ColorValue.
typedef enum ColorType {
    TYPE_NONE = -6,
    TYPE_INVALID_STYLE = -5,
    TYPE_INVALID_RGB_RANGE = -4,
    TYPE_INVALID_EXT_RANGE = COLOR_INVALID_RANGE,
    TYPE_INVALID = COLOR_INVALID,
    TYPE_BASIC = 0,
    TYPE_EXTENDED = 1,
    TYPE_RGB = 2,
    TYPE_STYLE = 3,
} ColorType;

#ifndef DOXYGEN_SKIP
// This makes enum values more friendly to _Generic, by explicitly casting
// from `int` to the enum type.
#define TYPE_NONE ((ColorType)TYPE_NONE)
#define TYPE_INVALID_STYLE ((ColorType)TYPE_INVALID_STYLE)
#define TYPE_INVALID_RGB_RANGE ((ColorType)TYPE_INVALID_RGB_RANGE)
#define TYPE_INVALID_EXT_RANGE ((ColorType)TYPE_INVALID_EXT_RANGE)
#define TYPE_INVALID ((ColorType)TYPE_INVALID)
#define TYPE_BASIC ((ColorType)TYPE_BASIC)
#define TYPE_EXTENDED ((ColorType)TYPE_EXTENDED)
#define TYPE_RGB ((ColorType)TYPE_RGB)
#define TYPE_STYLE ((ColorType)TYPE_STYLE)
#endif

/*! Holds a known color name and it's `BasicValue`.

    \details
    This is used for the `basic_names` array in colr.c.
*/
typedef struct BasicInfo {
    char* name;
    BasicValue value;
} BasicInfo;
/*! Holds a known color name and it's `ExtendedValue`.

    \details
    This is used for the `basic_names` array in colr.c.
*/
typedef struct ExtendedInfo {
    char* name;
    ExtendedValue value;
} ExtendedInfo;
/*! Holds a known style name and it's `StyleValue`.

    \details
    This is used for the `style_names` array in colr.c.
*/
typedef struct StyleInfo {
    char* name;
    StyleValue value;
} StyleInfo;

static_assert(
    sizeof(unsigned int) == 4,
    "Struct markers will not work without a 32-bit `unsigned int` type."
);

//! Holds a string justification method, width, and padding character for ColorTexts.
typedef struct ColorJustify {
    //! A marker used to inspect void pointers and determine if they are ColorJustifys.
    unsigned int marker;
    //! The justification method, can be JUST_NONE.
    ColorJustifyMethod method;
    //! The desired width for the final string, or `0` to use colr_term_size().
    int width;
    //! The desired padding character, or `0` to use the default (`' '`).
    char padchar;
} ColorJustify;

//! Breaks down Colr struct markers, such as COLORARG_MARKER, into individual bytes.
typedef union ColorStructMarker {
    //! The actual unsigned int marker value.
    unsigned int marker;
    //! Individual bytes that make up the marker.
    struct {
        unsigned char b1;
        unsigned char b2;
        unsigned char b3;
        unsigned char b4;
    } bytes;
} ColorStructMarker;
/*! Holds a color type and it's value.

    \details
    The `.type` member must always match the type of color value it is holding.

    \details
    This is internal. It's used to make the final interface easier to use.
    You probably shouldn't be using it.
*/
typedef struct ColorValue {
    ColorType type;
    BasicValue basic;
    ExtendedValue ext;
    RGB rgb;
    StyleValue style;
} ColorValue;

/*! Holds info about a known color name, like it's ExtendedValue and it's
    RGB value. Some of the names have the same ExtendedValue, and not all
    ExtendedValues have names.

    \details
    This is used in the colr_name_data array.
*/
typedef struct ColorNameData {
    //! The known name of the color.
    char* name;
    //! ExtendedValue (256-colors) for the color.
    ExtendedValue ext;
    //! RGB (TrueColor) for the color.
    RGB rgb;
} ColorNameData;

//! Holds an ArgType, and a ColorValue.
typedef struct ColorArg {
    //! A marker used to inspect void pointers and determine if they are ColorArgs.
    unsigned int marker;
    //! Fore, back, style, invalid.
    ArgType type;
    //! Color type and value.
    ColorValue value;
} ColorArg;

//! Holds a string of text, and optional fore, back, and style ColorArgs.
typedef struct ColorText {
    //! A marker used to inspect void pointers and determine if they are ColorTexts.
    unsigned int marker;
    //! Text to colorize.
    char* text;
    // Pointers are used for compatibility with the fore(), back(), and style() macros.
    //! ColorArg for fore color. Can be `NULL`.
    ColorArg *fore;
    //! ColorArg for back color. Can be `NULL`.
    ColorArg *back;
    //! ColorArg for style value. Can be `NULL`.
    ColorArg *style;
    //! ColorJustify info, set to JUST_NONE by default.
    ColorJustify just;
} ColorText;

//! Holds a terminal size, usually retrieved with colr_term_size().
typedef struct TermSize {
    unsigned short rows;
    unsigned short columns;
} TermSize;

#ifndef DOXYGEN_SKIP
//! A list of BasicInfo items, used with BasicValue_from_str().
extern const BasicInfo basic_names[];
//! Length of basic_names.
extern const size_t basic_names_len;
//! A list of ExtendedInfo, used with ExtendedValue_from_str().
extern const ExtendedInfo extended_names[];
//! Length of extended_names.
extern const size_t extended_names_len;
//! A list of StyleInfo items, used with StyleName_from_str().
extern const StyleInfo style_names[];
//! Length of style_names.
extern const size_t style_names_len;

//! A map of ExtendedValue (256-color) to RGB values.
extern const RGB ext2rgb_map[];
//! Length of ext2rgb_map (should be 256).
extern const size_t ext2rgb_map_len;

//! An array of ColorNameData, with all known color names, even the old/weird ones.
extern const ColorNameData colr_name_data[];
//! Length of colr_name_data.
extern const size_t colr_name_data_len;

//! A specific ColorArg-like struct that marks the end of variadic argument lists.
struct _ColrLastArg_s {
    unsigned int marker;
    unsigned short value;
};

static const struct _ColrLastArg_s _ColrLastArgValue = {
    .marker=COLORLASTARG_MARKER,
    .value=1337
};
// clang linter says it's unused, but it's used in the _colr* functions, and others.
#pragma clang diagnostic ignored "-Wunused-const-variable"
static const struct _ColrLastArg_s* const _ColrLastArg = &_ColrLastArgValue;
#endif // DOXYGEN_SKIP

/*! \file colr.h
    Common macros and definitions are found here in colr.h,
    however the functions are documented in colr.c.
*/
void colr_append_reset(char* s);

char colr_char_escape_char(const char c);
bool colr_char_in_str(const char c, const char* s);
bool colr_char_is_code_end(const char c);
char* colr_char_repr(char x);
bool colr_char_should_escape(const char c);

bool colr_check_marker(unsigned int marker, void* p);
char* colr_empty_str(void);
bool colr_supports_rgb(void);

size_t colr_str_char_count(const char*s, const char c);
char* colr_str_center(const char* s, const char padchar, int width);
size_t colr_str_code_cnt(const char* s);
size_t colr_str_code_len(const char* s);
char* colr_str_copy(char* dest, const char* src, size_t length);
bool colr_str_ends_with(const char* s, const char* suffix);
char** colr_str_get_codes(const char* s, bool unique);
bool colr_str_has_codes(const char* s);
unsigned long colr_str_hash(const char *s);
bool colr_str_is_all(const char* s, const char c);
bool colr_str_is_codes(const char* s);
bool colr_str_is_digits(const char* s);
bool colr_str_list_contains(char** lst, const char* s);
void colr_str_list_free(char** ps);
char* colr_str_ljust(const char* s, const char padchar, int width);
void colr_str_lower(char* s);
char* colr_str_lstrip_chars(const char* s, const char* chars);
size_t colr_str_mb_len(const char* s);
size_t colr_str_noncode_len(const char* s);
char* colr_str_replace(char* s, const char *target, const char* repl);
char* colr_str_replace_ColorArg(char* s, const char *target, const ColorArg* repl);
char* colr_str_replace_ColorText(char* s, const char *target, const ColorText* repl);
char* colr_str_repr(const char* s);
char* colr_str_rjust(const char* s, const char padchar, int width);
bool colr_str_starts_with(const char* s, const char* prefix);
char* colr_str_strip_codes(const char* s);
char* colr_str_to_lower(const char* s);

TermSize colr_term_size(void);
struct winsize colr_win_size(void);
struct winsize colr_win_size_env(void);
void format_bgx(char* out, unsigned char num);
void format_bg(char* out, BasicValue value);
void format_bg_RGB(char* out, RGB rgb);
void format_bg_RGB_term(char* out, RGB rgb);
void format_fgx(char* out, unsigned char num);
void format_fg(char* out, BasicValue value);
void format_fg_RGB(char* out, RGB rgb);
void format_fg_RGB_term(char* out, RGB rgb);
void format_style(char* out, StyleValue style);
/*! \internal
    A function that formats with RGB codes.
    \details
    This is used to implement the different rainbow formatters,
    one will use straight RGB values. The other uses the "closest terminal code".
    Some are for fore colors, others are for back colors.
    \endinternal
*/
/*! A function type that knows how to fill a string with an rgb escape code.
*/
typedef void (*RGB_fmter)(char* out, RGB rgb);
/*! A function type that knows how to create rainbowized text.
*/
typedef char* (*rainbow_creator)(const char* s, double freq, size_t offset);

char* _rainbow(RGB_fmter fmter, const char* s, double freq, size_t offset);
/*! \internal
    Rainbow-related functions.
    \endinternal
*/
char* rainbow_fg(const char* s, double freq, size_t offset);
char* rainbow_fg_term(const char* s, double freq, size_t offset);
char* rainbow_bg(const char* s, double freq, size_t offset);
char* rainbow_bg_term(const char* s, double freq, size_t offset);
RGB rainbow_step(double freq, size_t offset);

/*! \internal
    Helpers for the variadic colr* functions.
    \endinternal
*/
bool _colr_is_last_arg(void* p);
size_t _colr_ptr_length(void* p);

/*! \internal
    The multi-type variadiac function behind the colr() macro.
    \endinternal
*/
char* _colr(void* p, ...);
size_t _colr_size(void* p, va_list args);
/*! \internal
    The multi-type variadiac function behind the colr_join() macro.
    \endinternal
*/
size_t _colr_join_size(void* joinerp, va_list args);
char* _colr_join(void* joinerp, ...);

/*! \internal
    Array-based versions for colr_join().
    \endinternal
*/
size_t _colr_join_array_length(void* ps);
size_t _colr_join_arrayn_size(void* joinerp, void* ps, size_t count);
char* colr_join_array(void* joinerp, void* ps);
char* colr_join_arrayn(void* joinerp, void* ps, size_t count);

/*! \internal
    ArgType functions that only deal with argument types (fore, back, style).
    \endinternal
*/
bool ArgType_eq(ArgType a, ArgType b);
char* ArgType_repr(ArgType type);
char* ArgType_to_str(ArgType type);

/*! \internal
    ColorArg functions that deal with an ArgType, and a ColorValue.
    \endinternal
*/
ColorArg ColorArg_empty(void);
bool ColorArg_eq(ColorArg a, ColorArg b);
char* ColorArg_example(ColorArg carg);
void ColorArg_free(ColorArg* p);
ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value);
ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value);
ColorArg ColorArg_from_RGB(ArgType type, RGB value);
ColorArg ColorArg_from_esc(const char* s);
ColorArg ColorArg_from_str(ArgType type, const char* colorname);
ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value);
ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void* p);
bool ColorArg_is_empty(ColorArg carg);
bool ColorArg_is_invalid(ColorArg carg);
bool ColorArg_is_ptr(void* p);
bool ColorArg_is_valid(ColorArg carg);
size_t ColorArg_length(ColorArg carg);
char* ColorArg_repr(ColorArg carg);
ColorArg* ColorArg_to_ptr(ColorArg carg);
char* ColorArg_to_str(ColorArg carg);
ColorArg** ColorArgs_from_str(const char* s, bool unique);
void ColorArgs_list_free(ColorArg** ps);

/*! \internal
    ColorJustify functions that deal with colr/string justification.
    \endinternal
*/
ColorJustify ColorJustify_empty(void);
bool ColorJustify_eq(ColorJustify a, ColorJustify b);
bool ColorJustify_is_empty(ColorJustify cjust);
ColorJustify ColorJustify_new(ColorJustifyMethod method, int width, char padchar);
char* ColorJustify_repr(ColorJustify cjust);
char* ColorJustifyMethod_repr(ColorJustifyMethod meth);

/*! \internal
    ColorText functions that deal with a string of text, and fore/back/style
    ColorArgs.
    \endinternal
*/
ColorText ColorText_empty(void);
void ColorText_free(ColorText* p);
ColorText ColorText_from_values(char* text, ...);
bool ColorText_has_arg(ColorText ctext, ColorArg carg);
bool ColorText_has_args(ColorText ctext);
bool ColorText_is_empty(ColorText ctext);
bool ColorText_is_ptr(void* p);
size_t ColorText_length(ColorText ctext);
char* ColorText_repr(ColorText);
ColorText* ColorText_set_just(ColorText* ctext, ColorJustify cjust);
void ColorText_set_values(ColorText* ctext, char* text, ...);
ColorText* ColorText_to_ptr(ColorText ctext);
char* ColorText_to_str(ColorText ctext);

/*! \internal
    ColorType functions that deal with the type of ColorValue (basic, ext, rgb.)
    \endinternal
*/
bool ColorType_eq(ColorType a, ColorType b);
ColorType ColorType_from_str(const char* arg);
bool ColorType_is_invalid(ColorType type);
bool ColorType_is_valid(ColorType type);
char* ColorType_repr(ColorType type);
char* ColorType_to_str(ColorType type);

/*! \internal
    ColorValue functions that deal with a specific color value (basic, ext, rgb).
    \endinternal
*/
ColorValue ColorValue_empty(void);
bool ColorValue_eq(ColorValue a, ColorValue b);
char* ColorValue_example(ColorValue cval);
ColorValue ColorValue_from_esc(const char* s);
ColorValue ColorValue_from_str(const char* s);
ColorValue ColorValue_from_value(ColorType type, void* p);
bool ColorValue_has_BasicValue(ColorValue cval, BasicValue bval);
bool ColorValue_has_ExtendedValue(ColorValue cval, ExtendedValue eval);
bool ColorValue_has_StyleValue(ColorValue cval, StyleValue sval);
bool ColorValue_has_RGB(ColorValue cval, RGB rgbval);
bool ColorValue_is_empty(ColorValue cval);
bool ColorValue_is_invalid(ColorValue cval);
bool ColorValue_is_valid(ColorValue cval);
size_t ColorValue_length(ArgType type, ColorValue cval);
char* ColorValue_repr(ColorValue cval);
char* ColorValue_to_str(ArgType type, ColorValue cval);


/*! \internal
    BasicValue functions.
    \endinternal
*/
bool BasicValue_eq(BasicValue a, BasicValue b);
BasicValue BasicValue_from_esc(const char* s);
BasicValue BasicValue_from_str(const char* arg);
bool BasicValue_is_valid(BasicValue bval);
bool BasicValue_is_invalid(BasicValue bval);
int BasicValue_to_ansi(ArgType type, BasicValue bval);
char* BasicValue_to_str(BasicValue bval);
char* BasicValue_repr(BasicValue bval);
/*! \internal
    ExtendedValue functions.
    \endinternal
*/
bool ExtendedValue_eq(ExtendedValue a, ExtendedValue b);
int ExtendedValue_from_esc(const char* s);
int ExtendedValue_from_hex(const char* hexstr);
ExtendedValue ExtendedValue_from_hex_default(const char* hexstr, ExtendedValue default_value);
ExtendedValue ExtendedValue_from_RGB(RGB rgb);
int ExtendedValue_from_str(const char* arg);
bool ExtendedValue_is_invalid(int eval);
bool ExtendedValue_is_valid(int eval);
char* ExtendedValue_repr(int eval);
char* ExtendedValue_to_str(ExtendedValue eval);

/*! \internal
    StyleValue functions.
    \endinternal
*/
bool StyleValue_eq(StyleValue a, StyleValue b);
StyleValue StyleValue_from_esc(const char* s);
StyleValue StyleValue_from_str(const char* arg);
bool StyleValue_is_invalid(StyleValue sval);
bool StyleValue_is_valid(StyleValue sval);
char* StyleValue_repr(StyleValue sval);
char* StyleValue_to_str(StyleValue sval);
/*! \internal
    rgb/RGB functions.
    \endinternal
*/
bool RGB_eq(RGB a, RGB b);
int RGB_from_esc(const char* s, RGB* rgb);
int RGB_from_hex(const char* hexstr, RGB* rgb);
RGB RGB_from_hex_default(const char* hexstr, RGB default_value);
int RGB_from_str(const char* arg, RGB* rgb);
char* RGB_to_hex(RGB rgb);
char* RGB_to_str(RGB rgb);
RGB RGB_to_term_RGB(RGB rgb);
char* RGB_repr(RGB rgb);

/*! \internal
    Various struct-related functions.
*/
char* TermSize_repr(TermSize ts);

/*! \internal
    Some static assertions to make sure nothing breaks.
    \endinternal
*/
static_assert(
    (
        ((int)COLOR_INVALID == (int)TYPE_INVALID) &&
        ((int)TYPE_INVALID == (int)BASIC_INVALID) &&
        ((int)BASIC_INVALID == (int)EXT_INVALID) &&
        ((int)EXT_INVALID == (int)STYLE_INVALID)
    ),
    "Return/enum values for all color/style values should match."
);
static_assert(
    (
        COLORARG_MARKER &&
        (COLORLASTARG_MARKER &&
            (COLORLASTARG_MARKER != COLORARG_MARKER)
        ) &&
        (COLORJUSTIFY_MARKER &&
            (COLORJUSTIFY_MARKER != COLORLASTARG_MARKER) &&
            (COLORJUSTIFY_MARKER != COLORARG_MARKER)
        ) &&
        (COLORTEXT_MARKER &&
            (COLORTEXT_MARKER != COLORLASTARG_MARKER) &&
            (COLORTEXT_MARKER != COLORARG_MARKER) &&
            (COLORTEXT_MARKER != COLORJUSTIFY_MARKER)
        )
    ),
    "Markers must be positive and unique for each struct in Colr!"
);

#endif // COLR_H
