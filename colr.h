/*! \file colr.h
    Declarations for ColrC functions, enums, structs, etc.

    \details
    To use ColrC in your project, you will need to include colr.h
    and compile colr.c with the rest of your files.

    \code{.c}
    #include "colr.h"
    // Use ColrC functions/macros/etc.
    \endcode

    \code{.sh}
    gcc -std=c11 -c your_program.c colr.c -o myprogram
    \endcode
*/
// TODO: After fleshing out the interface, come back here and show actual
//       usage in the code block above.
#ifndef COLR_H
#define COLR_H
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

//! Current version for Colr.
#define COLR_VERSION "0.2.2"


#include <ctype.h>
#include <malloc.h>
#include <math.h>  /* Must include `-lm` in compiler args or Makefile LIBS! */
#include <limits.h>
#include <search.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
//! Short-hand for CODE_RESET_ALL, stands for "No Color".
#define NC CODE_RESET_ALL
//! Length of CODE_RESET_ALL, including `'\0'`.
#define CODE_RESET_LEN 5
/*! Maximum length for a basic fore/back escape code, including `'\0'`.
    Keep in mind that BasicValue actually has some extended values in it
    for convenience.
*/
#define CODE_LEN 12
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
#define MAX_COLOR_NAME_LEN 12

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

/*! \def alloc_basic
    Allocate enough for a basic code.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_basic() calloc(CODE_LEN, sizeof(char))

/*! \def alloc_extended
    Allocate enough for a extended code.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_extended() calloc(CODEX_LEN, sizeof(char))


/*! \def alloc_rgb
    Allocate enough for an rgb code.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_rgb() calloc(CODE_RGB_LEN, sizeof(char))

/*! \def alloc_style
    Allocate enough for a style code.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_style() calloc(STYLE_LEN, sizeof(char))

/*! \def alloc_with_code
    Allocate `str_len` + enough for a basic code with reset appended.

    \pm str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_code(str_len) calloc(str_len + CODEX_LEN, sizeof(char))
/*! \def alloc_with_codes
    Allocate `str_len` + enough for a mixture of fore/basic codes.

    \pi str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_codes(str_len) calloc(str_len + COLOR_LEN, sizeof(char))
/*! \def alloc_with_rgb
    Allocate `str_len` + enough for an rgb code with reset appended.

    \pi str_len Extra room to allocate for text.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_rgb(str_len) calloc(str_len + COLOR_RGB_LEN, sizeof(char))
/*! \def alloc_with_style
    Allocate `str_len` + enough for a style code with reset appended.

    \pi str_len Extra room to allocate for text.

    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_style(str_len) calloc(str_len + STYLE_LEN, sizeof(char))

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

    \pi x Value to case to `BasicValue`.
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

    \pi x Value to cast to `unsigned char`/`ExtendedValue`.
*/
#define ext(x) ((ExtendedValue)(x))

//! Convenience macro for `fprintf(stderr, ...)`.
#define printferr(...) fprintf(stderr, __VA_ARGS__)

/*! \def rgb
    Creates an anonymous RGB struct for use in function calls.

    \pi r `unsigned char` Red value.
    \pi g `unsigned char` Blue value.
    \pi b `unsigned char` Green value.

*/
#define rgb(r, g, b) ((struct RGB){.red=r, .green=g, .blue=b})

/*! \def streq
    Convenience macro for `!strcmp(s1, s2)`.

    \pi s1 The first string to compare.
    \pi s2 The second string to compare.

    \retval 0 if \p s1 and \p s2 are equal.
    \retval 1 if \p s1 is greater than \p s2.
    \retval 1 if \p s1 is less than \p s2.
*/
#define streq(s1, s2) (!strcmp(s1, s2))

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
        struct RGB: ColorArg_from_value(type, TYPE_RGB, &x) \
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
        struct RGB: ColorValue_from_value(TYPE_RGB, &x) \
    )


/*! \def force_repr
    Transforms several ColrC objects into strings. If a string is given, this does nothing.
    \details
    Uses _Generic (C11 standard) to dynamically ensure a string.

    \details
    This is used to dynamically join strings and colors.

    \details
    Supported Types:
        - struct ColorValue
        - ColorType
        - char*

    \pi x   A string, ColorValue, or ColorType to transform into a string.
            \remark _Obviously this is a no-op for strings._
    \return Either the string that was given, or a stringified version of what was given.
*/
#define force_repr(x) \
    _Generic( \
        (x), \
        struct ColorArg: ColorArg_repr, \
        struct ColorText: ColorText_repr, \
        struct ColorValue: ColorValue_repr, \
        ArgType: ArgType_repr, \
        ColorType: ColorType_repr, \
        char*: str_noop \
    )(x)

