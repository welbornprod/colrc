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
#include <stdarg.h>
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
#define STYLE_RESET_ALL "\033[0m"

//! Maximum length for a basic fore/back escape code, including `'\0'`.
#define CODE_LEN 10
//! Maximum length for an extended fore/back escape code, including `'\0'`.
#define CODEX_LEN 15

//! Maximum length for a style escape code, including `'\0'`.
#define STYLE_LEN 8

/*! Maximum length in chars for any combination of basic/extended escape codes.

    Should be `(CODEX_LEN * 2) + STYLE_LEN`.
    Allocating for a string that will be colorized must account for this.
*/
#define COLOR_LEN 40

//! Maximum length in chars for an RGB fore/back escape code.
#define CODE_RGB_LEN 23
/*! Maximum length in chars added to a rgb colorized string.

    Should be `CODE_RGB_LEN + STYLE_LEN`
    Allocating for a string that will be colorized with rgb values must account
    for this.
*/
#define COLOR_RGB_LEN 32
/*! Maximum length in chars for any possible escape code mixture.

    (basically `(CODE_RGB_LEN * 2) + STYLE_LEN` since rgb codes are the longest).
*/
#define CODE_ANY_LEN 54
//! Maximim string length for a fore, back, or style name.
#define MAX_COLOR_NAME_LEN 12

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

/*! \def streq
    Convenience macro for `!strcmp(s1, s2)`.

    \pi s1 The first string to compare.
    \pi s2 The second string to compare.

    \retval 0 if \p s1 and \p s2 are equal.
    \retval 1 if \p s1 is greater than \p s2.
    \retval 1 if \p s1 is less than \p s2.
*/
#define streq(s1, s2) (!strcmp(s1, s2))

/*! \def argeq
    Convenience macro for `!strcmp(arg, s1) || !strcmp(arg, s2)`

    \pi arg String to check.
    \pi s1  First string to compare against.
    \pi s2  Second string to compare against.

    \return Non-zero if \p arg matches either \p s1 or \p s2, otherwise `0`.
*/
#define argeq(arg, s1, s2) (!strcmp(arg, s1)) || (!strcmp(arg, s2))

//! Convenience macro for `fprintf(stderr, ...)`.
#define printferr(...) fprintf(stderr, __VA_ARGS__)

/*! \def color_arg
    Builds a correct ColorArg struct according to the type of it's first
    argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a ColorArg.

    \pi x `BasicValue`, `Extended` (`unsigned char`). or `RGB` value.
    \return  ColorArg_from_value([appropriate type], x)
*/
#define color_arg(x) \
    _Generic( \
        (x), \
        BasicValue: ColorArg_from_value(TYPE_BASIC, &x), \
        ExtendedValue: ColorArg_from_value(TYPE_EXTENDED, &x), \
        struct RGB: ColorArg_from_value(TYPE_RGB, &x) \
    )
/*! Uses the correct format_fg* function according to the type of it's first
    argument.

    \details
    Uses `_Generic` (C11 standard) to dynamically create a fore color
    escape code string.

    \po out `char*` with memory allocated for the escape code string.
    \pi x   `BasicValue`, `Extended` (`unsigned char`), or `RGB` value for fore color.
*/
#define format_fore(out, x) \
    _Generic( \
        (x), \
        struct RGB: format_fg_RGB, \
        BasicValue: format_fg, \
        ExtendedValue: format_fgx \
    )(out, x)

/*! Uses the correct format_bg* function according to the type of it's first
    argument.
    \details
    Uses `_Generic` (C11 standard) to dynamically create a back color
    escape code string.

    \po out `char*` with memory allocated for the escape code string.
    \pi x   `BasicValue`, `Extended` (`unsigned char`), or `RGB` value for fore color.
*/
#define format_back(out, x) \
    _Generic( \
        (x), \
        RGB: format_bg_RGB, \
        BasicValue: format_bg, \
        unsigned char: format_bgx \
    )(out, x)

/*! Uses the format_fore/back macros, along with format_style, to build a
    style (string of escape codes).
    \details
    Uses `_Generic` (C11 standard) to dynamically create a colorized/styled
    string.

    \details
    Arguments are passed to format_fore(), format_back(), and format_style().
    \details
    The string of codes is built in place, and copied into the `out` parameter.

    \po out   char *buffer, must have a size of at least CODE_ANY_LEN.
    \pi fore  BasicValue, ExtendedValue, or RGB for fore color.
    \pi back  BasicValue, ExtendedValue, or RGB for back color.
    \pi style StyleValue for style.
*/
#define format_all(out, fore, back, style) \
    do { \
        char _fa_fore[CODE_RGB_LEN]; \
        format_fore(_fa_fore, fore); \
        char _fa_back[CODE_RGB_LEN]; \
        format_back(_fa_back, back); \
        char _fa_style[STYLE_LEN]; \
        format_style(_fa_style, style); \
        sprintf(out, "%s%s%s", _fa_style, _fa_fore, _fa_back); \
    } while (0)

