/*  colr.c
    Colr implementations.

    -Christopher Welborn 06-22-2019
*/
/*! \file
    Implements everything in the colr.h header.
*/
#include "colr.h"
/*! \file colr.h
    \brief Possible error return value for `colorname_to_color()`, `colorname_to_colorx()`,
    \brief and `colorname_to_rgb()`.
*/
const int COLORVAL_INVALID = -2;
/*! \file colr.h
    \brief Possible error return value for `colorname_to_rgb()`.
*/
const int COLORVAL_INVALID_RANGE = -1;

/*! \file colr.h
    \brief A list of `ColorInfo` items, used with `colorname_to_color()`.
*/
struct ColorInfo color_names[] = {
    {"none", COLOR_NONE},
    {"black", BLACK},
    {"blue", BLUE},
    {"cyan", CYAN},
    {"green", GREEN},
    {"magenta", MAGENTA},
    {"red", RED},
    {"reset", RESET},
    {"yellow", YELLOW},
    {"white", WHITE},
    {"lightblue", LIGHTBLUE},
    {"lightcyan", LIGHTCYAN},
    {"lightgreen", LIGHTGREEN},
    {"lightmagenta", LIGHTMAGENTA},
    {"lightnormal", LIGHTNORMAL},
    {"lightred", LIGHTRED},
    {"lightyellow", LIGHTYELLOW},
    {"xblue", XBLUE},
    {"xcyan", XCYAN},
    {"xgreen", XGREEN},
    {"xmagenta", XMAGENTA},
    {"xnormal", XNORMAL},
    {"xred", XRED},
    {"xyellow", XYELLOW},
};

/*! \file colr.h
    \brief Length of `color_names`.
*/
size_t color_names_len = sizeof color_names / sizeof (struct ColorInfo);

/*! \file colr.h
    \brief A list of `StyleInfo` items, used with `stylename_to_style()`.
*/
struct StyleInfo style_names[] = {
    {"none", STYLE_NONE},
    {"reset", RESET_ALL},
    {"bold", BRIGHT},
    {"bright", BRIGHT},
    {"dim", DIM},
    {"italic", ITALIC},
    {"underline", UNDERLINE},
    {"flash", FLASH},
    {"highlight", HIGHLIGHT},
    {"normal", NORMAL},
};

/*! \file colr.h
    \brief Length of `style_names`.
*/
size_t style_names_len = sizeof style_names / sizeof (struct StyleInfo);

void
format_bgx(char *out, unsigned char num) {
    /*  Create an escape code for an extended background color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODEX_LEN`.
            value : Value to use for background.
    */
    snprintf(out, CODEX_LEN, "\033[48;5;%dm", num);
}

void
format_bg(char *out, BasicValue value) {
    /*  Create an escape code for a background color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODEX_LEN`.
            value : BasicValue value to use for background.
    */
    if (value > 9) {
        format_bgx(out, value - (value > 16 ? 8: 9));
    } else {
        snprintf(out, CODE_LEN, "\033[%dm", (value < 0 ? RESET: value) + 40);
    }
}

void
format_bg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue) {
    /*  Create an escape code for a true color (rgb) background color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODE_RGB_LEN`.
            red   : Value for red.
            green : Value for green.
            blue  : Value for blue.
    */
    snprintf(out, CODE_RGB_LEN, "\033[48;2;%d;%d;%dm", red, green, blue);
}

void
format_bg_RGB(char *out, RGB rgb) {
    /*  Create an escape code for a true color (rgb) background color
        using values from an RGB struct.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODE_RGB_LEN`.
            rgb   : RGB struct to get red, blue, and green values from.
    */
    format_bg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

void
format_fgx(char *out, unsigned char num) {
    /*  Create an escape code for an extended fore color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODEX_LEN`.
            value : Value to use for fore.
    */
    snprintf(out, CODEX_LEN, "\033[38;5;%dm", num);
}

void
format_fg(char *out, BasicValue value) {
    /*  Create an escape code for a fore color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODEX_LEN`.
            value : BasicValue value to use for fore.
    */
    if (value > 9) {
        format_fgx(out, value - (value > 16 ? 8: 9));
    } else {
        snprintf(out, CODE_LEN, "\033[%dm", (value < 0 ? RESET: value) + 30);
    }
}

void
format_fg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue) {
    /*  Create an escape code for a true color (rgb) fore color.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `CODE_RGB_LEN`.
            red   : Value for red.
            green : Value for green.
            blue  : Value for blue.
    */
    snprintf(out, CODE_RGB_LEN, "\033[38;2;%d;%d;%dm", red, green, blue);
}