/*! \def debug_repr
    Uses force_repr() to build a string representation of a Colr object,
    debug prints it, and calls free() when it's done.

    \details
    This is for debugging purposes obviously, and is a no-op when DEBUG is not
    defined.

    \pi lbl Label text for the debug print.
    \pi x   Any object supported by force_repr().
*/
#ifdef DEBUG
    #define debug_repr(lbl, x) \
        do { \
            char *_debug_repr_s = force_repr(x); \
            debug("%s: %s\n", lbl, _debug_repr_s); \
            free(_debug_repr_s); \
        } while(0)
#else
    #define debug_repr(lbl, x) ((void)0)
#endif
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
*/
#define fore_arg(x) \
    _Generic( \
        (x), \
        char *: ColorArg_from_str, \
        struct RGB: ColorArg_from_RGB, \
        BasicValue: ColorArg_from_BasicValue, \
        ExtendedValue: ColorArg_from_ExtendedValue, \
        StyleValue: ColorArg_from_StyleValue \
    )(FORE, x)


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
    Color names (`char *`) can be passed to generate the appropriate color value.

    \example fore_macro.c

    \pi x   A BasicValue, ExtendedValue, or RGB struct to use for the color value.
    \return A pointer to a heap-allocated ColorArg struct. If used with
            colr() or Colr(), __it will be free()'d__.
            This is to allow for an easy interface.
            If used outside of those macros, __you must free() it__.
*/
#define fore(x) ColorArg_to_ptr(fore_arg(x))
/*! \def fore_str
    Retrieve just the escape code string for a fore color.

    \details
    You must free() the resulting string.

    \pi x A BasicValue, ExtendedValue, or RGB struct.

    \sa fore_arg
*/
#define fore_str(x) ColorArg_to_str(fore_arg(x))

/*! \def back_arg
    Uses ColorArg_from_<type> to build a ColorArg with the appropriate color
    type, based on the type of it's argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorArg.
    This is used by the back() macro.

    \pi x   `BasicValue`, `Extended` (`unsigned char`), `RGB` struct,
            or string (color name) for back color.
    \return A ColorArg with the BACK type set, and it's `.value.type` set
            for the appropriate color type/value.
            For invalid values the `.value.type` may be set to TYPE_INVALID.
*/
#define back_arg(x) \
    _Generic( \
        (x), \
        char *: ColorArg_from_str, \
        struct RGB: ColorArg_from_RGB, \
        BasicValue: ColorArg_from_BasicValue, \
        ExtendedValue: ColorArg_from_ExtendedValue, \
        StyleValue: ColorArg_from_StyleValue \
    )(BACK, x)


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
    Color names (`char *`) can be passed to generate the appropriate color value.

    \example back_macro.c

    \pi x   A BasicValue, ExtendedValue, or RGB struct to use for the color value.
    \return A pointer to a heap-allocated ColorArg struct. If used with
            colr() or Colr(), __it will be free()'d__.
            This is to allow for an easy interface.
            If used outside of those macros, __you must free() it__.
*/
#define back(x) ColorArg_to_ptr(back_arg(x))
/*! \def back_str
    Retrieve just the escape code string for a back color.

    \details
    You must free() the resulting string.

    \pi x A BasicValue, ExtendedValue, or RGB struct.

    \sa back_arg
*/
#define back_str(x) ColorArg_to_str(back_arg(x))

/*! \def style_arg
    Uses ColorArg_from_StyleValue to build a ColorArg with the appropriate color
    type/value.

    \pi x   `StyleValue` for the style.
    \return A ColorArg with the STYLE type set, and it's `.value.type` set
            for the appropriate color type/value.
            For invalid values the `.value.type` may be set to TYPE_INVALID.
*/
#define style_arg(x) \
    _Generic( \
        (x), \
        char *: ColorArg_from_str, \
        StyleValue: ColorArg_from_StyleValue \
    )(STYLE, x)


/*! \def style
    Create a style suitable for use with the colr() and Colr() macros.


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
    Style names (`char *`) can be passed to generate the appropriate color value.

    \example style_macro.c

    \pi x   A StyleValue.
    \return A pointer to a heap-allocated ColorArg struct. If used with
            colr() or Colr(), __it will be free()'d__.
            This is to allow for an easy interface.
            If used outside of those macros, __you must free() it__.
*/
#define style(x) ColorArg_to_ptr(style_arg(x))
/*! \def style_str
    Retrieve just the escape code string for a style.

    \details
    You must free() the resulting string.

    \pi x StyleValue to use.

    \sa style_arg
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
*/
#define colr(...) Colr_join(__VA_ARGS__, NULL)
// TODO: colr and Colr examples in their docs, and on the main pages.
// TODO: ColrJoin (see colr_tool.c TODOS).

