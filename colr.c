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

/*! Allocates an empty string.
    \details
    This is for keeping the interface simple, so the return values from
    color functions with invalid values can be consistent.

    \return Pointer to an allocated string consisting of '\0'.
*/
char *colr_empty_str(void) {
    char *s = malloc(sizeof(char));
    if (!s) return NULL;
    s[0] = '\0';
    return s;
}

/*! Create an escape code for an extended background color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for background.
*/
void format_bgx(char *out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\033[48;5;%dm", num);
}

/*! Create an escape code for a background color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for background.
*/
void format_bg(char *out, BasicValue value) {
    if (!out) return;
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
    \pi redval   Value for red.
    \pi greenval Value for green.
    \pi blueval  Value for blue.
*/
void format_bg_rgb(char *out, unsigned char redval, unsigned char greenval, unsigned char blueval) {
    if (!out) return;
    snprintf(out, CODE_RGB_LEN, "\033[48;2;%d;%d;%dm", redval, greenval, blueval);
}

/*! Create an escape code for a true color (rgb) background color
    using values from an RGB struct.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODE_RGB_LEN`._
    \pi rgb RGB struct to get red, blue, and green values from.
*/
void format_bg_RGB(char *out, struct RGB rgb) {
    if (!out) return;
    format_bg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for an extended fore color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for fore.
*/
void format_fgx(char *out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\033[38;5;%dm", num);
}

/*! Create an escape code for a fore color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for fore.
*/
void format_fg(char *out, BasicValue value) {
    if (!out) return;
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

    \po out      Memory allocated for the escape code string.
                 _Must have enough room for `CODE_RGB_LEN`._
    \pi redval   Value for red.
    \pi greenval Value for green.
    \pi blueval  Value for blue.
*/
void format_fg_rgb(char *out, unsigned char redval, unsigned char greenval, unsigned char blueval) {
    if (!out) return;
    snprintf(out, CODE_RGB_LEN, "\033[38;2;%d;%d;%dm", redval, greenval, blueval);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB(char *out, struct RGB rgb) {
    if (!out) return;
    format_fg_rgb(out, rgb.red, rgb.green, rgb.blue);
}

/*! A single step in rainbow-izing a string.

    \po out  Memory allocated for the escape code string.
    \pi freq Frequency ("tightness") of the colors.
    \pi step Offset from the start of the rainbow.
             Usually an index into a string.
*/
void format_rainbow_fore(char *out, double freq, size_t step) {
    if (!out) return;
    double redval = sin(freq * step + 0) * 127 + 128;
    double greenval = sin(freq * step + 2 * M_PI / 3) * 127 + 128;
    double blueval = sin(freq * step + 4 * M_PI / 3) * 127 + 128;
    format_fg_rgb(
        out,
        (unsigned char)redval,
        (unsigned char)greenval,
        (unsigned char)blueval
    );
}

/*! Create an escape code for a style.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `STYLE_LEN`._
    \pi style StyleValue value to use for style.
*/
void format_style(char *out, StyleValue style) {
    if (!out) return;
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

/*! Joins ColorArgs, ColorTexts, and strings into one long string.

    \details
    This will free() any ColorArgs and ColorTexts that are passed in. It is
    backing the colr() macro, and enables easy throwaway color values.

    \details
    Any plain strings that are passed in are left alone. It is up to the caller
    to free those. Colr only manages the temporary Colr-based objects needed
    to build up these strings.

    \pi p   The first of any ColorArg, ColorText, or strings to join.
    \pi ... Zero or more ColorArg, ColorText, or string to join.
    \return An allocated string with mixed escape codes/strings.
            CODE_RESET_ALL is appended to all the pieces that aren't plain
            strings. This allows easy part-colored messages, so there's no
            need to use CODE_RESET_ALL directly.
            __You must free() the memory allocated by this function.__
*/
char *Colr_join(void *p, ...) {
    // Argument list must have ColorArg/ColorText with NULL members at the end.
    va_list args;
    va_start(args, p);
    char *s;
    struct ColorArg *cargp = NULL;
    struct ColorText *ctextp = NULL;
    if (ColorArg_is_ptr(p)) {
        // It's a ColorArg.
        cargp = p;
        s = ColorArg_to_str(*cargp);
        ColorArg_free(cargp);
    } else if (ColorText_is_ptr(p)) {
        ctextp = p;
        s = ColorText_to_str(*ctextp);
        ColorText_free(ctextp);
    } else {
        // It's a string, or it better be anyway.
        s = (char *)p;
    }
    size_t length = strlen(s) + 1;
    length += CODE_RESET_LEN;
    // Allocate enough for the reset code at the end.
    char *final = calloc(length, sizeof(char));
    sprintf(final, "%s", s);
    if (cargp) {
        free(s);
    }
    if (ctextp) {
        free(s);
    }

    void *arg = NULL;
    while ((arg = va_arg(args, void*))) {
        cargp = NULL;
        ctextp = NULL;
        // These ColorArgs/ColorTexts were heap allocated through the fore,
        // back, style, and Colr macros. I'm going to free them, so the user
        // doesn't have to keep track of all the temporary pieces that built
        // this string.
        if (ColorArg_is_ptr(arg)) {
            // It's a ColorArg.
            cargp = arg;
            s = ColorArg_to_str(*cargp);
            ColorArg_free(cargp);
        } else if (ColorText_is_ptr(arg)) {
            ctextp = arg;
            s = ColorText_to_str(*ctextp);
            ColorText_free(ctextp);
        } else {
            // It better be a string.
            s = (char *)arg;
        }
        // TODO: Not allocating enough, getting invalid writes.
        length += strlen(s) + 1;
        final = realloc(final, length);
        sprintf(final, "%s%s", final, s);
        // Free the temporary string from those ColorArgs/ColorTexts.
        if (cargp || ctextp) free(s);
    }
    sprintf(final, "%s%s", final, CODE_RESET_ALL);
    debug("FINAL LENGTH: %lu, ALLOCATED LENGTH: %lu\n", strlen(final), length);
    va_end(args);
    return final;
}

/*! Creates a string representation of a ArgType.

    \pi type A ArgType to get the type from.
    \return  A pointer to an allocated string. You must free() it.
*/
char *ArgType_repr(ArgType type) {
    char *typestr;
    switch (type) {
        case ARGTYPE_NONE:
            asprintf(&typestr, "ARGTYPE_NONE");
            break;
        case FORE:
            asprintf(&typestr, "FORE");
            break;
        case BACK:
            asprintf(&typestr, "BACK");
            break;
        case STYLE:
            asprintf(&typestr, "STYLE");
            break;
    }
    return typestr;
}

/*! Free allocated memory for a ColorArg.

    \details
    This has no advantage over `free(colorarg)` right now, it is used in
    debugging, and may be extended in the future. It's better just to use it.

    \pi p ColorArg to free.
*/
void ColorArg_free(struct ColorArg *p) {
    free(p);
}

/*! Explicit version of ColorArg_from_value that only handles BasicValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value BasicValue to use.

    \return A ColorArg, with the `.value.type` member possible set to `TYPE_INVALID`.
*/
struct ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    BasicValue val = value;
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_EXTENDED, &val),
    };
}

/*! Explicit version of ColorArg_from_value that only handles ExtendedValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value ExtendedValue to use.

    \return A ColorArg, with the `.value.type` member possible set to `TYPE_INVALID`.
*/
struct ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    ExtendedValue val = value;
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_EXTENDED, &val),
    };
}

/*! Explicit version of ColorArg_from_value that only handles RGB structs.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value RGB struct to use.

    \return A ColorArg, with the `.value.type` member possible set to `TYPE_INVALID`.
*/
struct ColorArg ColorArg_from_RGB(ArgType type, struct RGB value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    struct RGB val = value;
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_RGB, &val),
    };
}