void
format_fg_RGB(char *out, RGB rgb) {
    /*  Create an escape code for a true color (rgb) fore color using an
        RGB struct's values.
        Arguments:
            out  : Memory allocated for the escape code string.
            rgb  : Pointer to an RGB struct.
    */
    format_fg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

void
format_rainbow_fore(char *out, double freq, size_t step) {
    /*  A single step in rainbow-izing a string.
        Arguments:
            out  : Memory allocated for the escape code string.
            freq : Frequency ("tightness") of the colors.
            step : Offset from the start of the rainbow.
                   Usually an index into a string.
    */
    double red = sin(freq * step + 0) * 127 + 128;
    double green = sin(freq * step + 2 * M_PI / 3) * 127 + 128;
    double blue = sin(freq * step + 4 * M_PI / 3) * 127 + 128;
    format_fg_rgb(
        out,
        (unsigned char)red,
        (unsigned char)green,
        (unsigned char)blue
    );
}

void
format_style(char *out, StyleValue style) {
    /*  Create an escape code for a style.
        Arguments:
            out   : Memory allocated for the escape code string.
                    *Must have enough room for `STYLE_LEN`.
            style : StyleValue value to use for style.
    */
    snprintf(out, STYLE_LEN, "\033[%dm", style < 0 ? RESET_ALL: style);
}


char*
str_copy(char *dest, const char *src, size_t length) {
    /*  Like `strncopy`, but ensures null-termination.

        If `src` is NULL, or `dest` is NULL, NULL is returned.
        On success, a pointer to `dest` is returned.

        *Warning: If `src` does not contain a null-terminator, this function
         will truncate at `length` characters.

        A null-terminator is always appended to `dest`.

        Arguments:
            dest    : Memory allocated for new string.
                      *Must have room for `strlen(src)`.
            src     : Source string to copy.
            length  : Maximum characters to copy, if `src` is not
                      null-terminated.
    */
    if (!(src && dest)) {
        return NULL;
    }
    size_t pos;
    size_t maxchars = length - 1;
    for (pos=0; pos < maxchars && src[pos] != '\0'; pos++) {
        dest[pos] = src[pos];
    }
    dest[pos] = '\0';
    return dest;
}

int
str_startswith(const char *s, const char *prefix) {
    /*  Returns non-zero if the string `s` starts with `prefix`.
        Returns 0 if one of the strings is null, or the prefix isn't found.

        *Warning: `prefix` must be null-terminated.

        Arguments:
            s      : The string to check.
            prefix : The prefix string to look for.
    */
    if (!s || !prefix) {
        // One of the strings is null.
        return 0;
    }
    size_t pre_len = strlen(prefix);
    for (size_t i = 0; i < pre_len; i++) {
        if (s[i] == '\0') {
            // Reached the end of `s` before the end of `prefix`.
            return 0;
        }
        if (prefix[i] != s[i]) {
            // Character differs from the prefix.
            return 0;
        }
    }
    return 1;
}

void
str_tolower(char *out, const char *s) {
    int length = 0;
    for (int i = 0; s[i]; i++) {
        length++;
        out[i] = tolower(s[i]);
    }
    out[length] = '\0';
}

/* ---------------------------- Colr Functions ---------------------------- */
BasicValue
colorname_to_color(const char *arg) {
    /*  Convert named argument to an actual BasicValue enum value.
        Returns a BasicValue value on success, or COLOR_INVALID on error.
    */
    char arglower[MAX_COLOR_NAME_LEN];
    str_tolower(arglower, arg);
    for (size_t i=0; i < color_names_len; i++) {
        if (!strcmp(arglower, color_names[i].name)) {
            return color_names[i].color;
        }
    }

    return COLOR_INVALID;
}

int
colorname_to_colorx(const char *arg) {
    /*  Converts an integer string (0-255) into a number suitable
        for the colrx functions.
        Returns a value between 0 and 255 on success.
        Returns `COLORVAL_INVALID` on error or bad values.
    */
    if (streq(arg, "xred")) return XRED;
    if (streq(arg, "xgreen")) return XGREEN;
    if (streq(arg, "xyellow")) return XYELLOW;
    if (streq(arg, "xblue")) return XBLUE;
    if (streq(arg, "xmagenta")) return XMAGENTA;
    if (streq(arg, "xcyan")) return XCYAN;
    if (streq(arg, "xnormal")) return XNORMAL;
    if (streq(arg, "lightred")) return LIGHTRED;
    if (streq(arg, "lightgreen")) return LIGHTGREEN;
    if (streq(arg, "lightyellow")) return LIGHTYELLOW;
    if (streq(arg, "lightblue")) return LIGHTBLUE;
    if (streq(arg, "lightmagenta")) return LIGHTMAGENTA;
    if (streq(arg, "lightcyan")) return LIGHTCYAN;
    if (streq(arg, "lightnormal")) return LIGHTNORMAL;

    // Using `long` to combat easy overflow.
    long usernum;
    if (!sscanf(arg, "%ld", &usernum)) {
        // Not a number.
        return COLORVAL_INVALID;
    }
    if (usernum < 0 || usernum > 255) {
        return COLORVAL_INVALID_RANGE;
    }
    return (int)usernum;
}

int
colorname_to_color_rgb(const char *arg, unsigned char *r, unsigned char *g, unsigned char *b) {
    /*  Convert an RGB string into red,green,blue values suitable for the
        colr*rgb functions.

        The format for RGB strings can be one of:
            "RED,GREEN,BLUE"
            "RED GREEN BLUE"
            "RED:GREEN:BLUE"

        Returns 0 on success.
        Returns either COLORVAL_INVALID, or COLORVAL_INVALID_RANGE on error.

        Arguments:
            arg  : String to check for RGB values.
            r    : Pointer to an unsigned char for red value on success.
            g    : Pointer to an unsigned char for green value on success.
            b    : Pointer to an unsigned char for blue value on success.

    */
    const char *formats[4] = {
        "%ld,%ld,%ld",
        "%ld %ld %ld",
        "%ld:%ld:%ld",
        NULL
    };
    long userred, usergreen, userblue;
    int i = 0;
    while (formats[i]) {
        if (sscanf(arg, formats[i], &userred, &usergreen, &userblue) == 3) {
            // Found a match.
            if (userred < 0 || userred > 255) return COLORVAL_INVALID_RANGE;
            if (usergreen < 0 || usergreen > 255) return COLORVAL_INVALID_RANGE;
            if (userblue < 0 || userblue > 255) return COLORVAL_INVALID_RANGE;
            // Valid ranges, set values for out parameters.
            *r = (unsigned char)userred;
            *g = (unsigned char)usergreen;
            *b = (unsigned char)userblue;
            return 0;
        }
        i++;
    }
    return COLORVAL_INVALID;
}

int
colorname_to_color_RGB(const char *arg, RGB *rgb) {
    /*  Convert an RGB string into a RGB struct suitable for the
        colr*RGB functions.

        The format for RGB strings can be one of:
            "RED,GREEN,BLUE"
            "RED GREEN BLUE"
            "RED:GREEN:BLUE"

        Returns 0 on success.
        Returns either COLORVAL_INVALID, or COLORVAL_INVALID_RANGE on error.

        Arguments:
            arg  : String to check for RGB values.
            rgb  : Pointer to an RGB struct to fill in the values for.
    */
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    int ret = colorname_to_color_rgb(arg, &r, &g, &b);
    if (ret) {
        // An error occurred.
        return ret;
    }
    rgb->red = r;
    rgb->green = g;
    rgb->blue = b;
    return 0;
}

ColorNameType
colorname_type(const char *arg) {
    /*  Determine which type of color value is desired by name.
        Example:
            "red" == COLORNAME_BASIC
            "253" == COLORNAME_EXTENDED
            "123,55,67" == COLORNAME_RGB

        Returns a ColorNameType value on success.
        On error, returns one of:
            COLORNAME_INVALID
            COLORNAME_INVALID_EXTENDED_RANGE
            COLORNAME_INVALID_RGB_RANGE
    */
    if (!arg) {
        return COLORNAME_INVALID;
    }
    // Try rgb first.
    unsigned char r, g, b;
    int rgb_ret = colorname_to_color_rgb(arg, &r, &g, &b);
    if (rgb_ret == COLORVAL_INVALID_RANGE) {
        return COLORNAME_INVALID_RGB_RANGE;
    } else if (rgb_ret != COLORNAME_INVALID) {
        return COLORNAME_RGB;
    }
    // Extended colors.
    int x_ret = colorname_to_colorx(arg);
    if (x_ret == COLORVAL_INVALID_RANGE) {
        return COLORNAME_INVALID_EXTENDED_RANGE;
    } else if (x_ret != COLORVAL_INVALID) {
        return COLORNAME_EXTENDED;
    }
    // Try basic colors.
    if (colorname_to_color(arg) != COLOR_INVALID) {
        return COLORNAME_BASIC;
    }
    return COLORNAME_INVALID;
}

void
colrbg(char *out, const char *s, BasicValue back) {
    /*  Prepend back color codes to a string, and copy the result into `out`.
        The STYLE_RESET_ALL code is already appended to the result.
        Arguments:
            out  : Memory allocated for the result.
                   *Must have enough room for `strlen(s) + COLOR_LEN`.
            s    : The string to colorize.
                   *Must be null-terminated.
            back : `BasicValue` code to use.
    */
    char backcode[CODEX_LEN];
    format_bg(backcode, back);
    size_t oldlen = strlen(s);
    size_t codeslen = CODEX_LEN + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s", backcode, s, STYLE_RESET_ALL
    );
}