/*! \def Colr
    Returns a heap-allocated ColorText object that can be used by itself,
    or with the colr() macro.

    \details
    You must `free()` the resulting ColorText struct using ColorText_free(),
    unless you pass it to colr(), which will `free()` it for you.
*/
#define Colr(...) ColorText_to_ptr(ColorText_from_values(__VA_ARGS__, NULL))

/*! Basic color values, with a few convenience values for extended colors.

    \details
    Values greater than 9 will trigger the use of an ExtendedValue (and it's
    associated functions).

    \internal
    The enum values are immediately defined to be explicit casts to the
    BasicValue_t type, this ensure no confusion between StyleValue and
    BasicValue when they are passed to _Generic-based macros.
    \endinternal
*/
typedef enum BasicValue_t {
    COLOR_INVALID = -2,
    COLOR_NONE = -1,
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
    // The following colors trigger extended color code use (format_fgx()).
    // The actual 256-color value is BasicValue - 20.
    XBLACK = 20,
    XRED = 22,
    XGREEN = 22,
    XYELLOW = 23,
    XBLUE = 24,
    XMAGENTA = 25,
    XCYAN = 26,
    XWHITE = 27,
    // The same "bright" colors as above, using the 256-color code.
    // The actual 256-color value is BasicValue - 20.
    XLIGHTBLACK = 28,
    XLIGHTRED = 29,
    XLIGHTGREEN = 30,
    XLIGHTYELLOW = 31,
    XLIGHTBLUE = 33,
    XLIGHTMAGENTA = 33,
    XLIGHTCYAN = 34,
    XLIGHTWHITE = 35,
} BasicValue;

#ifndef DOXYGEN_SKIP
#define COLOR_INVALID basic(COLOR_INVALID)
#define COLOR_NONE basic(COLOR_NONE)
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
#define XBLACK basic(XBLACK)
#define XRED basic(XRED)
#define XGREEN basic(XGREEN)
#define XYELLOW basic(XYELLOW)
#define XBLUE basic(XBLUE)
#define XMAGENTA basic(XMAGENTA)
#define XCYAN basic(XCYAN)
#define XWHITE basic(XWHITE)
#define XLIGHTBLACK basic(XLIGHTBLACK)
#define XLIGHTRED basic(XLIGHTRED)
#define XLIGHTGREEN basic(XLIGHTGREEN)
#define XLIGHTYELLOW basic(XLIGHTYELLOW)
#define XLIGHTBLUE basic(XLIGHTBLUE)
#define XLIGHTMAGENTA basic(XLIGHTMAGENTA)
#define XLIGHTCYAN basic(XLIGHTCYAN)
#define XLIGHTWHITE basic(XLIGHTWHITE)
#endif // DOXYGEN_SKIP

//! Convenience `typedef` for clarity when dealing with extended (256) colors.
typedef unsigned char ExtendedValue;

//! Container for RGB values.
struct RGB {
    unsigned char red;
    unsigned char blue;
    unsigned char green;
};

//! Style values.
typedef enum StyleValue_t {
    STYLE_INVALID = -2,
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
    TYPE_INVALID_EXTENDED_RANGE = -4,
    TYPE_INVALID_RGB_RANGE = -3,
    TYPE_INVALID = -2,
    TYPE_BASIC = 0,
    TYPE_EXTENDED = 1,
    TYPE_RGB = 2,
    TYPE_STYLE = 3,
} ColorType;



/*! Holds a known color name and it's `BasicValue`.

    \details
    This is used for the `color_names` array in colr.c.
*/
struct ColorInfo {
    char *name;
    BasicValue color;
};
/*! Holds a known style name and it's `StyleValue`.

    \details
    This is used for the `style_names` array in colr.c.
*/
struct StyleInfo {
    char *name;
    StyleValue style;
};

/*! Holds a color type and it's value.

    \details
    The `.type` member must always match the type of color value it is holding.

    \details
    This is internal. It's used to make the final interface easier to use.
    You probably shouldn't be using it.
*/
struct ColorValue {
    ColorType type;
    BasicValue basic;
    ExtendedValue ext;
    struct RGB rgb;
    StyleValue style;
};