/*! Build a ColorArg (fore, back, or style value) from a known color name/style.

    \details
    The `.value.type` attribute can be checked for an invalid type, or you
    can call ColorArg_is_invalid(x).

    \pi type      ArgType (FORE, BACK, STYLE).
    \pi colorname A known color name/style.

    \return A ColorArg struct with usable values.
*/
struct ColorArg ColorArg_from_str(ArgType type, char *colorname) {
    struct ColorValue carg = ColorValue_from_str(colorname);
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=carg
    };

}

/*! Explicit version of ColorArg_from_value that only handles StyleValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value StyleValue to use.

    \return A ColorArg, with the `.value.type` member possible set to `TYPE_INVALID`.
*/
struct ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    StyleValue val = value;
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_STYLE, &val),
    };
}

/*! Used with the color_arg macro to dynamically create a ColorArg based
    on it's argument type.

    \pi type     ArgType value, to mark the type of ColorArg.
    \pi colrtype ColorType value, to mark the type of ColorValue.
    \pi p        A pointer to either a BasicValue, ExtendedValue, or a struct RGB.

    \return A ColorArg struct with the appropriate `.value.type` member set for
            the value that was passed. For invalid types the `.value.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXTENDED_RANGE
        - TYPE_INVALID_RGB_RANGE
*/
struct ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void *p) {
    if (!p) {
        return (struct ColorArg){
            .marker=COLORARG_MARKER,
            .type=ARGTYPE_NONE,
            .value=ColorValue_from_value(TYPE_INVALID, NULL)
        };
    }
    return (struct ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(colrtype, p),
    };
}