void
colrbgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue) {
    /*  Colorize a string using true color, rgb back colors and copy the
        result into `out`.
        The StyleValue.RESET_ALL code is already appended to the result.
        Arguments:
            out   : Allocated memory to copy the result to.
                    *Must have enough room for `strlen(s) + COLOR_LEN`.
            s     : String to colorize.
                    *Must be null-terminated.
            red   : Value for red.
            green : Value for green.
            blue  : Value for blue.
    */
    char backcode[CODE_RGB_LEN];
    format_bg_rgb(backcode, red, green, blue);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, STYLE_RESET_ALL
    );
}

void
colrbgRGB(char *out, const char *s, RGB rgb) {
    /*  Colorize a string using true color, rgb back colors and copy the
        result into `out`.
        The StyleValue.RESET_ALL code is already appended to the result.
        Arguments:
            out   : Allocated memory to copy the result to.
                    *Must have enough room for `strlen(s) + COLOR_LEN`.
            s     : String to colorize.
                    *Must be null-terminated.
            rgb   : Pointer to an RGB struct to get the r, g, and b values.
    */
    char backcode[CODE_RGB_LEN];
    format_bg_RGB(backcode, rgb);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, STYLE_RESET_ALL
    );
}

void
colrbgx(char *out, const char *s, unsigned char num) {
    /*  Colorize a string using extended, 256, bg colors, and copy the
        result into `out`.
        The STYLE_RESET_ALL code is already appended to the result.
        Arguments:
            out  : Allocated memory to copy the result to.
                   *Must have enough room for `strlen(s) + COLOR_LEN`.
            s    : String to colorize.
                   *Must be null-terminated.
    */
    char backcode[CODEX_LEN];
    format_bgx(backcode, num);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;

    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, STYLE_RESET_ALL
    );
}

