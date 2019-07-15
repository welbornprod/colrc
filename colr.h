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
#define COLR_VERSION "0.2.2"

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
    #error "ColrC cannot compile without C11 generic selections (_Generic).")
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
#include <unistd.h>
#include <wchar.h>
#include <ttyent.h>

#include "dbug.h"

/* Tell gcc to ignore unused macros. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* Tell gcc to ignore clang pragmas, for linting. */
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
/* Tell clang to ignore unused macros. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"

#ifndef M_PI
    //! Definition of PI for `libm` and the rainbow functions.
    #define M_PI (3.14159265358979323846)
#endif
//! Convenience definition, because this is used a lot.
#define CODE_RESET_ALL "\033[0m"
//! Convenience definition for wide chars.
#define WCODE_RESET_ALL L"\033[0m"
//! Short-hand for CODE_RESET_ALL, stands for "No Color".
#define NC CODE_RESET_ALL
//! Length of CODE_RESET_ALL, including `'\0'`.
#define CODE_RESET_LEN 5
/*! Maximum length for a basic fore/back escape code, including `'\0'`.
    Keep in mind that BasicValue actually has some "light" colors (104).
*/
#define CODE_LEN 7
//! Maximum length for an extended fore/back escape code, including `'\0'`.
#define CODEX_LEN 12

//! Maximum length for a style escape code, including `'\0'`.
#define STYLE_LEN 6

/*! Maximum length in chars for any combination of basic/extended escape codes.

    Should be `(CODEX_LEN * 2) + STYLE_LEN`.
    Allocating for a string that will be colorized must account for this.
*/
#define COLOR_LEN 30

//! Maximum length in chars for an RGB fore/back escape code, including '\0'.
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

//! Maximim string length for a fore, back, or style name.
#define MAX_COLOR_NAME_LEN 14

/*! \internal
    Marker for the ColroArg struct, for identifying a void pointer as a
    ColorArg.
    \endinternal
*/
#define COLORARG_MARKER UINT_MAX

/*! \internal
    Marker for the ColorText struct, for identifying a void pointer as a
    ColorText.
    \endinternal
*/
#define COLORTEXT_MARKER (UINT_MAX >> 1)

/*! Possible error return value for BasicValue_from_str(), ExtendedValue_from_str(),
    and colorname_to_rgb().
*/
#define COLOR_INVALID (-2)
//! Possible error return value for rgb_from_str() and RGB_from_str().
#define COLOR_INVALID_RANGE (-1)

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

/*! \def basic
    Casts to BasicValue.

    \pi x   Value to case to `BasicValue`.
    \return A BasicValue.

    \sa fore back colr Colr
*/
#define basic(x) ((enum BasicValue_t)(x))

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

/*! \def ext
    Casts to ExtendedValue (unsigned char).

    \pi x   Value to cast to `unsigned char`/`ExtendedValue`.
    \return An ExtendedValue.

    \sa fore back colr Colr
*/
#define ext(x) ((ExtendedValue)(x))

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

/*! \def colr_streq
    Convenience macro for `!strcmp(s1, s2)`.

    \pi s1  The first string to compare.
    \pi s2  The second string to compare.

    \return `1` if \p s1 and \p s2 are equal, otherwise `0`.
*/
#define colr_streq(s1, s2) ((s1 && s2) ? !strcmp(s1, s2) : 0)
/*! \def colr_istreq
    Convenience macro for `!strcasecmp(s1, s2)`.

    \pi s1  The first string to compare.
    \pi s2  The second string to compare.

    \return `1` if \p s1 and \p s2 are equal, otherwise `0`.
*/
#define colr_istreq(s1, s2) ((s1 && s2) ? !strcasecmp(s1, s2) : 0)

/*! \def colr_str_either
    Convenience macro for `!strcmp(s1, s2) || !strcmp(s1, s3)`.

    \pi s1 The string to compare against the other two strings.
    \pi s2 The first string to compare with.
    \pi s3 The second string to compare with.

    \return `1` if \p s1 is equal to \p s2 or \p s3, otherwise `0`.
*/
#define colr_str_either(s1, s2, s3) ((s1 && s2 && s3) ? (colr_streq(s1, s2) || colr_streq(s1, s3)) : 0)
/*! \def colr_istr_either
    Convenience macro for `!strcasecmp(s1, s2) || !strcasecmp(s1, s3)`.

    \pi s1 The string to compare against the other two strings.
    \pi s2 The first string to compare with.
    \pi s3 The second string to compare with.

    \return `1` if \p s1 is equal to \p s2 or \p s3, otherwise `0`.
*/
#define colr_istr_either(s1, s2, s3) ((s1 && s2 && s3) ? (colr_istreq(s1, s2) || colr_istreq(s1, s3)) : 0)

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


