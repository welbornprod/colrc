/*! \file colr.c
    Implements everything in the colr.h header.

    \author Christopher Welborn
    \date 06-22-2019
*/
#include "colr.h"
/*! Possible error return value for BasicValue_from_str(), ExtendedValue_from_str(),
    and colorname_to_rgb().
*/
const int COLORVAL_INVALID = -2;
//! Possible error return value for rgb_from_str() and RGB_from_str().
const int COLORVAL_INVALID_RANGE = -1;

//! A list of ColorInfo items, used with BasicValue_from_str().
struct ColorInfo color_names[] = {
    {"none", COLOR_NONE},
    {"reset", RESET},
    {"black", BLACK},
    {"blue", BLUE},
    {"cyan", CYAN},
    {"green", GREEN},
    {"magenta", MAGENTA},
    {"normal", WHITE},
    {"red", RED},
    {"white", WHITE},
    {"yellow", YELLOW},
    {"lightblack", LIGHTBLACK},
    {"lightblue", LIGHTBLUE},
    {"lightcyan", LIGHTCYAN},
    {"lightgreen", LIGHTGREEN},
    {"lightmagenta", LIGHTMAGENTA},
    {"lightnormal", LIGHTWHITE},
    {"lightred", LIGHTRED},
    {"lightwhite", LIGHTWHITE},
    {"lightyellow", LIGHTYELLOW},
    {"xblue", XBLUE},
    {"xcyan", XCYAN},
    {"xgreen", XGREEN},
    {"xmagenta", XMAGENTA},
    {"xnormal", XWHITE},
    {"xred", XRED},
    {"xwhite", XWHITE},
    {"xyellow", XYELLOW},
    {"xlightblack", XLIGHTBLACK},
    {"xlightblue", XLIGHTBLUE},
    {"xlightcyan", XLIGHTCYAN},
    {"xlightgreen", XLIGHTGREEN},
    {"xlightmagenta", XLIGHTMAGENTA},
    {"xlightnormal", XLIGHTWHITE},
    {"xlightred", XLIGHTRED},
    {"xlightwhite", XLIGHTWHITE},
    {"xlightyellow", XLIGHTYELLOW},
};

//! Length of color_names.
size_t color_names_len = sizeof color_names / sizeof (struct ColorInfo);

//! A list of StyleInfo items, used with StyleName_from_str().
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

//! Length of style_names.
size_t style_names_len = sizeof style_names / sizeof (struct StyleInfo);

/*! Create an escape code for an extended background color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for background.
*/
void format_bgx(char *out, unsigned char num) {
    snprintf(out, CODEX_LEN, "\033[48;5;%dm", num);
}

/*! Create an escape code for a background color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for background.
*/
void format_bg(char *out, BasicValue value) {
    int use_value = (int)value;
    if (value < 0) {
        // Invalid, just use the RESET code.
        use_value = RESET;
    } else if (value >= 20) {
        // Use 256-colors.
        use_value -= 20;
        format_fgx(out, use_value);
    } else {
        if (value >= 10) {
            // Bright colors.
            use_value += 90;
        }  else {
            // Normal basic value.
            use_value += 40;
        }
        snprintf(out, CODE_LEN, "\033[%dm", use_value);
    }
}

/*! Create an escape code for a true color (rgb) background color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODE_RGB_LEN`._
    \pi red   Value for red.
    \pi green Value for green.
    \pi blue  Value for blue.
*/
void format_bg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue) {
    snprintf(out, CODE_RGB_LEN, "\033[48;2;%d;%d;%dm", red, green, blue);
}