/*! Transforms several ColrC objects into strings. If a string is given, this does nothing.
    \details
    Uses _Generic (C11 standard) to dynamically ensure a string.

    \details
    This is used to dynamically join strings and colors.

    \details
    Supported Types:
        - struct ColorArg
        - ColorType
        - char*

    \pi x   A string, ColorArg, or ColorType to transform into a string.
            \remark _Obviously this is a no-op for strings._
    \return Either the string that was given, or a stringified version of what was given.
*/
#define force_str(x) \
    _Generic( \
        (x), \
        struct ColorArg: ColorArg_repr, \
        ColorType: ColorType_repr, \
        char*: str_noop \
    )(x)

/*! Creates an anonymous RGB struct for use in function calls.

    \pi r `unsigned char` Red value.
    \pi g `unsigned char` Blue value.
    \pi b `unsigned char` Green value.

*/
#define rgb(r, g, b) ((struct RGB){.red=r, .green=g, .blue=b})

/*! Casts to ExtendedValue (unsigned char).
    \def ext
    \pi x Value to cast to `unsigned char`/`ExtendedValue`.
*/
#define ext(x) (ExtendedValue)(x)


/*! Basic color values, with a few convenience values for extended colors.

    \details
    Values greater than 9 will trigger the use of an ExtendedValue (and it's
    associated functions).
*/
typedef enum BasicValue_t {
    COLOR_INVALID = -2,
    COLOR_NONE = -1,
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
    // The following colors trigger extended color code use.
    XRED = 10,
    XGREEN = 11,
    XYELLOW = 12,
    XBLUE = 13,
    XMAGENTA = 14,
    XCYAN = 15,
    XNORMAL = 16,
    LIGHTRED = 17,
    LIGHTGREEN = 18,
    LIGHTYELLOW = 19,
    LIGHTBLUE = 20,
    LIGHTMAGENTA = 21,
    LIGHTCYAN = 22,
    LIGHTNORMAL = 23
} BasicValue;

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


//! Color code name types. Used with ColorType_from_str().
typedef enum ColorType_t {
    TYPE_INVALID_EXTENDED_RANGE = -4,
    TYPE_INVALID_RGB_RANGE = -3,
    TYPE_INVALID = -2,
    TYPE_BASIC = 0,
    TYPE_EXTENDED = 1,
    TYPE_RGB = 2,
} ColorType;

/*! Holds a known color name and it's `BasicValue`.
    \struct ColorInfo

    \details
    This is used for the `color_names` array in colr.c.
*/
struct ColorInfo {
    // TODO: Map these, like Colr.py.
    char *name;
    BasicValue color;
};
/*! Holds a known style name and it's `StyleValue`.
    \struct StyleInfo

    \details
    This is used for the `style_names` array in colr.c.
*/
struct StyleInfo {
    // TODO: Map these, like Colr.py.
    char *name;
    StyleValue style;
};

/*! Holds an arg type and it's value for a single fore/back color arg.
    \struct ColorArg

    \details
    The `.type` member must always match the type of color value it is holding.

    \details
    This is internal. It's used to make the final interface easier to use.
    You probably shouldn't be using it.
*/
struct ColorArg {
    ColorType type;
    BasicValue basic;
    ExtendedValue ext;
    struct RGB rgb;
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

struct ColorArg ColorArg_from_str(char *s);
struct ColorArg ColorArg_from_value(ColorType type, void *p);
char *ColorArg_repr(struct ColorArg);

ColorType ColorType_from_str(const char *arg);
char *ColorType_repr(ColorType type);

BasicValue BasicValue_from_str(const char *arg);
int ExtendedValue_from_str(const char *arg);
int rgb_from_str(const char *arg, unsigned char *r, unsigned char *g, unsigned char *b);
int RGB_from_str(const char *arg, struct RGB *rgb);
StyleValue StyleValue_from_str(const char *arg);

void colrbg(char *out, const char *s, BasicValue back);
void colrbgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue);
void colrbgRGB(char *out, const char *s, struct RGB rgb);
void colrbgx(char *out, const char *s, unsigned char num);
void colrfg(char *out, const char *s, BasicValue fore);
void colrfgchar(char *out, const char c, BasicValue fore);
void colrfgrainbow(char *out, const char *s, double freq, size_t offset);
char *acolrfgrainbow(const char *s, double freq, size_t offset);
void colrfgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue);
void colrfgRGB(char *out, const char *s, struct RGB rgb);
void colrfgx(char *out, const char *s, unsigned char num);
void colrize(char *out, const char *s, BasicValue fore, BasicValue back, StyleValue style);
void colrizechar(char *out, char c, BasicValue fore, BasicValue back, StyleValue style);
void colrizex(
    char *out,
    const char *s,
    unsigned char forenum, unsigned char backnum, StyleValue style);
void colrstyle(char *out, const char *s, StyleValue style);
#endif // COLR_H