/*! \def colr_repr
    Transforms several ColrC objects into their string representations.

    \details
    Uses _Generic (C11 standard) to dynamically ensure a string.

    \details
    Supported Types:
        - ArgType
        - ColorType
        - ColorArg
        - ColorText
        - ColorValue
        - char*

    \pi x   A value with one of the supported types to transform into a string.
    \return Stringified representation of what was passed in.\n
            \mustfree

*/
#define colr_repr(x) \
    _Generic( \
        (x), \
        ColorArg: ColorArg_repr, \
        ColorText: ColorText_repr, \
        ColorValue: ColorValue_repr, \
        RGB: RGB_repr, \
        ArgType: ArgType_repr, \
        ColorType: ColorType_repr, \
        char*: str_repr \
    )(x)

/*! \def dbug_repr
    Uses colr_repr() to build a string representation of a ColrC object,
    dbug prints it, and calls free() when it's done.

    \details
    This is for dbugging purposes, and is a no-op when DEBUG is not
    defined.

    \pi lbl Label text for the dbug print.
    \pi x   Any object supported by colr_repr().

*/
#if defined(DEBUG) && defined(dbug)
    #define dbug_repr(lbl, x) \
        do { \
            char* _dbug_repr_s = colr_repr(x); \
            dbug("%s: %s\n", lbl, _dbug_repr_s); \
            free(_dbug_repr_s); \
        } while(0)
#else
    #if !defined(dbug)
        #define dbug(...) ((void)0)
    #endif
    #define dbug_repr(lbl, x) ((void)0)
#endif

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
#define colr(...) _colr(__VA_ARGS__, NULL)

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
#define colr_join(joiner, ...) _colr_join(joiner, __VA_ARGS__, NULL)

