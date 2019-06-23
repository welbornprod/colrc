#ifndef COLR_H
#define COLR_H
//! \file

/*! \mainpage
    Documentation for ColrC.

    Files:
    - colr.h
    - colr.c
    - dbug.h

    \tableofcontents
*/
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
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
    // For the rainbow functions.
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

/*! \brief Maximum length in chars for any combination of basic/extended escape codes.

    Should be `(CODEX_LEN * 2) + STYLE_LEN`.
    Allocating for a string that will be colorized must account for this.
*/
#define COLOR_LEN 40

//! Maximum length in chars for an RGB fore/back escape code.
#define CODE_RGB_LEN 23
/*! \brief Maximum length in chars added to a rgb colorized string.

    Should be `CODE_RGB_LEN + STYLE_LEN`
    Allocating for a string that will be colorized with rgb values must account
    for this.
*/
#define COLOR_RGB_LEN 32
/*! \brief Maximum length in chars for any possible escape code mixture.

    (basically `(CODE_RGB_LEN * 2) + STYLE_LEN` since rgb codes are the longest).
*/
#define CODE_ANY_LEN 54
//! Maximim string length for a fore, back, or style name.
#define MAX_COLOR_NAME_LEN 12

/*! \brief Allocate `str_len` + enough for a basic code with reset appended.

    \param str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_code(str_len) (char*)calloc(str_len + CODEX_LEN, sizeof(char))
/*! \brief Allocate `str_len` + enough for a mixture of fore/basic codes.
    \param str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_codes(str_len) (char*)calloc(str_len + COLOR_LEN, sizeof(char))
/*! \brief Allocate `str_len` + enough for an rgb code with reset appended.

    \param str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_rgb(str_len) (char*)calloc(str_len + COLOR_RGB_LEN, sizeof(char))
/*! \brief Allocate `str_len` + enough for a style code with reset appended.

    \param str_len Extra room to allocate for text.
    \return Pointer to the allocated string, or NULL on error.
*/
#define alloc_with_style(str_len) (char*)calloc(str_len + STYLE_LEN, sizeof(char))

/*! \brief Convenience macro for `!strcmp(s1, s2)`.

    \param s1 The first string to compare.
    \param s2 The second string to compare.

    \return 0 if equal, 1 if \p s1 is greater, and -1 if \p s1 is less than.
*/
#define streq(s1, s2) (!strcmp(s1, s2))

/*! \brief Convenience macro for `!strcmp(arg, s1) || !strcmp(arg, s2)`

    \param arg String to check.
    \param s1  First string to compare against.
    \param s2  Second string to compare against.

    \return Non-zero if \p arg matches either \p s1 or \p s2, otherwise `0`.
*/
#define argeq(arg, s1, s2) (!strcmp(arg, s1)) || (!strcmp(arg, s2))

//! Convenience macro for `fprintf(stderr, ...)`.
#define printferr(...) fprintf(stderr, __VA_ARGS__)

/*! \brief Uses the correct format_fg* function according to the type of it's first
    \brief argument.

    \param out `char*` with memory allocated for the escape code string.
    \param x   `BasicValue`, `Extended`` (unsigned char), or `RGB` value for fore color.
*/
#define format_fore(out, x) \
    _Generic( \
        (x), \
        RGB: format_fg_RGB, \
        BasicValue: format_fg, \
        unsigned char: format_fgx \
    )(out, x)

/*! \brief Uses the correct format_bg* function according to the type of it's first
    \brief argument.

    \param out `char*` with memory allocated for the escape code string.
    \param x   `BasicValue`, `Extended`` (unsigned char), or `RGB` value for fore color.
*/
#define format_back(out, x) \
    _Generic( \
        (x), \
        RGB: format_bg_RGB, \
        BasicValue: format_bg, \
        unsigned char: format_bgx \
    )(out, x)

/*! \brief Uses the format_fore/back macros, along with format_style, to build a
    \brief style (string of escape codes).

        \param out   char *buffer, must have a size of at least CODE_ANY_LEN.
        \param fore  BasicValue, ExtendedValue, or RGB for fore color.
        \param back  BasicValue, ExtendedValue, or RGB for back color.
        \param style StyleValue for style.
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

/*! \brief Creates an anonymous RGB struct for use in function calls.

    \param r `unsigned char` Red value.
    \param g `unsigned char` Blue value.
    \param b `unsigned char` Green value.

*/
#define rgb(r, g, b) ((RGB){r, g, b})