/*! Create an escape code for a true color (rgb) background color
    using values from an RGB struct.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODE_RGB_LEN`._
    \pi rgb RGB struct to get red, blue, and green values from.
*/
void format_bg_RGB(char *out, struct RGB rgb) {
    format_bg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for a background color from a ColorArg.

    \po out  Memory allocated for the escape code string.
             _Must have enough room for `CODE_RGB_LEN`._
    \pi carg ColorArg value to get the value from.
*/
void format_bg_color_arg(char *out, struct ColorArg carg) {
    switch (carg.type) {
        case TYPE_BASIC:
            format_bg(out, carg.basic);
            break;
        case TYPE_EXTENDED:
            format_bgx(out, carg.ext);
            break;
        case TYPE_RGB:
            format_bg_RGB(out, carg.rgb);
            break;
        default:
            // Invalid color or color range.
            break;
    }
}

/*! Create an escape code for an extended fore color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for fore.
*/
void format_fgx(char *out, unsigned char num) {
    snprintf(out, CODEX_LEN, "\033[38;5;%dm", num);
}

/*! Create an escape code for a fore color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for fore.
*/
void format_fg(char *out, BasicValue value) {
    int use_value = (int)value;
    if (value < 0) {
        // Invalid, just use the RESET code.
        use_value = RESET;
    } else if (value >= 20) {
        // Use 256-colors.
        use_value -= 20;
        format_fgx(out, use_value);
    } else {
        if (value >= 10) {
            // Bright colors.
            use_value += 80;
        }  else {
            // Normal basic value.
            use_value += 30;
        }
        snprintf(out, CODE_LEN, "\033[%dm", use_value);
    }
}

/*! Create an escape code for a true color (rgb) fore color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODE_RGB_LEN`._
    \pi red   Value for red.
    \pi green Value for green.
    \pi blue  Value for blue.
*/
void format_fg_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue) {
    snprintf(out, CODE_RGB_LEN, "\033[38;2;%d;%d;%dm", red, green, blue);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB(char *out, struct RGB rgb) {
    format_fg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

/*! A single step in rainbow-izing a string.

    \po out  Memory allocated for the escape code string.
    \pi freq Frequency ("tightness") of the colors.
    \pi step Offset from the start of the rainbow.
             Usually an index into a string.
*/
void format_rainbow_fore(char *out, double freq, size_t step) {
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

/*! Create an escape code for a style.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `STYLE_LEN`._
    \pi style StyleValue value to use for style.
*/
void format_style(char *out, StyleValue style) {
    snprintf(out, STYLE_LEN, "\033[%dm", style < 0 ? RESET_ALL: style);
}


/*! Like strncopy, but ensures null-termination.

    \details
    If src is NULL, or dest is NULL, NULL is returned.

    \details
    If src does not contain a null-terminator, _this function
    will truncate at `length` characters_.

    \details
    A null-terminator is always appended to dest.

    \pi dest   Memory allocated for new string.
               _Must have room for `strlen(src)`._
    \pi src    Source string to copy.
    \pi length Maximum characters to copy, if src is not null-terminated.

    \returns On success, a pointer to dest is returned.
*/
char *str_copy(char *dest, const char *src, size_t length) {
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

/*! This is a no-op function that simply returns the pointer it is given.
    \details
    It is used in the force_str macro to dynamically ensure it's argument
    is converted to a string. If a string is passed to force_str it is
    simply returned as-is.

    \pi s   The string to return.
    \return The string that was given.
*/
char *str_noop(char *s) {
    return s;
}

/*! Checks a string for a certain prefix substring.

    \details
    _`prefix` must be null-terminated._

    \pi s      The string to check.
    \pi prefix The prefix string to look for.

    \return non-zero if the string `s` starts with prefix.
    \return 0 if one of the strings is null, or the prefix isn't found.
*/
int str_startswith(const char *s, const char *prefix) {
    if (!s || !prefix) {
        // One of the strings is null.
        return 0;
    }
    size_t pre_len = strlen(prefix);
    for (size_t i = 0; i < pre_len; i++) {
        if (s[i] == '\0') {
            // Reached the end of s before the end of prefix.
            return 0;
        }
        if (prefix[i] != s[i]) {
            // Character differs from the prefix.
            return 0;
        }
    }
    return 1;
}

/*! Converts a string into lower case, and copies it into `out`.
    \details
    _Input string must be null-terminated._

    \po out Memory allocated for the result.
            _Must have capacity for `strlen(s) + 1`._
    \pi s   The input string to convert to lower case.
*/
void str_tolower(char *out, const char *s) {
    int length = 0;
    for (int i = 0; s[i]; i++) {
        length++;
        out[i] = tolower(s[i]);
    }
    out[length] = '\0';
}

/* ---------------------------- Colr Functions ---------------------------- */

/*! Create a ColorArg from a known color name, or RGB string.

    \pi s    A string to parse the color name from (can be an RGB string).
    \return  A ColorArg (with no fore/back information, only the color type and value).
*/
struct ColorArg ColorArg_from_str(char *s) {
    if (!s) {
        return ColorArg_from_value(TYPE_INVALID, NULL);
    }
    // Get the actual type, even if it's invalid.
    ColorType type = ColorType_from_str(s);
    // Try rgb first.
    struct RGB rgb;
    int rgb_ret = RGB_from_str(s, &rgb);
    if (rgb_ret == COLORVAL_INVALID_RANGE) {
        return ColorArg_from_value(TYPE_INVALID_RGB_RANGE, NULL);
    } else if (rgb_ret != TYPE_INVALID) {
        return ColorArg_from_value(type, &rgb);
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(s);
    if (x_ret == COLORVAL_INVALID_RANGE) {
        return ColorArg_from_value(TYPE_INVALID_EXTENDED_RANGE, NULL);
    } else if (x_ret != COLORVAL_INVALID) {
        // Need to cast back into a real ExtendedValue now that I know it's
        // not invalid. Also, ColorArg_from_value expects a pointer, to
        // help with it's "dynamic" uses.
        ExtendedValue xval = (ExtendedValue)x_ret;
        return ColorArg_from_value(type, &xval);
    }
    // Try basic colors.
    int b_ret = BasicValue_from_str(s);
    if ( b_ret != COLOR_INVALID) {
        BasicValue bval = (BasicValue)b_ret;
        return ColorArg_from_value(type, &bval);
    }
    return ColorArg_from_value(TYPE_INVALID, NULL);
}

/*! Used with the color_arg macro to dynamically create a ColorArg based
    on it's argument type.

    \pi type A ColorType value, to mark the type of ColorArg.
    \pi p    A pointer to either a BasicValue, ExtendedValue, or a struct RGB.

    \return A ColorArg struct with the appropriate `.type` member set for
            the value that was passed. For invalid types the `.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXTENDED_RANGE
        - TYPE_INVALID_RGB_RANGE
*/
struct ColorArg ColorArg_from_value(ColorType type, void *p) {
    if (
        type == TYPE_INVALID ||
        type == TYPE_INVALID_EXTENDED_RANGE ||
        type == TYPE_INVALID_RGB_RANGE
        ) {
        return (struct ColorArg){.type=type};
    }
    if (!p) {
        return (struct ColorArg){.type=TYPE_INVALID};
    }
    if (type == TYPE_BASIC) {
        BasicValue *bval = p;
        return (struct ColorArg){.type=TYPE_BASIC, .basic=*bval};
    } else if (type == TYPE_EXTENDED) {
        ExtendedValue *eval = p;
        return (struct ColorArg){.type=TYPE_EXTENDED, .ext=*eval};
    } else if (type == TYPE_RGB) {
        struct RGB *rgbval = p;
        return (struct ColorArg){.type=TYPE_RGB, .rgb=*rgbval};
    }
    return (struct ColorArg){.type=type};
}

/*! Creates a string representation of a ColorArg.

    \pi carg    A ColorArg to get the type and value from.
    \return     A pointer to an allocated string. You must free() it.
*/
char *ColorArg_repr(struct ColorArg carg) {
    char *argstr;
    switch (carg.type) {
        case TYPE_RGB:
            asprintf(
                &argstr,
                "struct RGB {.red=%d, .green=%d, .blue=%d}",
                carg.rgb.red,
                carg.rgb.green,
                carg.rgb.blue
            );
            break;
        case TYPE_BASIC:
            asprintf(&argstr, "(BasicValue) %d", carg.basic);
            break;
        case TYPE_EXTENDED:
            asprintf(&argstr, "(ExtendedValue) %d", carg.ext);
            break;
        default:
            return ColorType_repr(carg.type);
    }
    return argstr;
}

/*! Determine which type of color value is desired by name.

    \details
    Example:
        - "red" == TYPE_BASIC
        - "253" == TYPE_EXTENDED
        - "123,55,67" == TYPE_RGB

    \pi arg Color name to get the ColorType for.

    \retval ColorType value on success.
    \retval TYPE_INVALID for invalid color names/strings.
    \retval TYPE_INVALID_EXTENDED_RANGE for ExtendedValues outside of 0-255.
    \retval TYPE_INVALID_RGB_RANGE for rgb values outside of 0-255.
*/
ColorType ColorType_from_str(const char *arg) {
    if (!arg) {
        return TYPE_INVALID;
    }
    // Try rgb first.
    unsigned char r, g, b;
    int rgb_ret = rgb_from_str(arg, &r, &g, &b);
    if (rgb_ret == COLORVAL_INVALID_RANGE) {
        return TYPE_INVALID_RGB_RANGE;
    } else if (rgb_ret != TYPE_INVALID) {
        return TYPE_RGB;
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(arg);
    if (x_ret == COLORVAL_INVALID_RANGE) {
        return TYPE_INVALID_EXTENDED_RANGE;
    } else if (x_ret != COLORVAL_INVALID) {
        return TYPE_EXTENDED;
    }
    // Try basic colors.
    if (BasicValue_from_str(arg) != COLOR_INVALID) {
        return TYPE_BASIC;
    }
    return TYPE_INVALID;
}

/*! Creates a string representation of a ColorType.

    \pi type A ColorType to get the type from.
    \return  A pointer to an allocated string. You must free() it.
*/
char *ColorType_repr(ColorType type) {
    char *typestr;
    switch (type) {
        case TYPE_BASIC:
            asprintf(&typestr, "TYPE_BASIC");
            break;
        case TYPE_EXTENDED:
            asprintf(&typestr, "TYPE_EXTENDED");
            break;
        case TYPE_RGB:
            asprintf(&typestr, "TYPE_RGB");
            break;
        case TYPE_INVALID:
            asprintf(&typestr, "TYPE_INVALID");
            break;
        case TYPE_INVALID_EXTENDED_RANGE:
            asprintf(&typestr, "TYPE_INVALID_EXTENDED_RANGE");
            break;
        case TYPE_INVALID_RGB_RANGE:
            asprintf(&typestr, "TYPE_INVALID_RGB_RANGE");
            break;
    }
    return typestr;
}

/*! Convert named argument to an actual BasicValue enum value.

    \pi arg Color name to find the BasicValue for.
    \return BasicValue value on success, or COLOR_INVALID on error.
*/
BasicValue BasicValue_from_str(const char *arg) {
    char arglower[MAX_COLOR_NAME_LEN];
    str_tolower(arglower, arg);
    for (size_t i=0; i < color_names_len; i++) {
        if (!strcmp(arglower, color_names[i].name)) {
            return color_names[i].color;
        }
    }

    return COLOR_INVALID;
}

/*! Converts an integer string (0-255) into an ExtendedValue suitable
    for the extended-value-based functions.

    \pi arg Color name to find the ExtendedValue for.

    \return A value between 0 and 255 on success.
    \retval COLORVAL_INVALID on error or bad values.
*/
int ExtendedValue_from_str(const char *arg) {
    if (streq(arg, "xred")) return XRED;
    if (streq(arg, "xgreen")) return XGREEN;
    if (streq(arg, "xyellow")) return XYELLOW;
    if (streq(arg, "xblue")) return XBLUE;
    if (streq(arg, "xmagenta")) return XMAGENTA;
    if (streq(arg, "xcyan")) return XCYAN;
    if (streq(arg, "xnormal")) return XWHITE;
    if (streq(arg, "xwhite")) return XWHITE;
    if (streq(arg, "xlightred")) return XLIGHTRED;
    if (streq(arg, "xlightgreen")) return XLIGHTGREEN;
    if (streq(arg, "xlightyellow")) return XLIGHTYELLOW;
    if (streq(arg, "xlightblack")) return XLIGHTBLACK;
    if (streq(arg, "xlightblue")) return XLIGHTBLUE;
    if (streq(arg, "xlightmagenta")) return XLIGHTMAGENTA;
    if (streq(arg, "xlightcyan")) return XLIGHTCYAN;
    if (streq(arg, "xlightnormal")) return XLIGHTWHITE;
    if (streq(arg, "xlightwhite")) return XLIGHTWHITE;

    // Using long to combat easy overflow.
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

/*! Convert an RGB string into separate red, green, blue values.

    \details
    The format for RGB strings can be one of:
        - "RED,GREEN,BLUE"
        - "RED GREEN BLUE"
        - "RED:GREEN:BLUE"

    \pi arg String to check for RGB values.
    \po r   Pointer to an unsigned char for red value on success.
    \po g   Pointer to an unsigned char for green value on success.
    \po b   Pointer to an unsigned char for blue value on success.

    \retval 0 on success, with \p rgbval filled with the values.
    \retval COLORVAL_INVALID for non-rgb strings.
    \retval COLORVAL_INVALID_RANGE for rgb values outside of 0-255.
*/
int rgb_from_str(const char *arg, unsigned char *r, unsigned char *g, unsigned char *b) {
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

/*! Convert an RGB string into a RGB struct suitable for the
    colr*RGB functions.

    \details
    The format for RGB strings can be one of:
        - "RED,GREEN,BLUE"
        - "RED GREEN BLUE"
        - "RED:GREEN:BLUE"

    \details
    Example:
    \code
struct RGB rgbval;
RGB_from_str("123,0,234", &rgbval)
    \endcode

    \pi arg    String to check for RGB values.
    \po rgbval Pointer to an RGB struct to fill in the values for.

    \retval 0 on success, with \p rgbval filled with the values.
    \retval COLORVAL_INVALID for non-rgb strings.
    \retval COLORVAL_INVALID_RANGE for rgb values outside of 0-255.
*/
int RGB_from_str(const char *arg, struct RGB *rgbval) {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    int ret = rgb_from_str(arg, &r, &g, &b);
    if (ret) {
        // An error occurred.
        return ret;
    }
    rgbval->red = r;
    rgbval->green = g;
    rgbval->blue = b;
    return 0;
}

/*! Convert named argument to actual StyleValue enum value.

    \pi arg Style name to convert into a StyleValue.
    \return A usable StyleValue value on success, or STYLE_INVALID on error.
*/
StyleValue StyleValue_from_str(const char *arg) {
    char arglower[MAX_COLOR_NAME_LEN];
    str_tolower(arglower, arg);

    for (size_t i=0; i < style_names_len; i++) {
        if (!strcmp(arglower, style_names[i].name)) {
            return style_names[i].style;
        }
    }
    return STYLE_INVALID;
}

/*! Prepend back color codes to a string, and copy the result into out.
    \details
    The `STYLE_RESET_ALL` code is already appended to the result.

    \po out  Memory allocated for the result.
             _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s    The string to colorize.
             _Must be null-terminated._
    \pi back BasicValue code to use.
*/
void colrbg(char *out, const char *s, BasicValue back) {
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

/*! Colorize a string using true color, rgb back colors and copy the
    result into out.
    \details
    The `StyleValue.RESET_ALL` code is already appended to the result.

    \po out   Allocated memory to copy the result to.
              _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s     String to colorize.
              _Must be null-terminated._
    \pi red   Value for red.
    \pi green Value for green.
    \pi blue  Value for blue.
*/
void colrbgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue) {
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

/*! Colorize a string using true color, rgbval back colors and copy the
    result into out.
    \details
    The `StyleValue.RESET_ALL` code is already appended to the result.

    \po out    Allocated memory to copy the result to.
               _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s      String to colorize.
               _Must be null-terminated._
    \pi rgbval Pointer to an RGB struct to get the r, g, and b values.
*/
void colrbgRGB(char *out, const char *s, struct RGB rgbval) {
    char backcode[CODE_RGB_LEN];
    format_bg_RGB(backcode, rgbval);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, STYLE_RESET_ALL
    );
}

/*! Colorize a string using extended, 256, bg colors, and copy the
    result into out.
    \details
    The `STYLE_RESET_ALL` code is already appended to the result.

    \po out Allocated memory to copy the result to.
            _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s   String to colorize.
            _Must be null-terminated._
    \pi num ExtendedValue to use for the color value.
*/
void colrbgx(char *out, const char *s, unsigned char num) {
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

/*! Prepend fore color codes to a string, and copy the result into out.
    \details
    The STYLE_RESET_ALL code is already appended to the result.

    \po out  Memory allocated for the result.
             _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s    The string to colorize.
             _Must be null-terminated._
    \pi fore BasicValue code to use.
*/
void colrfg(char *out, const char *s, BasicValue fore) {
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

/*! Build a colorized string, from a single character.

    \po out  Memory allocated for the resulting string.
             _Must have enough room for `COLOR_LEN + 1`._
    \pi c    Character to colorize.
    \pi fore Fore color from BasicValue to use.
*/
void colrfgchar(char *out, const char c, BasicValue fore) {
    char s[2] = "\0\0";
    s[0] = c;
    colrfg(out, s, fore);
}

/*! Rainbow-ize some text using rgb fore colors, lolcat style.
    \details
    The `STYLE_RESET_ALL` code is already appended to the result.

    \po out    Memory allocated for the result.
               _Must have enough room for `strlen(s) + (CODE_RGB_LEN * strlen(s))`._
    \pi s      The string to colorize.
               _Must be null-terminated._
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
*/
void colrfgrainbow(char *out, const char *s, double freq, size_t offset) {
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

/*! Like colrfgrainbow, except it allocates the string for you, with
    enough room to fit the string and any escape codes needed.

    \pi s      The string to colorize.
               _Must be null-terminated._
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.
*/
char * acolrfgrainbow(const char *s, double freq, size_t offset) {
    size_t oldlen = strlen(s);
    char *out = calloc(oldlen + (CODE_RGB_LEN * oldlen), sizeof(char));
    colrfgrainbow(out, s, freq, offset);
    return out;
}

/*! Colorize a string using true color, rgb fore colors and copy the
    result into out.
    \details
    The `StyleValue.RESET_ALL` code is already appended to the result.

    \po out   Allocated memory to copy the result to.
              _Must have enough room for `strlen(s) + COLOR_RGB_LEN`._
    \pi s     String to colorize.
              _Must be null-terminated._
    \pi red   Value for red.
    \pi green Value for green.
    \pi blue  Value for blue.
*/
void colrfgrgb(char *out, const char *s, unsigned char red, unsigned char green, unsigned char blue) {
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

/*! Colorize a string using true color, rgb fore colors, and copy the
    result into `out`.
    \details
    The StyleValue.RESET_ALL code is already appended to the result.

    \po out    Allocated memory to copy the result to.
               _Must have enough room for `strlen(s) + COLOR_RGB_LEN`.
    \pi s      The string to colorize.
               _Must be null-terminated._
    \pi rgbval RGB struct to use for the r, g, and b values.
*/
void colrfgRGB(char *out, const char *s, struct RGB rgbval) {
    char forecode[CODE_RGB_LEN];
    format_fg_RGB(forecode, rgbval);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, STYLE_RESET_ALL
    );
}

/*!
    \brief Colorize a string using extended, 256 colors, and copy the result into `out`.
    \details
    The `StyleValue.RESET_ALL` code is already appended to the result.

    \po out Allocated memory to copy the result to.
            _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s   String to colorize.
            _Must be null-terminated._
    \pi num Code number, 0-255.
*/
void colrfgx(char *out, const char *s, unsigned char num) {
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


/*! Prepends escape codes for fore, back, and style to s and copies
    the result into out.

    \po out  Allocated memory to copy the result to.
             _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s    String to colorize.
             _Must be null-terminated._
    \pi fore BasicValue value to use for fore.
    \pi back BasicValue value to use for background.
    \pi style StyleValue value to use.
*/
void colrize(char *out, const char *s, BasicValue fore, BasicValue back, StyleValue style) {

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

/*! Prepends escape codes for fore, back, and style to a character (c)
    and copies the result into out.

    \po out   Allocated memory to copy the result to.
               _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi c     Character to colorize.
    \pi fore  BasicValue value to use for fore.
    \pi back  BasicValue value to use for background.
    \pi style StyleValue value to use.
*/
void colrizechar(char *out, char c, BasicValue fore, BasicValue back, StyleValue style) {
    char s[2] = "\0\0";
    s[0] = c;
    colrize(out, s, fore, back, style);
}

/*! Prepends escape codes for extended fore, back, and style to s and
    copies the result into an allocated string.

    \pout out   Allocated memory to copy the result to.
                _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi  s     String to colorize.
                _Must be null-terminated._
    \pi  fore  ExtendedValue to use for fore.
    \pi  back  ExtendedValue to use for background.
    \pi  style StyleValue value to use.
*/
void colrizex(
    char *out,
    const char *s,
    ExtendedValue fore, ExtendedValue back, StyleValue style) {
    // Build forecolor only.
    char forecode[CODEX_LEN];
    format_fgx(forecode, fore);
    // Build backcolor only.
    char backcode[CODEX_LEN];
    format_bgx(backcode, back);
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

/*! Prepend style codes to a string, and copy the result into out.
    \details
    The STYLE_RESET_ALL code is already appended to the result.

    \po out   Memory allocated for the result.
              _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s     The string to colorize.
              _Must be null-terminated._
    \pi style StyleValue code to use.
*/
void colrstyle(char *out, const char *s, StyleValue style) {
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