/*! Checks to see if a ColorArg holds an invalid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is invalid, otherwise `false`.
*/
bool ColorArg_is_invalid(struct ColorArg carg) {
    return !(bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type));
}

/*! Checks a void pointer to see if it contains a ColorArg struct.

    \details The first member of a ColorArg is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorArg, otherwise `false`.
*/
bool ColorArg_is_ptr(void *p) {
    if (!p) return false;
    // The head of a ColorArg is always a valid marker.
    struct ColorArg *cargp = p;
    return cargp->marker == COLORARG_MARKER;
}

/*! Checks to see if a ColorArg holds a valid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is valid, otherwise `false`.
*/
bool ColorArg_is_valid(struct ColorArg carg) {
    return bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type);
}



/*! Creates a string representation for a ColorArg.
    \details
    Allocates memory for the string representation.

    \pi carg ColorArg struct to get the representation for.
    \return Allocated string for the representation.
            You must free() it.
*/
char *ColorArg_repr(struct ColorArg carg) {
    char *type = ArgType_repr(carg.type);
    char *value = ColorValue_repr(carg.value);
    char *repr;
    asprintf(&repr, "struct ColorArg {.type=%s, .value=%s}", type, value);
    free(type);
    free(value);
    return repr;
}

/*! Copies a ColorArg into memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi carg ColorArg to copy/allocate for.
    \return Pointer to a heap-allocated ColorArg.
*/
struct ColorArg *ColorArg_to_ptr(struct ColorArg carg) {
    struct ColorArg *p = malloc(sizeof(carg));
    *p = carg;
    return p;
}

/*! Converts a ColorArg into an escape code string.
    \details
    Allocates memory for the string.

    \details
    If the ColorValue is invalid, an empty string is returned.
    You must still free the empty string.

    \pi carg ColorArg to get the ArgType and ColorValue from.
    \return Allocated string for the escape code.
            You must free() it.
*/
char *ColorArg_to_str(struct ColorArg carg) {
    return ColorValue_to_str(carg.type, carg.value);
}


/*! Frees a ColorText and it's ColorArgs.

    \details
    The text member is left alone, because it wasn't created by Colr.

    \pi p Pointer to ColorText to free, along with it's Colr-based members.
*/
void ColorText_free(struct ColorText *p) {
    if (!p) return;
    if (p->fore) free(p->fore);
    if (p->back) free(p->back);
    if (p->style) free(p->style);

    free(p);
}