void
colrfg(char *out, const char *s, BasicValue fore) {
    /*  Prepend fore color codes to a string, and copy the result into `out`.
        The STYLE_RESET_ALL code is already appended to the result.
        Arguments:
            out  : Memory allocated for the result.
                   *Must have enough room for `strlen(s) + COLOR_LEN`.
            s    : The string to colorize.
                   *Must be null-terminated.
            fore : `BasicValue` code to use.
    */
    char forecode[CODEX_LEN];
    format_fg(forecode, fore);
    size_t oldlen = strlen(s);
    size_t codeslen = CODEX_LEN + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s", forecode, s, STYLE_RESET_ALL
    );
}

void
colrfgchar(char *out, const char c, BasicValue fore) {
    /*  Build a colorized string, from a single character.

        Arguments:
            out   : Memory allocated for the resulting string.
                    *Must have enough room for `COLOR_LEN + 1`.
            c     : Character to colorize.
            fore  : Fore color from `BasicValue` to use.
    */
    char s[2] = "\0\0";
    s[0] = c;
    colrfg(out, s, fore);
}

void
colrfgrainbow(char *out, const char *s, double freq, size_t offset) {
    /*  Rainbow-ize some text using rgb fore colors, lolcat style.
        The STYLE_RESET_ALL code is already appended to the result.
        Arguments:
            out    : Memory allocated for the result.
                     *Must have enough room for
                     `strlen(s) + (CODE_RGB_LEN * strlen(s))`.
            s      : The string to colorize.
                     *Must be null-terminated.
            freq   : Frequency ("tightness") for the colors.
            offset : Starting offset in the rainbow.
    */
    // Enough room for the escape code and one character.
    char codes[CODE_RGB_LEN];
    size_t singlecharlen = CODE_RGB_LEN + 1;
    char singlechar[singlecharlen];
    singlechar[0] = '\0';
    size_t oldlen = strlen(s);
    out[0] = '\0';
    for (size_t i = 0; i < oldlen; i++) {
        format_rainbow_fore(codes, freq, offset + i);
        snprintf(singlechar, singlecharlen, "%s%c", codes, s[i]);
        strncat(out, singlechar, CODE_RGB_LEN);
    }
    strncat(out, STYLE_RESET_ALL, STYLE_LEN);
}