/*! \def Colr
    Returns a heap-allocated ColorText object that can be used by itself,
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
#define Colr(text, ...) ColorText_to_ptr(ColorText_from_values(text, __VA_ARGS__, NULL))

/*! \def color_name_is_valid
    Convenience macro for checking if a color name is valid.

    \pi x Color name (`char*`) to check.
    \return `true` if the name is a valid color name, otherwise `false`.

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

/*! \def color_name_is_invalid
    Convenience macro for checking if a color name is invalid.

    \pi x Color name (`char*`) to check.
    \return `true` if the name is an invalid color name, otherwise `false`.

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

/*! Basic color values, with a few convenience values for extended colors.

    \internal
    The enum values are immediately defined to be explicit casts to the
    BasicValue_t type, this ensure no confusion between StyleValue and
    BasicValue when they are passed to _Generic-based macros.
    \endinternal
*/
typedef enum BasicValue_t {
    BASIC_INVALID = COLOR_INVALID,
    BASIC_NONE = -1,
    // The actual escape code value for fore colors is BasicValue + 30.
    // The actual escape code value for back colors is BasicValue + 40.
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
    // The actual escape code value for fore colors is BasicValue + 80.
    // The actual escape code value for back colors is BasicValue + 90.
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
# define XBLACK ((ExtendedValue)0)
# define XRED ((ExtendedValue)1)
# define XGREEN ((ExtendedValue)2)
# define XYELLOW ((ExtendedValue)3)
# define XBLUE ((ExtendedValue)4)
# define XMAGENTA ((ExtendedValue)5)
# define XCYAN ((ExtendedValue)6)
# define XWHITE ((ExtendedValue)7)
# define XLIGHTBLACK ((ExtendedValue)8)
# define XLIGHTRED ((ExtendedValue)9)
# define XLIGHTGREEN ((ExtendedValue)10)
# define XLIGHTYELLOW ((ExtendedValue)11)
# define XLIGHTBLUE ((ExtendedValue)12)
# define XLIGHTMAGENTA ((ExtendedValue)13)
# define XLIGHTCYAN ((ExtendedValue)14)
# define XLIGHTWHITE ((ExtendedValue)15)
#endif // DOXYGEN_SKIP

//! Convenience `typedef` for clarity when dealing with extended (256) colors.
typedef unsigned char ExtendedValue;
/*! Alias for COLOR_INVALID.
    \details
    All color values share an _INVALID member with the same value, so:
    \code
    COLOR_INVALID == BASIC_INVALID == EXTENDED_INVALID == STYLE_INVALID
    \endcode
*/
#define EXTENDED_INVALID COLOR_INVALID

//! Container for RGB values.
typedef struct RGB_t {
    unsigned char red;
    unsigned char blue;
    unsigned char green;
} RGB;

//! Style values.
typedef enum StyleValue_t {
    STYLE_INVALID = COLOR_INVALID,
    STYLE_NONE = -1,
    RESET_ALL = 0,
    BRIGHT = 1,
    DIM = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    FLASH = 5,
    HIGHLIGHT = 7,
    NORMAL = 22
} StyleValue;

#ifndef DOXYGEN_SKIP
#define STYLE_INVALID ((enum StyleValue_t)STYLE_INVALID)
#define STYLE_NONE ((enum StyleValue_t)STYLE_NONE)
#define RESET_ALL ((enum StyleValue_t)RESET_ALL)
#define BRIGHT ((enum StyleValue_t)BRIGHT)
#define DIM ((enum StyleValue_t)DIM)
#define ITALIC ((enum StyleValue_t)ITALIC)
#define UNDERLINE ((enum StyleValue_t)UNDERLINE)
#define FLASH ((enum StyleValue_t)FLASH)
#define HIGHLIGHT ((enum StyleValue_t)HIGHLIGHT)
#define NORMAL ((enum StyleValue_t)NORMAL)
#endif // DOXYGEN_SKIP

//! Argument types (fore, back).
typedef enum ArgType_t {
    ARGTYPE_NONE = -1,
    FORE = 0,
    BACK = 1,
    STYLE = 2,
} ArgType;

//! Color/Style code types. Used with ColorType_from_str() and ColorValue.
typedef enum ColorType_t {
    TYPE_INVALID_EXTENDED_RANGE = -5,
    TYPE_INVALID_RGB_RANGE = -4,
    TYPE_INVALID_STYLE = -3,
    TYPE_INVALID = COLOR_INVALID,
    TYPE_BASIC = 0,
    TYPE_EXTENDED = 1,
    TYPE_RGB = 2,
    TYPE_STYLE = 3,
} ColorType;



/*! Holds a known color name and it's `BasicValue`.

    \details
    This is used for the `basic_names` array in colr.c.
*/
typedef struct BasicInfo_t {
    char* name;
    BasicValue value;
} BasicInfo;
/*! Holds a known color name and it's `ExtendedValue`.

    \details
    This is used for the `basic_names` array in colr.c.
*/
typedef struct ExtendedInfo_t {
    char* name;
    ExtendedValue value;
} ExtendedInfo;
/*! Holds a known style name and it's `StyleValue`.

    \details
    This is used for the `style_names` array in colr.c.
*/
typedef struct StyleInfo_t {
    char* name;
    StyleValue value;
} StyleInfo;

/*! Holds a color type and it's value.

    \details
    The `.type` member must always match the type of color value it is holding.

    \details
    This is internal. It's used to make the final interface easier to use.
    You probably shouldn't be using it.
*/
typedef struct ColorValue_t {
    ColorType type;
    BasicValue basic;
    ExtendedValue ext;
    RGB rgb;
    StyleValue style;
} ColorValue;

/*! Holds an ArgType, and a ColorValue.
*/
typedef struct ColorArg_t {
    //! A marker used to inspect void pointers and determine if they are ColorArgs.
    unsigned int marker;
    //! Fore, back, style, invalid.
    ArgType type;
    //! Color type and value.
    ColorValue value;
} ColorArg;


/*! Holds a string of text, and optional fore, back, and style ColorArgs.
*/
typedef struct ColorText_t {
    unsigned int marker;
    char* text;
    ColorArg *fore;
    ColorArg *back;
    ColorArg *style;
} ColorText;

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

//! A map of RGB values to ExtendedValue (256-color).
extern const RGB rgb2term_map[];
//! Length of rgb2term_map (should be 256).
extern const size_t rgb2term_map_len;

#endif


/*! \file colr.h
    Common macros and definitions are found here in colr.h,
    however the functions are documented in colr.c.
*/
char char_escape_char(const char c);
bool char_in_str(const char c, const char* s);
bool char_should_escape(const char c);
char* colr_empty_str(void);
bool colr_supports_rgb(void);
void format_bgx(char* out, unsigned char num);
void format_bg(char* out, BasicValue value);
void format_bg_rgb(char* out, unsigned char red, unsigned char green, unsigned char blue);
void format_bg_RGB(char* out, RGB rgb);
void format_bg_RGB_term(char* out, RGB rgb);
void format_fgx(char* out, unsigned char num);
void format_fg(char* out, BasicValue value);
void format_fg_rgb(char* out, unsigned char red, unsigned char green, unsigned char blue);
void format_fg_RGB(char* out, RGB rgb);
void format_fg_RGB_term(char* out, RGB rgb);
void format_style(char* out, StyleValue style);
/*! \internal
    A function that formats in the rainbow style.
    \details
    This is used to implement the different rainbow formatters,
    one will use straight RGB values. The other uses the "closest terminal code".
    Some are for fore colors, others are for back colors.
    \endinternal
*/
/*! A function type that knows how to fill a string with an rgb escape code.
*/
typedef void (*RGB_fmter)(char* out, RGB rgb);
char* _rainbow(RGB_fmter fmter, const char* s, double freq, size_t step);
/*! \internal
    Rainbow-related functions.
    \endinternal
*/
char* rainbow_fg(const char* s, double freq, size_t offset);
char* rainbow_fg_term(const char* s, double freq, size_t offset);
char* rainbow_bg(const char* s, double freq, size_t offset);
char* rainbow_bg_term(const char* s, double freq, size_t offset);
RGB rainbow_step(double freq, size_t step);

/*! \internal
    String-based functions.
    \endinternal
*/
void str_append_reset(char* s);
char* str_copy(char* dest, const char* src, size_t length);
bool str_endswith(const char* s, const char* suffix);
bool str_has_codes(const char* s);
bool str_is_all(const char* s, const char c);
bool str_is_digits(const char* s);
void str_lower(char* s);
char* str_lstrip_chars(const char* s, const char* chars);
char* str_repr(const char* s);
bool str_startswith(const char* s, const char* prefix);
char* str_to_lower(const char* s);
wchar_t* str_to_wide(const char* s);
char* wide_to_str(const wchar_t* s);

/*! \internal
    The multi-type variadiac function behind the colr() macro.
    \endinternal
*/
char* _colr(void *p, ...);
/*! \internal
    The multi-type variadiac function behind the colr_join() macro.
    \endinternal
*/
char* _colr_join(void *joinerp, ...);

/*! \internal
    ArgType functions that only deal with argument types (fore, back, style).
    \endinternal
*/
char* ArgType_repr(ArgType type);
char* ArgType_to_str(ArgType type);

/*! \internal
    ColorArg functions that deal with an ArgType, and a ColorValue.
    \endinternal
*/
void ColorArg_free(ColorArg *p);
ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value);
ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value);
ColorArg ColorArg_from_RGB(ArgType type, RGB value);
ColorArg ColorArg_from_str(ArgType type, char* colorname);
ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value);
ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void *p);
bool ColorArg_is_invalid(ColorArg carg);
bool ColorArg_is_ptr(void *p);
bool ColorArg_is_valid(ColorArg carg);
ColorArg *ColorArg_to_ptr(ColorArg carg);
char* ColorArg_repr(ColorArg carg);
char* ColorArg_to_str(ColorArg carg);