/*! Builds a ColorText from 1 mandatory string, and optional fore, back, and
    style args (pointers to ColorArgs).
    \pi text Text to colorize (a regular string).
    \pi ... ColorArgs for fore, back, and style, in any order.
    \return An initialized ColorText struct.
*/
struct ColorText ColorText_from_values(char *text, ...) {
    // Argument list must have ColorArg with NULL members at the end.
    struct ColorText ctext = {
        .marker=COLORTEXT_MARKER,
        .text=text,
        .fore=NULL,
        .back=NULL,
        .style=NULL
    };
    va_list colrargs;
    va_start(colrargs, text);
    struct ColorArg *arg;
    while ((arg = va_arg(colrargs, struct ColorArg*))) {
        // It's a ColorArg.
        if (arg->type == FORE) {
            // It's the fore arg.
            ctext.fore = arg;
        } else if (arg->type == BACK) {
            ctext.back = arg;
        } else if (arg->type == STYLE) {
            ctext.style = arg;
            break;
        }
    }
    va_end(colrargs);
    return ctext;
}
/*! Checks a void pointer to see if it contains a ColorText struct.

    \details The first member of a ColorText is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorText, otherwise `false`.
*/
bool ColorText_is_ptr(void *p) {
    if (!p) return false;
    // The head of a ColorText is always a valid marker.
    struct ColorText *ctextp = p;
    return ctextp->marker == COLORTEXT_MARKER;
}

/*! Allocate a string representation for a ColorText.

    \pi ctext ColorText to get the string representation for.
    \return Allocated string for the ColorText.
*/
char *ColorText_repr(struct ColorText ctext) {
    char *s;
    char *sfore = ctext.fore ? ColorArg_repr(*(ctext.fore)) : NULL;
    char *sback = ctext.back ? ColorArg_repr(*(ctext.back)) : NULL;
    char *sstyle = ctext.style ? ColorArg_repr(*(ctext.style)) : NULL;

    asprintf(
        &s,
        "struct ColorText {.text=%s, .fore=%s, .back=%s, .style=%s}\n",
        ctext.text ? ctext.text : "NULL",
        sfore ? sfore : "NULL",
        sback ? sback : "NULL",
        sstyle ? sstyle : "NULL"
    );
    free(sfore);
    free(sback);
    free(sstyle);
    return s;
}
/*! Copies a ColorText into memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi ctext ColorText to copy/allocate for.
    \return Pointer to a heap-allocated ColorText.
*/
struct ColorText *ColorText_to_ptr(struct ColorText ctext) {
    size_t length = sizeof(struct ColorText);
    if (ctext.text) length += strlen(ctext.text) + 1;
    struct ColorText *p = malloc(length);
    *p = ctext;
    if (!(p->marker)) p->marker = COLORTEXT_MARKER;
    return p;
}

/*! Stringifies a ColorText struct.

    \details
    You must free() the resulting string.
    \pi ctext ColorText to stringify.
    \return An allocated string. _You must `free()` it_.
*/
char *ColorText_to_str(struct ColorText ctext) {
    if (!ctext.text) return colr_empty_str();
    size_t length = strlen(ctext.text) + 1;
    // Make room for any fore/back/style code combo plus the reset_all code.
    length += CODE_ANY_LEN + STYLE_LEN;
    char *final = calloc(length, sizeof(char));
    if (ctext.style) {
        char *stylecode = ColorArg_to_str(*(ctext.style));
        sprintf(final, "%s%s", final, stylecode);
        free(stylecode);
    }
    if (ctext.fore) {
        char *forecode = ColorArg_to_str(*(ctext.fore));
        sprintf(final, "%s%s", final, forecode);
        free(forecode);
    }
    if (ctext.back) {
        char *backcode = ColorArg_to_str(*(ctext.back));
        sprintf(final, "%s%s", final, backcode);
        free(backcode);
    }
    sprintf(final, "%s%s%s", final, ctext.text, CODE_RESET_ALL);
    return final;
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
    // Try styles.
    if (StyleValue_from_str(arg) != STYLE_INVALID) {
        return TYPE_STYLE;
    }
    return TYPE_INVALID;
}

/*! Check to see if a ColorType value is considered invalid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered invalid, otherwise `false`.
*/
bool ColorType_is_invalid(ColorType type) {
    return !(bool_colr_enum(type));
}