char *
acolrfgrainbow(const char *s, double freq, size_t offset) {
    /*  Like `colrfgrainbow`, except it allocates the string for you, with
     enough room to fit the string and any escape codes needed.

     Returns the allocated/formatted string on success.

     Arguments:
         s      : The string to colorize.
                  *Must be null-terminated.
         freq   : Frequency ("tightness") for the colors.
         offset : Starting offset in the rainbow.
    */
    size_t oldlen = strlen(s);
    char *out = (char*)calloc(oldlen + (CODE_RGB_LEN * oldlen), sizeof(char));
    colrfgrainbow(out, s, freq, offset);
    return out;
}

void
colrfgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue) {
    /*  Colorize a string using true color, rgb fore colors and copy the
        result into `out`.
        The StyleValue.RESET_ALL code is already appended to the result.
        Arguments:
            out   : Allocated memory to copy the result to.
                    *Must have enough room for `strlen(s) + COLOR_RGB_LEN`.
            s     : String to colorize.
                    *Must be null-terminated.
            red   : Value for red.
            green : Value for green.
            blue  : Value for blue.
    */
    char forecode[CODE_RGB_LEN];
    format_fg_rgb(forecode, red, green, blue);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, STYLE_RESET_ALL
    );
}

void
colrfgRGB(char *out, const char *s, RGB rgb) {
    /*  Colorize a string using true color, rgb fore colors and copy the
        result into `out`.
        The StyleValue.RESET_ALL code is already appended to the result.
        Arguments:
            out  : Allocated memory to copy the result to.
                   *Must have enough room for `strlen(s) + COLOR_RGB_LEN`.
            rgb  : Pointer to an RGB struct to use for r, g, and b values.
    */
    char forecode[CODE_RGB_LEN];
    format_fg_RGB(forecode, rgb);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, STYLE_RESET_ALL
    );
}

void
colrfgx(char *out, const char *s, unsigned char num) {
    /*  Colorize a string using extended, 256 colors, and copy the
        result into `out`.
        The StyleValue.RESET_ALL code is already appended to the result.
        Arguments:
            out  : Allocated memory to copy the result to.
                   *Must have enough room for `strlen(s) + COLOR_LEN`.
            s    : String to colorize.
                   *Must be null-terminated.
            num  : Code number, 0-255 inclusive.
    */
    char forecode[CODEX_LEN];
    format_fgx(forecode, num);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;

    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, STYLE_RESET_ALL
    );
}