/*! \internal
    ColorText functions that deal with a string of text, and fore/back/style
    ColorArgs.
    \endinternal
*/
void ColorText_free(ColorText *p);
ColorText ColorText_from_values(char* text, ...);
bool ColorText_is_ptr(void *p);
char* ColorText_repr(ColorText);
ColorText *ColorText_to_ptr(ColorText ctext);
char* ColorText_to_str(ColorText ctext);

/*! \internal
    ColorType functions that deal with the type of ColorValue (basic, ext, rgb.)
    \endinternal
*/
ColorType ColorType_from_str(const char* arg);
bool ColorType_is_invalid(ColorType type);
bool ColorType_is_valid(ColorType type);
char* ColorType_repr(ColorType type);

/*! \internal
    ColorValue functions that deal with a specific color value (basic, ext, rgb).
    \endinternal
*/
ColorValue ColorValue_from_str(char* s);
ColorValue ColorValue_from_value(ColorType type, void *p);
bool ColorValue_is_invalid(ColorValue cval);
char* ColorValue_repr(ColorValue cval);
char* ColorValue_to_str(ArgType type, ColorValue cval);


/*! \internal
    Functions that deal with basic color values, and their types.
    \endinternal
*/
BasicValue BasicValue_from_str(const char* arg);
int BasicValue_to_ansi(ArgType type, BasicValue bval);
int ExtendedValue_from_str(const char* arg);
int rgb_from_hex(const char *hexstr, unsigned char* r, unsigned char* g, unsigned char*b);
int rgb_from_str(const char* arg, unsigned char* r, unsigned char* g, unsigned char* b);
bool RGB_eq(RGB a, RGB b);
int RGB_from_hex(const char* arg, RGB *rgb);
int RGB_from_str(const char* arg, RGB* rgb);
char* RGB_to_hex(RGB rgb);
RGB RGB_to_term_RGB(RGB rgb);
ExtendedValue RGB_to_ExtendedValue(RGB rgb);
char* RGB_repr(RGB rgb);

StyleValue StyleValue_from_str(const char* arg);

#endif // COLR_H