/*! Check to see if a ColorType value is considered valid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered valid, otherwise `false`.
*/
bool ColorType_is_valid(ColorType type) {
    return bool_colr_enum(type);
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
        case TYPE_STYLE:
            asprintf(&typestr, "TYPE_STYLE");
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

/*! Create a ColorValue from a known color name, or RGB string.

    \pi s    A string to parse the color name from (can be an RGB string).
    \return  A ColorValue (with no fore/back information, only the color type and value).
*/
struct ColorValue ColorValue_from_str(char *s) {
    if (!s) {
        return ColorValue_from_value(TYPE_INVALID, NULL);
    }
    // Get the actual type, even if it's invalid.
    ColorType type = ColorType_from_str(s);
    if (ColorType_is_invalid(type)) {
        return ColorValue_from_value(type, NULL);
    }
    // Try rgb first.
    struct RGB rgb;
    int rgb_ret = RGB_from_str(s, &rgb);
    if (rgb_ret == COLORVAL_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_RGB_RANGE, NULL);
    } else if (rgb_ret != TYPE_INVALID) {
        return ColorValue_from_value(type, &rgb);
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(s);
    if (x_ret == COLORVAL_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_EXTENDED_RANGE, NULL);
    } else if (x_ret != COLORVAL_INVALID) {
        // Need to cast back into a real ExtendedValue now that I know it's
        // not invalid. Also, ColorValue_from_value expects a pointer, to
        // help with it's "dynamic" uses.
        ExtendedValue xval = (ExtendedValue)x_ret;
        return ColorValue_from_value(type, &xval);
    }
    // Try styles.
    int s_ret = StyleValue_from_str(s);
    if (s_ret != STYLE_INVALID) {
        StyleValue sval = (StyleValue)s_ret;
        return ColorValue_from_value(type, &sval);
    }

    // Try basic colors.
    int b_ret = BasicValue_from_str(s);
    if ( b_ret != COLOR_INVALID) {
        BasicValue bval = (BasicValue)b_ret;
        return ColorValue_from_value(type, &bval);
    }
    return ColorValue_from_value(TYPE_INVALID, NULL);
}

/*! Used with the color_val macro to dynamically create a ColorValue based
    on it's argument type.

    \pi type A ColorType value, to mark the type of ColorValue.
    \pi p    A pointer to either a BasicValue, ExtendedValue, or a struct RGB.

    \return A ColorValue struct with the appropriate `.type` member set for
            the value that was passed. For invalid types the `.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXTENDED_RANGE
        - TYPE_INVALID_RGB_RANGE
*/
struct ColorValue ColorValue_from_value(ColorType type, void *p) {
    if (!p) {
        return (struct ColorValue){.type=TYPE_INVALID};
    }
    if (
        type == TYPE_INVALID ||
        type == TYPE_INVALID_EXTENDED_RANGE ||
        type == TYPE_INVALID_RGB_RANGE
        ) {
        return (struct ColorValue){.type=type};
    }
    if (!p) {
        return (struct ColorValue){.type=TYPE_INVALID};
    }
    if (type == TYPE_BASIC) {
        BasicValue *bval = p;
        return (struct ColorValue){.type=TYPE_BASIC, .basic=*bval};
    } else if (type == TYPE_EXTENDED) {
        ExtendedValue *eval = p;
        return (struct ColorValue){.type=TYPE_EXTENDED, .ext=*eval};
    } else if (type == TYPE_STYLE) {
        StyleValue *sval = p;
        return (struct ColorValue){.type=TYPE_STYLE, .style=*sval};
    } else if (type == TYPE_RGB) {
        struct RGB *rgbval = p;
        return (struct ColorValue){.type=TYPE_RGB, .rgb=*rgbval};
    }
    return (struct ColorValue){.type=type};
}

/*! Checks to see if a ColorValue holds an invalid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is invalid, otherwise `false`.
*/
bool ColorValue_is_invalid(struct ColorValue cval) {
    return !(bool_colr_enum(cval.type));
}

/*! Checks to see if a ColorValue holds a valid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is valid, otherwise `false`.
*/
bool ColorValue_is_valid(struct ColorValue cval) {
    return bool_colr_enum(cval.type);
}

/*! Creates a string representation of a ColorValue.

    \pi cval    A ColorValue to get the type and value from.
    \return     A pointer to an allocated string. You must free() it.
*/
char *ColorValue_repr(struct ColorValue cval) {
    char *argstr;
    switch (cval.type) {
        case TYPE_RGB:
            asprintf(
                &argstr,
                "struct RGB {.red=%d, .green=%d, .blue=%d}",
                cval.rgb.red,
                cval.rgb.green,
                cval.rgb.blue
            );
            break;
        case TYPE_BASIC:
            asprintf(&argstr, "(BasicValue) %d", cval.basic);
            break;
        case TYPE_EXTENDED:
            asprintf(&argstr, "(ExtendedValue) %d", cval.ext);
            break;
        case TYPE_STYLE:
            asprintf(&argstr, "(StyleValue) %d", cval.style);
            break;
        default:
            return ColorType_repr(cval.type);
    }
    return argstr;
}