/*! \brief Casts to ExtendedValue (unsigned char).

    \param x Value to cast to `unsigned char`/`ExtendedValue`.
*/
#define ext(x) (ExtendedValue)(x)


//!  Basic color values, with a few convenience values for extended colors.
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

//! Convenience `typedef` for clarity.
typedef unsigned char ExtendedValue;

//! Container for RGB values.
typedef struct RGB_t {
    unsigned char red;
    unsigned char blue;
    unsigned char green;
} RGB;

//!  Style values.
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


//!  Color code name types. Used with `colorname_type()`.
typedef enum ColorNameType_t {
    COLORNAME_INVALID_EXTENDED_RANGE = -4,
    COLORNAME_INVALID_RGB_RANGE = -3,
    COLORNAME_INVALID = -2,
    COLORNAME_BASIC = 0,
    COLORNAME_EXTENDED = 1,
    COLORNAME_RGB = 2,
} ColorNameType;

/*! \struct ColorInfo
    Holds a known color name and it's `BasicValue`.

    This is used for the `color_names` array in colr.c.
*/
struct ColorInfo {
    // TODO: Map these, like Colr.py.
    char *name;
    BasicValue color;
};
/*! \brief List of `ColorInfo` items.

    Initialized in colr.c.
*/
extern struct ColorInfo color_names[];

/*! \brief Length of `color_names`.

    Initialized in colr.c.
*/
extern size_t color_names_len;

/*! \struct StyleInfo
    Holds a known style name and it's `StyleValue`.

    This is used for the `style_names` array in colr.c.
*/
struct StyleInfo {
    // TODO: Map these, like Colr.py.
    char *name;
    StyleValue style;
};
/*! \brief A list of `StyleInfo`.

    Initialized in colr.c.
*/
extern struct StyleInfo style_names[];

/*! \brief Length of style_names.

    Initialized in colr.c.
*/
extern size_t style_names_len;

/*! \brief Returned from the `colorname_to_color*` functions for invalid values.
*/
extern const int COLORVAL_INVALID;
/*! \brief Returned from `colorname_to_colorx()` and `colorname_to_color_rgb()` for invalid values.
*/
extern const int COLORVAL_INVALID_RANGE;

void format_bgx(char *out, unsigned char num);
void format_bg(char *out, BasicValue value);
void format_bg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue);
void format_bg_RGB(char *out, RGB rgb);
void format_fgx(char *out, unsigned char num);
void format_fg(char *out, BasicValue value);
void format_fg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue);
void format_fg_RGB(char *out, RGB rgb);
void format_rainbow_fore(char *out, double freq, size_t step);
void format_style(char *out, StyleValue style);

char *str_copy(char *dest, const char *src, size_t length);
int str_startswith(const char *s, const char *prefix);
void str_tolower(char *out, const char *s);
BasicValue colorname_to_color(const char *arg);
int colorname_to_colorx(const char *arg);
int colorname_to_color_rgb(const char *arg, unsigned char *r, unsigned char *g, unsigned char *b);
int colorname_to_color_RGB(const char *arg, RGB *rgb);
ColorNameType colorname_type(const char *arg);
void colrbg(char *out, const char *s, BasicValue back);
void colrbgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue);
void colrbgRGB(char *out, const char *s, RGB rgb);
void colrbgx(char *out, const char *s, unsigned char num);
void colrfg(char *out, const char *s, BasicValue fore);
void colrfgchar(char *out, const char c, BasicValue fore);
void colrfgrainbow(char *out, const char *s, double freq, size_t offset);
char *acolrfgrainbow(const char *s, double freq, size_t offset);
void colrfgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue);
void colrfgRGB(char *out, const char *s, RGB rgb);
void colrfgx(char *out, const char *s, unsigned char num);
void colrize(char *out, const char *s, BasicValue fore, BasicValue back, StyleValue style);
void colrizechar(char *out, char c, BasicValue fore, BasicValue back, StyleValue style);
void colrizex(
    char *out,
    const char *s,
    unsigned char forenum, unsigned char backnum, StyleValue style);
void colrstyle(char *out, const char *s, StyleValue style);
StyleValue stylename_to_style(const char *arg);
#endif // COLR_H