/*! Holds an ArgType, and a ColorValue.
*/
struct ColorArg {
    unsigned int marker;
    ArgType type;
    struct ColorValue value;
};


/*! Holds a string of text, and optional fore, back, and style ColorArgs.
*/
struct ColorText {
    unsigned int marker;
    char *text;
    struct ColorArg *fore;
    struct ColorArg *back;
    struct ColorArg *style;
};

/*! \internal
    Allows access to color_names (implemented in colr.c).
    \endinternal
*/
extern struct ColorInfo color_names[];

/*! \internal
    Allows access to color_names_len (implemented in colr.c).
    \endinternal
*/
extern size_t color_names_len;

/*! \internal
    Allows access to style_names (implemented in colr.c).
    \endinternal
*/
extern struct StyleInfo style_names[];

/*! \internal
    Allows access to style_names_len (implemented in colr.c).
    \endinternal
*/
extern size_t style_names_len;


/*! \file colr.h
    Common macros and definitions are found here in colr.h,
    however the functions are documented in colr.c.
*/
char *colr_empty_str(void);
void format_bgx(char *out, unsigned char num);
void format_bg(char *out, BasicValue value);
void format_bg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue);
void format_bg_RGB(char *out, struct RGB rgb);
void format_fgx(char *out, unsigned char num);
void format_fg(char *out, BasicValue value);
void format_fg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue);
void format_fg_RGB(char *out, struct RGB rgb);
void format_rainbow_fore(char *out, double freq, size_t step);
void format_style(char *out, StyleValue style);

char *str_copy(char *dest, const char *src, size_t length);
char *str_noop(char *s);
int str_startswith(const char *s, const char *prefix);
void str_tolower(char *out, const char *s);


/*! \internal
    The multi-type variadiac function behind the colr() macro.
    \endinternal
*/
char *Colr_join(void *ctext, ...);

/*! \internal
    ArgType functions that only deal with argument types (fore, back, style).
    \endinternal
*/
char *ArgType_repr(ArgType type);

/*! \internal
    ColorArg functions that deal with an ArgType, and a ColorValue.
    \endinternal
*/
void ColorArg_free(struct ColorArg *p);
struct ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value);
struct ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value);
struct ColorArg ColorArg_from_RGB(ArgType type, struct RGB value);
struct ColorArg ColorArg_from_str(ArgType type, char *colorname);
struct ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value);
struct ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void *p);
bool ColorArg_is_invalid(struct ColorArg carg);
bool ColorArg_is_ptr(void *p);
bool ColorArg_is_valid(struct ColorArg carg);
struct ColorArg *ColorArg_to_ptr(struct ColorArg carg);
char *ColorArg_repr(struct ColorArg carg);
char *ColorArg_to_str(struct ColorArg carg);

/*! \internal
    ColorText functions that deal with a string of text, and fore/back/style
    ColorArgs.
    \endinternal
*/
void ColorText_free(struct ColorText *p);
struct ColorText ColorText_from_values(char *text, ...);
bool ColorText_is_ptr(void *p);
char *ColorText_repr(struct ColorText);
struct ColorText *ColorText_to_ptr(struct ColorText ctext);
char *ColorText_to_str(struct ColorText ctext);

/*! \internal
    ColorType functions that deal with the type of ColorValue (basic, ext, rgb.)
    \endinternal
*/
ColorType ColorType_from_str(const char *arg);
bool ColorType_is_invalid(ColorType type);
bool ColorType_is_valid(ColorType type);
char *ColorType_repr(ColorType type);

/*! \internal
    ColorValue functions that deal with a specific color value (basic, ext, rgb).
    \endinternal
*/
struct ColorValue ColorValue_from_str(char *s);
struct ColorValue ColorValue_from_value(ColorType type, void *p);
bool ColorValue_is_invalid(struct ColorValue cval);
char *ColorValue_repr(struct ColorValue cval);
char *ColorValue_to_str(ArgType type, struct ColorValue cval);


/*! \internal
    Functions that deal with basic color values, and their types.
    \endinternal
*/
BasicValue BasicValue_from_str(const char *arg);
int ExtendedValue_from_str(const char *arg);
int rgb_from_str(const char *arg, unsigned char *r, unsigned char *g, unsigned char *b);
int RGB_from_str(const char *arg, struct RGB *rgb);
StyleValue StyleValue_from_str(const char *arg);


void colrfgrainbow(char *out, const char *s, double freq, size_t offset);
char *acolrfgrainbow(const char *s, double freq, size_t offset);
#endif // COLR_H