/*! Converts a ColorValue into an escape code string.

    \details
    Memory is allocated for the string.
    You must free() it.

    \pi type ArgType (FORE, BACK, STYLE) to build the escape code for.
    \pi cval ColorValue to get the color value from.

    \return  An allocated string with the appropriate escape code.
             For invalid values, an empty string is returned.
*/
char *ColorValue_to_str(ArgType type, struct ColorValue cval) {
    char *codes;
    switch (type) {
        case FORE:
            switch (cval.type) {
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_fg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_fgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_fg_RGB(codes, cval.rgb);
                    return codes;
                // This case is not valid, but I will try to do the right thing.
                case TYPE_STYLE:
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                default:
                    return colr_empty_str();
                }
        case BACK:
            switch (cval.type) {
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_bg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_bgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_bg_RGB(codes, cval.rgb);
                    return codes;
                // This case is not even valid, but okay.
                case TYPE_STYLE:
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                default:
                    return colr_empty_str();
                }
        case STYLE:
            switch (cval.type) {
                case TYPE_STYLE:
                    // This is the only appropriate case.
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                // All of these other cases are a product of mismatched info.
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_fg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_fgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_fg_RGB(codes, cval.rgb);
                    return codes;
                default:
                    return colr_empty_str();
            }
        default:
            return colr_empty_str();
    }
    return colr_empty_str();
}
/*! Convert named argument to an actual BasicValue enum value.

    \pi arg Color name to find the BasicValue for.
    \return BasicValue value on success, or COLOR_INVALID on error.
*/
BasicValue BasicValue_from_str(const char *arg) {
    if (!arg) {
        return COLOR_INVALID;
    }
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
    if (!arg) {
        return COLORVAL_INVALID;
    }
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
    if (!arg) {
        return COLORVAL_INVALID;
    }
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
    if (!arg) {
        return COLORVAL_INVALID;
    }
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
    if (!arg) {
        return STYLE_INVALID;
    }
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
    The `CODE_RESET_ALL` code is already appended to the result.

    \po out  Memory allocated for the result.
             _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s    The string to colorize.
             _Must be null-terminated._
    \pi back BasicValue code to use.
*/
void colrbg(char *out, const char *s, BasicValue back) {
    if (!(out && s)) {
        return;
    }
    char backcode[CODEX_LEN];
    format_bg(backcode, back);
    size_t oldlen = strlen(s);
    size_t codeslen = CODEX_LEN + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s", backcode, s, CODE_RESET_ALL
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
    if (!(out && s)) {
        return;
    }
    char backcode[CODE_RGB_LEN];
    format_bg_rgb(backcode, red, green, blue);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, CODE_RESET_ALL
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
    if (!(out && s)) {
        return;
    }
    char backcode[CODE_RGB_LEN];
    format_bg_RGB(backcode, rgbval);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, CODE_RESET_ALL
    );
}

/*! Colorize a string using extended, 256, bg colors, and copy the
    result into out.
    \details
    The `CODE_RESET_ALL` code is already appended to the result.

    \po out Allocated memory to copy the result to.
            _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s   String to colorize.
            _Must be null-terminated._
    \pi num ExtendedValue to use for the color value.
*/
void colrbgx(char *out, const char *s, unsigned char num) {
    if (!(out && s)) {
        return;
    }
    char backcode[CODEX_LEN];
    format_bgx(backcode, num);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(backcode) + STYLE_LEN;

    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        backcode, s, CODE_RESET_ALL
    );
}

/*! Prepend fore color codes to a string, and copy the result into out.
    \details
    The CODE_RESET_ALL code is already appended to the result.

    \po out  Memory allocated for the result.
             _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s    The string to colorize.
             _Must be null-terminated._
    \pi fore BasicValue code to use.
*/
void colrfg(char *out, const char *s, BasicValue fore) {
    if (!(out && s)) {
        return;
    }
    char forecode[CODEX_LEN];
    format_fg(forecode, fore);
    size_t oldlen = strlen(s);
    size_t codeslen = CODEX_LEN + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s", forecode, s, CODE_RESET_ALL
    );
}