void
colrize(char *out, const char *s, BasicValue fore, BasicValue back, StyleValue style) {
    /*  Prepends escape codes for fore, back, and style to `s` and copies
        the result into `out`.
        Arguments:
            out       : Allocated memory to copy the result to.
                        *Must have enough room for `strlen(s) + COLOR_LEN`.
            s         : String to colorize.
                        *Must be null-terminated.
            fore      : BasicValue value to use for fore.
            back      : BasicValue value to use for background.
            style : StyleValue value to use.
    */

    // Build forecolor only.
    char forecode[CODEX_LEN];
    format_fg(forecode, fore);
    // Build backcolor only.
    char backcode[CODEX_LEN];
    format_bg(backcode, back);
    // Build style only.
    char stylecode[STYLE_LEN];
    format_style(stylecode, style);

    // Seperate reset codes from other codes by making sure normal codes
    // are used last.
    size_t codeslen = COLOR_LEN;
    char frontcodes[codeslen];
    frontcodes[0] = '\0';
    char endcodes[codeslen];
    endcodes[0] = '\0';
    if (style == RESET_ALL || style == NORMAL) {
            strncat(frontcodes, stylecode, STYLE_LEN);
    } else {
            strncat(endcodes, stylecode, STYLE_LEN);
    }
    strncat(fore == RESET ? frontcodes: endcodes, forecode, CODEX_LEN);
    strncat(back == RESET ? frontcodes: endcodes, backcode, CODEX_LEN);

    size_t oldlen = strlen(s);
    codeslen =  strlen(frontcodes) + strlen(endcodes) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s%s",
        frontcodes, endcodes, s, STYLE_RESET_ALL
    );
}

void
colrizechar(char *out, char c, BasicValue fore, BasicValue back, StyleValue style) {
    /*  Prepends escape codes for fore, back, and style to a character (`c`)
        and copies the result into `out`.
        Arguments:
            out       : Allocated memory to copy the result to.
                        *Must have enough room for `strlen(s) + COLOR_LEN`.
            c         : Character to colorize.
            fore      : BasicValue value to use for fore.
            back      : BasicValue value to use for background.
            style : StyleValue value to use.
    */
    char s[2] = "\0\0";
    s[0] = c;
    colrize(out, s, fore, back, style);
}

void
colrizex(
    char *out,
    const char *s,
    unsigned char forenum, unsigned char backnum, StyleValue style) {
    /*  Prepends escape codes for extended fore, back, and style to `s` and
        copies the result into an allocated string.
        The string must be freed by the caller.
        Arguments:
            out       : Allocated memory to copy the result to.
                        *Must have enough room for `strlen(s) + COLOR_LEN`.
            s         : String to colorize.
                        *Must be null-terminated.
            fore      : BasicValue value to use for fore.
            back      : BasicValue value to use for background.
            style : StyleValue value to use.
    */
    // Build forecolor only.
    char forecode[CODEX_LEN];
    format_fgx(forecode, forenum);
    // Build backcolor only.
    char backcode[CODEX_LEN];
    format_bgx(backcode, backnum);
    // Build style only.
    char stylecodes[STYLE_LEN];
    format_style(stylecodes, style);

    // Seperate reset codes from other codes by making sure normal codes
    // are used last.
    size_t codeslen = COLOR_LEN;
    char frontcodes[codeslen];
    frontcodes[0] = '\0';
    char endcodes[codeslen];
    endcodes[0] = '\0';
    strncat(
        (style == RESET_ALL ? frontcodes: endcodes),
        stylecodes,
        STYLE_LEN
    );
    strncat(frontcodes, forecode, CODEX_LEN);
    strncat(frontcodes, backcode, CODEX_LEN);

    size_t oldlen = strlen(s);
    codeslen =  strlen(frontcodes) + strlen(endcodes) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s%s",
        frontcodes, endcodes, s, STYLE_RESET_ALL
    );
}

void
colrstyle(char *out, const char *s, StyleValue style) {
    /*  Prepend style codes to a string, and copy the result into `out`.
        The STYLE_RESET_ALL code is already appended to the result.
        Arguments:
            out   : Memory allocated for the result.
                    *Must have enough room for `strlen(s) + COLOR_LEN`.
            s     : The string to colorize.
                    *Must be null-terminated.
            style : `StyleValue` code to use.
    */
    if (style < 0) style = RESET_ALL;
    char stylecode[STYLE_LEN];
    snprintf(stylecode, STYLE_LEN, "\033[%dm", style);

    size_t oldlen = strlen(s);
    size_t codeslen = strlen(stylecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        stylecode, s, STYLE_RESET_ALL
    );
}

StyleValue
stylename_to_style(const char *arg) {
    /*  Convert named argument to actual StyleValue enum value.
        Returns a StyleValue value on success, or STYLE_INVALID on error.
    */
    char arglower[MAX_COLOR_NAME_LEN];
    str_tolower(arglower, arg);

    for (size_t i=0; i < style_names_len; i++) {
        if (!strcmp(arglower, style_names[i].name)) {
            return style_names[i].style;
        }
    }
    return STYLE_INVALID;
}