/*! Build a colorized string, from a single character.

    \po out  Memory allocated for the resulting string.
             _Must have enough room for `COLOR_LEN + 1`._
    \pi c    Character to colorize.
    \pi fore Fore color from BasicValue to use.
*/
void colrfgchar(char *out, const char c, BasicValue fore) {
    if (!out) {
        return;
    }
    char s[2] = "\0\0";
    s[0] = c;
    colrfg(out, s, fore);
}

/*! Rainbow-ize some text using rgb fore colors, lolcat style.
    \details
    The `CODE_RESET_ALL` code is already appended to the result.

    \po out    Memory allocated for the result.
               _Must have enough room for `strlen(s) + (CODE_RGB_LEN * strlen(s))`._
    \pi s      The string to colorize.
               _Must be null-terminated._
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
*/
void colrfgrainbow(char *out, const char *s, double freq, size_t offset) {
    if (!(out && s)) {
        return;
    }
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
    strncat(out, CODE_RESET_ALL, STYLE_LEN);
}

/*! Like colrfgrainbow, except it allocates the string for you, with
    enough room to fit the string and any escape codes needed.

    \pi s      The string to colorize.
               _Must be null-terminated._
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.
*/
char *acolrfgrainbow(const char *s, double freq, size_t offset) {
    if (!s) {
        return NULL;
    }
    size_t oldlen = strlen(s);
    char *out = calloc(oldlen + (CODE_RGB_LEN * oldlen), sizeof(char));
    colrfgrainbow(out, s, freq, offset);
    return out;
}

/*! Colorize a string using true color, rgb fore colors and copy the
    result into out.
    \details
    The `StyleValue.RESET_ALL` code is already appended to the result.

    \po out      Allocated memory to copy the result to.
                 _Must have enough room for `strlen(s) + COLOR_RGB_LEN`._
    \pi s        String to colorize.
                 _Must be null-terminated._
    \pi redval   Value for red.
    \pi greenval Value for green.
    \pi blueval  Value for blue.
*/
void colrfgrgb(char *out, const char *s, unsigned char redval, unsigned char greenval, unsigned char blueval) {
    if (!(out && s)) {
        return;
    }
    char forecode[CODE_RGB_LEN];
    format_fg_rgb(forecode, redval, greenval, blueval);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, CODE_RESET_ALL
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
    if (!(out && s)) {
        return;
    }
    char forecode[CODE_RGB_LEN];
    format_fg_RGB(forecode, rgbval);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, CODE_RESET_ALL
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
    if (!(out && s)) {
        return;
    }
    char forecode[CODEX_LEN];
    format_fgx(forecode, num);
    size_t oldlen = strlen(s);
    size_t codeslen = strlen(forecode) + STYLE_LEN;

    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        forecode, s, CODE_RESET_ALL
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
    if (!(out && s)) {
        return;
    }

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
        frontcodes, endcodes, s, CODE_RESET_ALL
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
    if (!out) {
        return;
    }
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
    if (!(out && s)) {
        return;
    }
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
        frontcodes, endcodes, s, CODE_RESET_ALL
    );
}

/*! Prepend style codes to a string, and copy the result into out.
    \details
    The CODE_RESET_ALL code is already appended to the result.

    \po out   Memory allocated for the result.
              _Must have enough room for `strlen(s) + COLOR_LEN`._
    \pi s     The string to colorize.
              _Must be null-terminated._
    \pi style StyleValue code to use.
*/
void colrstyle(char *out, const char *s, StyleValue style) {
    if (!(out && s)) {
        return;
    }
    if (style < 0) style = RESET_ALL;
    char stylecode[STYLE_LEN];
    snprintf(stylecode, STYLE_LEN, "\033[%dm", style);

    size_t oldlen = strlen(s);
    size_t codeslen = strlen(stylecode) + STYLE_LEN;
    snprintf(
        out,
        oldlen + codeslen,
        "%s%s%s",
        stylecode, s, CODE_RESET_ALL
    );
}

