/*! \file colr.c
    Implements everything in the colr.h header.

    \internal
    \author Christopher Welborn
    \date 06-22-2019
    \endinternal
*/
#include "colr.h"

//! A list of BasicInfo items, used with BasicValue_from_str().
const BasicInfo basic_names[] = {
    {"none", RESET},
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
};

//! Length of basic_names.
const size_t basic_names_len = sizeof(basic_names) / sizeof(basic_names[0]);


//! A list of ExtendedInfo, used with ExtendedValue_from_str().
const ExtendedInfo extended_names[] = {
    {"xred", XRED},
    {"xgreen", XGREEN},
    {"xyellow", XYELLOW},
    {"xblue", XBLUE},
    {"xmagenta", XMAGENTA},
    {"xcyan", XCYAN},
    {"xnormal", XWHITE},
    {"xwhite", XWHITE},
    {"xlightred", XLIGHTRED},
    {"xlightgreen", XLIGHTGREEN},
    {"xlightyellow", XLIGHTYELLOW},
    {"xlightblack", XLIGHTBLACK},
    {"xlightblue", XLIGHTBLUE},
    {"xlightmagenta", XLIGHTMAGENTA},
    {"xlightwhite", XLIGHTWHITE},
    {"xlightcyan", XLIGHTCYAN},
    {"xlightnormal", XLIGHTWHITE},
};

//! Length of extended_names.
const size_t extended_names_len = sizeof(extended_names) / sizeof(extended_names[0]);

//! A list of StyleInfo items, used with StyleName_from_str().
const StyleInfo style_names[] = {
    {"none", RESET_ALL},
    {"reset", RESET_ALL},
    {"reset-all", RESET_ALL},
    {"reset_all", RESET_ALL},
    {"reset all", RESET_ALL},
    {"bold", BRIGHT},
    {"bright", BRIGHT},
    {"dim", DIM},
    {"italic", ITALIC},
    {"underline", UNDERLINE},
    {"flash", FLASH},
    {"highlight", HIGHLIGHT},
    {"normal", NORMAL},
    {"strikethru", STRIKETHRU},
    {"strike", STRIKETHRU},
    {"strikethrough", STRIKETHRU},
    {"frame", FRAME},
    {"encircle", ENCIRCLE},
    {"circle", ENCIRCLE},
    {"overline", OVERLINE},
};

//! Length of style_names.
const size_t style_names_len = sizeof(style_names) / sizeof(style_names[0]);

/*! A map from ExtendedValue (256-color) to RGB value, where the index is the
    is the ExtendedValue, and the value is the RGB.

    \details
    This is used in several RGB/ExtendedValue functions.

    \sa ExtendedValue_from_RGB RGB_to_term_RGB

    \examplecodefor{ext2rgb_map,.c}
    // Fast map an ExtendedValue to an RGB value.
    ExtendedValue eval = 9; // 9 happens to be XRED (255;0;0 in RGB).
    RGB rgbval = ext2rgb_map[eval];

    // The result from ExtendedValue_from_RGB should always match the map.
    assert(ExtendedValue_from_RGB(rgbval) == eval);

    // Also, the result of RGB_to_term_RGB() should always be in the map.
    assert(RGB_eq(ext2rgb_map[eval], RGB_to_term_RGB(rgb(255, 47, 23))));
    \endexamplecode
*/
const RGB ext2rgb_map[] = {
    // Primary 3-bit colors (8 colors, 0-7)
    {0, 0, 0},
    {128, 0, 0},
    {0, 128, 0},
    {128, 128, 0},
    {0, 0, 128},
    {128, 0, 128},
    {0, 128, 128},
    {192, 192, 192},
    // "Bright" versions of the original 8 colors (8-15).
    {128, 128, 128},
    {255, 0, 0},
    {0, 255, 0},
    {255, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 255},
    // Strictly ascending.
    {0, 0, 0},
    {0, 0, 95},
    {0, 0, 135},
    {0, 0, 175},
    {0, 0, 215},
    {0, 0, 255},
    {0, 95, 0},
    {0, 95, 95},
    {0, 95, 135},
    {0, 95, 175},
    {0, 95, 215},
    {0, 95, 255},
    {0, 135, 0},
    {0, 135, 95},
    {0, 135, 135},
    {0, 135, 175},
    {0, 135, 215},
    {0, 135, 255},
    {0, 175, 0},
    {0, 175, 95},
    {0, 175, 135},
    {0, 175, 175},
    {0, 175, 215},
    {0, 175, 255},
    {0, 215, 0},
    {0, 215, 95},
    {0, 215, 135},
    {0, 215, 175},
    {0, 215, 215},
    {0, 215, 255},
    {0, 255, 0},
    {0, 255, 95},
    {0, 255, 135},
    {0, 255, 175},
    {0, 255, 215},
    {0, 255, 255},
    {95, 0, 0},
    {95, 0, 95},
    {95, 0, 135},
    {95, 0, 175},
    {95, 0, 215},
    {95, 0, 255},
    {95, 95, 0},
    {95, 95, 95},
    {95, 95, 135},
    {95, 95, 175},
    {95, 95, 215},
    {95, 95, 255},
    {95, 135, 0},
    {95, 135, 95},
    {95, 135, 135},
    {95, 135, 175},
    {95, 135, 215},
    {95, 135, 255},
    {95, 175, 0},
    {95, 175, 95},
    {95, 175, 135},
    {95, 175, 175},
    {95, 175, 215},
    {95, 175, 255},
    {95, 215, 0},
    {95, 215, 95},
    {95, 215, 135},
    {95, 215, 175},
    {95, 215, 215},
    {95, 215, 255},
    {95, 255, 0},
    {95, 255, 95},
    {95, 255, 135},
    {95, 255, 175},
    {95, 255, 215},
    {95, 255, 255},
    {135, 0, 0},
    {135, 0, 95},
    {135, 0, 135},
    {135, 0, 175},
    {135, 0, 215},
    {135, 0, 255},
    {135, 95, 0},
    {135, 95, 95},
    {135, 95, 135},
    {135, 95, 175},
    {135, 95, 215},
    {135, 95, 255},
    {135, 135, 0},
    {135, 135, 95},
    {135, 135, 135},
    {135, 135, 175},
    {135, 135, 215},
    {135, 135, 255},
    {135, 175, 0},
    {135, 175, 95},
    {135, 175, 135},
    {135, 175, 175},
    {135, 175, 215},
    {135, 175, 255},
    {135, 215, 0},
    {135, 215, 95},
    {135, 215, 135},
    {135, 215, 175},
    {135, 215, 215},
    {135, 215, 255},
    {135, 255, 0},
    {135, 255, 95},
    {135, 255, 135},
    {135, 255, 175},
    {135, 255, 215},
    {135, 255, 255},
    {175, 0, 0},
    {175, 0, 95},
    {175, 0, 135},
    {175, 0, 175},
    {175, 0, 215},
    {175, 0, 255},
    {175, 95, 0},
    {175, 95, 95},
    {175, 95, 135},
    {175, 95, 175},
    {175, 95, 215},
    {175, 95, 255},
    {175, 135, 0},
    {175, 135, 95},
    {175, 135, 135},
    {175, 135, 175},
    {175, 135, 215},
    {175, 135, 255},
    {175, 175, 0},
    {175, 175, 95},
    {175, 175, 135},
    {175, 175, 175},
    {175, 175, 215},
    {175, 175, 255},
    {175, 215, 0},
    {175, 215, 95},
    {175, 215, 135},
    {175, 215, 175},
    {175, 215, 215},
    {175, 215, 255},
    {175, 255, 0},
    {175, 255, 95},
    {175, 255, 135},
    {175, 255, 175},
    {175, 255, 215},
    {175, 255, 255},
    {215, 0, 0},
    {215, 0, 95},
    {215, 0, 135},
    {215, 0, 175},
    {215, 0, 215},
    {215, 0, 255},
    {215, 95, 0},
    {215, 95, 95},
    {215, 95, 135},
    {215, 95, 175},
    {215, 95, 215},
    {215, 95, 255},
    {215, 135, 0},
    {215, 135, 95},
    {215, 135, 135},
    {215, 135, 175},
    {215, 135, 215},
    {215, 135, 255},
    {215, 175, 0},
    {215, 175, 95},
    {215, 175, 135},
    {215, 175, 175},
    {215, 175, 215},
    {215, 175, 255},
    {215, 215, 0},
    {215, 215, 95},
    {215, 215, 135},
    {215, 215, 175},
    {215, 215, 215},
    {215, 215, 255},
    {215, 255, 0},
    {215, 255, 95},
    {215, 255, 135},
    {215, 255, 175},
    {215, 255, 215},
    {215, 255, 255},
    {255, 0, 0},
    {255, 0, 95},
    {255, 0, 135},
    {255, 0, 175},
    {255, 0, 215},
    {255, 0, 255},
    {255, 95, 0},
    {255, 95, 95},
    {255, 95, 135},
    {255, 95, 175},
    {255, 95, 215},
    {255, 95, 255},
    {255, 135, 0},
    {255, 135, 95},
    {255, 135, 135},
    {255, 135, 175},
    {255, 135, 215},
    {255, 135, 255},
    {255, 175, 0},
    {255, 175, 95},
    {255, 175, 135},
    {255, 175, 175},
    {255, 175, 215},
    {255, 175, 255},
    {255, 215, 0},
    {255, 215, 95},
    {255, 215, 135},
    {255, 215, 175},
    {255, 215, 215},
    {255, 215, 255},
    {255, 255, 0},
    {255, 255, 95},
    {255, 255, 135},
    {255, 255, 175},
    {255, 255, 215},
    {255, 255, 255},
    {8, 8, 8},
    {18, 18, 18},
    {28, 28, 28},
    {38, 38, 38},
    {48, 48, 48},
    {58, 58, 58},
    {68, 68, 68},
    {78, 78, 78},
    {88, 88, 88},
    {98, 98, 98},
    {108, 108, 108},
    {118, 118, 118},
    {128, 128, 128},
    {138, 138, 138},
    {148, 148, 148},
    {158, 158, 158},
    {168, 168, 168},
    {178, 178, 178},
    {188, 188, 188},
    {198, 198, 198},
    {208, 208, 208},
    {218, 218, 218},
    {228, 228, 228},
    {238, 238, 238},
};

//! Length of ext2rgb_map  (should always be 256).
const size_t ext2rgb_map_len = sizeof(ext2rgb_map) / sizeof(ext2rgb_map[0]);


/*! Returns the char needed to represent an escape sequence in C.

    \details
    The following characters are supported:
        Escape Sequence  |  Description Representation
        ---------------: | :--------------------------
                    @\ ' | single quote
                   @\ @" | double quote
                    @\ ? | question mark
                   @\ @\ | backslash
                    @\ a | audible bell
                    @\ b | backspace
                    @\ f | form feed - new page
                    @\ n | line feed - new line
                    @\ r | carriage return
                    @\ t | horizontal tab
                    @\ v | vertical tab

    \pi c   The character to check.
    \return The letter, without a backslash, needed to create an escape sequence.
            If the char doesn't need an escape sequence, it is simply returned.

    \examplecodefor{char_escape_char,.c}
        char constantchar = char_escape_char('\n');
        assert(constantchar == 'n');

        char constantquote = char_escape_char('"');
        assert(constantquote == '"');

        // The actual escape sequence would need the backslash added to it:
        char* escaped;
        asprintf(&escaped, "\\%c", char_escape_char('\t'));
        free(escaped);
    \endexamplecode
*/
char char_escape_char(const char c) {
    switch (c) {
        // 0 is a special case for char_repr().
        case '\0': return '0';
        case '\'': return '\'';
        case '\"': return '"';
        case '\?': return '?';
        case '\\': return '\\';
        case '\a': return 'a';
        case '\b': return 'b';
        case '\f': return 'f';
        case '\n': return 'n';
        case '\r': return 'r';
        case '\t': return 't';
        case '\v': return 'v';
        default:
            return c;
    }
}

/*! Determines if a character exists in the given string.
    \pi c Character to search for.
    \pi s String to check.
          \mustnullin

    \return `true` if \p c is found in \p s, otherwise `false`.
*/
bool char_in_str(const char c, const char* s) {
    size_t length = strlen(s);
    for (size_t i = 0; i < length; i++) {
        if (s[i] == c) return true;
    }
    return false;
}

/*! Determines if a character is suitable for an escape code ending.

    \details
    `m` is used as the last character in color codes, but other characters
    can be used for escape sequences (such as "\033[2A", cursor up). Actual
    escape code endings can be in the range (`char`) 64-126 (inclusive).

    \details
    Since ColrC only deals with color codes and maybe some cursor/erase codes,
    this function tests if the character is either `A-Z` or `a-z`.

    \details
    For more information, see: https://en.wikipedia.org/wiki/ANSI_escape_code

    \pi c   Character to test.
    \return `true` if the character is a possible escape code ending, otherwise `false`.
*/
bool char_is_code_end(const char c) {
    /*  The actual end chars can be: 64-126 (inclusive) ( ASCII: @A–Z[\]^_`a–z{ )
        I'm just testing for alpha chars. A: 65, Z: 90, a: 97, z: 122
        This is not a macro because it may be expanded in the future to detect
        the full range of "end" chars.
    */
    // Lowercase `m` is the most common case for ColrC.
    return ((c > 64) && (c < 91)) || ((c > 96) && (c < 123));
}

/*! Creates a string representation for a char.

    \pi c   Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* char_repr(char c) {
    char* repr;
    switch (c) {
        case '\0':
            asprintf_or_return(NULL, &repr, "'\\0'");
            break;
        case '\033':
            asprintf_or_return(NULL, &repr, "'\\033'");
            break;
        case '\'':
            asprintf_or_return(NULL, &repr, "'\\\'");
            break;
        case '\"':
            asprintf_or_return(NULL, &repr, "'\\\"'");
            break;
        case '\?':
            asprintf_or_return(NULL, &repr, "'\\?'");
            break;
        case '\\':
            asprintf_or_return(NULL, &repr, "'\\\\");
            break;
        case '\a':
            asprintf_or_return(NULL, &repr, "'\\a'");
            break;
        case '\b':
            asprintf_or_return(NULL, &repr, "'\\b'");
            break;
        case '\f':
            asprintf_or_return(NULL, &repr, "'\\f'");
            break;
        case '\n':
            asprintf_or_return(NULL, &repr, "'\\n'");
            break;
        case '\r':
            asprintf_or_return(NULL, &repr, "'\\r'");
            break;
        case '\t':
            asprintf_or_return(NULL, &repr, "'\\t'");
            break;
        case '\v':
            asprintf_or_return(NULL, &repr, "'\\v'");
            break;
        default:
            if (iscntrl(c)) {
                // Handle all other non-printables in hex-form.
                asprintf_or_return(NULL, &repr, "'\\x%x'", c);
            } else {
                asprintf_or_return(NULL, &repr, "'%c'", c);
            }
    }
    return repr;
}

/*! Determines if an ascii character has an escape sequence in C.

    \details
    The following characters are supported:
        Escape Sequence  |  Description Representation
        ---------------: | :--------------------------
                    @\ ' | single quote
                   @\ @" | double quote
                    @\ ? | question mark
                   @\ @\ | backslash
                    @\ a | audible bell
                    @\ b | backspace
                    @\ f | form feed - new page
                    @\ n | line feed - new line
                    @\ r | carriage return
                    @\ t | horizontal tab
                    @\ v | vertical tab

    \pi c   The character to check.
    \return `true` if the character needs an escape sequence, otherwise `false`.
*/
bool char_should_escape(const char c) {
    switch (c) {
        // 0 is a special case for char_repr().
        case '\0': return true;
        case '\'': return true;
        case '\"': return true;
        case '\?': return true;
        case '\\': return true;
        case '\a': return true;
        case '\b': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default:
            return false;
    }
}

/*! Allocates an empty string.
    \details
    This is for keeping the interface simple, so the return values from
    color functions with invalid values can be consistent.

    \return Pointer to an allocated string consisting of '\0'.
*/
char* colr_empty_str(void) {
    char* s = malloc(sizeof(char));
    if (!s) return NULL;
    s[0] = '\0';
    return s;
}

/*! Determine whether the current environment support RGB (True Colors).

    \details
    This checks `$COLORTERM` for the appropriate value (`'truecolor'` or `'24bit'`).
    On "dumber" terminals, RGB codes are probably ignored or mistaken for a
    256-color or even 8-color value.

    \details
    For instance, RGB is supported in `konsole`, but not in `xterm` or `linux`
    ttys. Using RGB codes in `xterm` makes the colors appear as though a 256-color
    value was used (closest matching value, like RGB_to_term_RGB()).
    Using RGB codes in a simpler `linux` tty makes them appear as though an 8-color
    value was used. Very ugly, but not a disaster.

    \details
    I haven't seen a <em>modern</em> linux terminal spew garbage across the screen
    from using RGB codes when they are not supported, but I could be wrong.
    I would like to see that terminal if you know of one.

    \return `true` if 24-bit (true color, or "rgb") support is detected, otherwise `false`.
*/
bool colr_supports_rgb(void) {
    char* colorterm;
    // Check $COLORTERM for 'truecolor' or '24bit'
    if ((colorterm = getenv("COLORTERM"))) {
        if (colr_istr_either(colorterm, "truecolor", "24bit")) return true;
    }
    // TODO: Send an rgb code, test the terminal response?
    // char* testcode = "\033[38:2:255:255:255m\033P$qm\033\\\n";
    // Should get: 2:255:255:255m
    return false;
}

/*! Attempts to retrieve the row/column size of the terminal and returns a TermSize.

    \details
    If the call to `ioctl()` fails, a default TermSize struct is returned:
    \code
    (TermSize){.rows=35, .columns=80}
    \endcode
    \return A TermSize struct with terminal size information.
*/
TermSize colr_term_size(void) {
    struct winsize ws = colr_win_size();
    return (TermSize){.rows=ws.ws_row, .columns=ws.ws_col};
}

/*! Attempts to retrieve a `winsize` struct from an `ioctl` call.

    \details
    If the call fails, the environment variables `LINES` and `COLUMNS` are checked.
    If that fails, a default `winsize` struct is returned:
    \code
    (struct winsize){.ws_row=35, .ws_col=80, .ws_xpixel=0, .ws_ypixel=0}
    \endcode

    \details
    `man ioctl_tty` says that `.ws_xpixel` and `.ws_ypixel` are unused.

    \return A `winsize` struct (`sys/ioctl.h`) with window size information.
*/
struct winsize colr_win_size(void) {
    struct winsize ws = {0, 0, 0, 0};
    if (ioctl(0, TIOCGWINSZ, &ws) < 0) {
        // No support?
        dbug("No support for ioctl TIOCGWINSZ, using defaults.");
        char* env_rows = getenv("LINES");
        unsigned short default_rows = 0;
        if (sscanf(env_rows, "%hu", &default_rows) != 1) {
            default_rows = 35;
        }
        char* env_cols = getenv("COLUMNS");
        unsigned short default_cols = 0;
        if (sscanf(env_cols, "%hu", &default_cols) != 1) {
            default_cols = 80;
        }
        return (struct winsize){
            .ws_row=default_rows,
            .ws_col=default_cols,
            .ws_xpixel=0,
            .ws_ypixel=0
        };
    }
    return ws;
}

/*! Create an escape code for a background color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for background.
*/
void format_bg(char* out, BasicValue value) {
    if (!out) return;
    snprintf(out, CODE_LEN, "\033[%dm", BasicValue_to_ansi(BACK, value));
}

/*! Create an escape code for an extended background color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for background.
*/
void format_bgx(char* out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\033[48;5;%dm", num);
}

/*! Create an escape code for a true color (rgb) background color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODE_RGB_LEN`._
    \pi redval   Value for red.
    \pi greenval Value for green.
    \pi blueval  Value for blue.
*/
void format_bg_rgb(char* out, unsigned char redval, unsigned char greenval, unsigned char blueval) {
    if (!out) return;
    format_bg_RGB(out, rgb(redval, greenval, blueval));
}

/*! Create an escape code for a true color (rgb) background color
    using values from an RGB struct.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODE_RGB_LEN`._
    \pi rgb RGB struct to get red, blue, and green values from.
*/
void format_bg_RGB(char* out, RGB rgb) {
    if (!out) return;
    snprintf(out, CODE_RGB_LEN, "\033[48;2;%d;%d;%dm", rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values, approximating 256-color values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_bg_RGB_term(char* out, RGB rgb) {
    format_bgx(out, ExtendedValue_from_RGB(rgb));
}

/*! Create an escape code for a fore color.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `CODEX_LEN`._
    \pi value BasicValue value to use for fore.
*/
void format_fg(char* out, BasicValue value) {
    if (!out) return;
    snprintf(out, CODE_LEN, "\033[%dm", BasicValue_to_ansi(FORE, value));
}

/*! Create an escape code for an extended fore color.

    \po out Memory allocated for the escape code string.
            _Must have enough room for `CODEX_LEN`._
    \pi num Value to use for fore.
*/
void format_fgx(char* out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\033[38;5;%dm", num);
}

/*! Create an escape code for a true color (rgb) fore color.

    \po out      Memory allocated for the escape code string.
                 _Must have enough room for `CODE_RGB_LEN`._
    \pi redval   Value for red.
    \pi greenval Value for green.
    \pi blueval  Value for blue.
*/
void format_fg_rgb(char* out, unsigned char redval, unsigned char greenval, unsigned char blueval) {
    if (!out) return;
    format_fg_RGB(out, rgb(redval, greenval, blueval));
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB(char* out, RGB rgb) {
    if (!out) return;
        snprintf(out, CODE_RGB_LEN, "\033[38;2;%d;%d;%dm", rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values, approximating 256-color values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB_term(char* out, RGB rgb) {
    format_fgx(out, ExtendedValue_from_RGB(rgb));
}

/*! Create an escape code for a style.

    \po out   Memory allocated for the escape code string.
              _Must have enough room for `STYLE_LEN`._
    \pi style StyleValue value to use for style.
*/
void format_style(char* out, StyleValue style) {
    if (!out) return;
    snprintf(out, STYLE_LEN, "\033[%dm", style < 0 ? RESET_ALL: style);
}

/*! Appends CODE_RESET_ALL to a string, but makes sure to do it before any
    newlines.

    \details
    \mustnullin

    \pi s The string to append to.
          <em>Must have extra room for CODE_RESET_ALL</em>.
*/
void str_append_reset(char *s) {
    if (!s) return;
    if (s[0] == '\0') {
        // Special case, an empty string, with room for CODE_RESET_ALL.
        snprintf(s, CODE_RESET_LEN, "%s", CODE_RESET_ALL);
        return;
    }
    if (str_ends_with(s, CODE_RESET_ALL)) {
        // Already has one.
        return;
    }
    size_t length = strlen(s);
    size_t lastindex = length - 1;
    size_t newlines = 0;
    // Cut newlines off if needed. I'll add them after the reset code.
    while ((lastindex > 0) && (s[lastindex] == '\n')) {
        s[lastindex] = '\0';
        newlines++;
        lastindex--;
    }
    if ((lastindex == 0) && s[lastindex] == '\n') {
        // String starts with a newline.
        s[lastindex] = '\0';
        newlines++;
    } else {
        lastindex++;
    }
    char* p = s + lastindex;
    snprintf(p, CODE_RESET_LEN, "%s", CODE_RESET_ALL);
    p += CODE_RESET_LEN - 1;
    while (newlines--) {
        *(p++) = '\n';
    }
    *p = '\0';
}

/*! Center-justifies a string, ignoring escape codes when measuring the width.

    \pi s       The string to justify.\n
                \mustnullin
    \pi padchar The character to pad with. If '0', then `' '` is used.
    \pi width   The overall width for the resulting string.\n
                If set to '0', the terminal width will be used from colr_term_size().

    \return     An allocated string with the result,
                or `NULL` if \p s is `NULL` or the allocation failed.
                or `NULL` if \p s is `NULL` or the allocation failed.

    \sa str_ljust str_rjust colr_term_size
*/
char* str_center(const char* s, const char padchar, int width) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    size_t pos = 0;
    size_t leftdiff = diff / 2;
    if (diff % 2 == 1) leftdiff++;
    // Handle left-just.
    while (pos < leftdiff) {
        result[pos++] = pad;
    }
    // Handle string.
    size_t i = 0;
    while (s[i]) {
        result[pos++] = s[i++];
    }
    // Handle remaining right-just.
    final_len--;
    while (pos < final_len) {
        result[pos++] = pad;
    }
    return result;
}

/*! Counts the number of characters (`c`) that are found in a string (`s`).

    \details
    Returns `0` if `s` is `NULL`, or `c` is `'\0'`.

    \pi s The string to examine.
          \mustnullin
    \pi c The character to count.
          \mustnotzero

    \return The number of times \p c occurs in \p s.
*/
size_t str_char_count(const char* s, const char c) {
    if (!(s && c)) return 0;
    if (s[0] == '\0') return 0;

    size_t i = 0;
    size_t total = 0;
    while (s[i]) {
        if (s[i++] == c) total++;
    }
    return total;
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
char* str_copy(char* dest, const char* src, size_t length) {
    if (!(src && dest)) {
        return NULL;
    }
    size_t pos;
    for (pos=0; pos < length && src[pos] != '\0'; pos++) {
        dest[pos] = src[pos];
    }
    dest[pos] = '\0';
    return dest;
}

/*! Determine if one string ends with another.

    \details
    `str` and `suf` \mustnull

    \pi str String to check.
    \pi suf Suffix to check for.
    \return True if `str` ends with `suf`.
    \return False if either is NULL, or the string doesn't end with the suffix.
*/
bool str_ends_with(const char* str, const char* suf) {
    if (!str || !suf) {
        return false;
    }
    size_t strlength = strlen(str);
    size_t suflength = strlen(suf);
    if ((!(suflength && strlength)) || (suflength > strlength)) {
        // Empty strings, or suffix is longer than the entire string.
        return false;
    }
    return (strncmp(str + (strlength - suflength), suf, suflength) == 0);
}

/*! Determines if a string (`char*`) has ANSI escape codes in it.

    \details
    This will detect any ansi escape code, not just colors.

    \pi s   The string to check. Can be `NULL`.
            \mustnullin

    \return `true` if the string has at least one escape code, otherwise `false`.

    \sa str_is_codes
*/
bool str_has_codes(const char* s) {
    if (!s) return false;
    size_t length = strlen(s);
    size_t i = 0;
    while ((i < length) && s[i]) {
        if ((s[i] == '\033') && (s[i + 1] == '[')) {
            // Skip past "\033["
            i += 2;
            while ((i < length) && s[i]) {
                if (s[i] == 'm') return true;
                if (!(isdigit(s[i]) || s[i] == ';')) return false;
                i++;
            }
        }
        i++;
    }
    return false;
}

/*! Determines whether a string consists of only one character, possibly repeated.

    \pi s   String to check.
    \pi c   Character to test for. Must not be `0`.

    \return `true` if \p s contains only the character \p c, otherwise `false`.
*/
bool str_is_all(const char* s, const char c) {
    if (!(s && c)) return false;
    size_t i = 0;
    while (s[i]) {
        if (s[i] != c) return false;
        i++;
    }
    return true;
}

/*! Determines if a string is composed entirely of escape codes.

    \details
    Returns `false` if the string is `NULL`, or empty.

    \pi s   The string to check.
            \mustnullin
    \return `true` if the string is escape-codes only, otherwise `false`.

    \sa str_has_codes
*/
bool str_is_codes(const char* s) {
    if (!s) return false;
    if (s[0] == '\0') return false;
    size_t i = 0;
    while (s[i]) {
        if (s[i] == '\033') {
            // Skip past the code.
            while (!char_is_code_end(s[i++]));
            continue;
        }
        // Found a non-escape-code char.
        return false;
    }
    return true;
}
/*! Determines whether all characters in a string are digits.

    \details
    If \p s is NULL or an empty string (`""`), `false` is returned.

    \pi s   String to check.
            \mustnullin
    \return `true` if all characters are digits (0-9), otherwise `false`.
*/
bool str_is_digits(const char* s) {
    if (!s) return false;
    if (s[0] == '\0') return false;

    size_t i = 0;
    while (s[i]) {
        if (!isdigit(s[i])) return false;
        i++;
    }
    return true;
}

/*! Converts a string into lower case in place.
    \details
    \mustnullin

    \details
    If `s` is `NULL`, nothing is done.

    \pi s The input string to convert to lower case.
*/
void str_lower(char* s) {
    if (!s) return;
    size_t i = 0;
    while (s[i]) {
        char c = tolower(s[i]);
        s[i] = c;
        i++;
    }
    // This works for empty strings too.
    if (s[i] != '\0') s[i] = '\0';
}


/*! Left-justifies a string, ignoring escape codes when measuring the width.

    \pi s       The string to justify.\n
                \mustnullin
    \pi padchar The character to pad with. If '0', then `' '` is used.
    \pi width   The overall width for the resulting string.\n
                If set to '0', the terminal width will be used from colr_term_size().

    \return     An allocated string with the result,
                or `NULL` if \p s is `NULL` or the allocation failed.

    \sa str_center str_rjust colr_term_size
*/
char* str_ljust(const char* s, const char padchar, int width) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    char* start = result;
    sprintf(result, "%s", s);
    int pos = 0;
    while (pos < diff) {
        result[length + pos++] = pad;
    }
    return start;
}

/*! Removes certain characters from the start of a string.
    \details
    The order of the characters in \p chars does not matter. If any of them
    are found at the start of a string, they will be removed.

    `str_lstrip_chars("aabbccTEST", "bca") == "TEST"`


    \pi s     The string to strip.
              \p s \mustnull
    \pi chars A string of characters to remove. Each will be removed from the start
              of the string.
              \p chars \mustnull
    \return   An allocated string with the result. May return NULL if the allocation
              fails, or if \p s or \p chars is NULL.
              \mustfree
*/
char* str_lstrip_chars(const char* s, const char* chars) {
    if (!(s && chars)) return NULL;
    if ((s[0] == '\0') || (chars[0] == '\0')) return NULL;

    size_t length = strlen(s);
    char* result = calloc(length + 1, sizeof(char));
    size_t result_pos = 0;
    bool done_trimming = false;
    for (size_t i = 0; i < length; i++) {
        if ((!done_trimming) && char_in_str(s[i], chars)) {
            continue;
        } else {
            // First non-`chars` character. We're done.
            done_trimming = true;
        }
        result[result_pos] = s[i];
        result_pos++;
    }
    return result;
}


/*! Returns the number of characters in a string, taking into account possibly
    multi-byte characters.

    \pi s The string to get the length of.
    \return The number of characters, single and multi-byte, or `0` if \p s is
            `NULL`, empty, or has invalid multibyte sequences.
*/
size_t str_mb_len(const char* s) {
    if ((!s) || (s[0] == '\0')) return 0;
    int i = 0;
    int next_len = 0;
    size_t total = 0;
    while ((next_len = mblen(s + i, 6))) {
        if (next_len < 0) {
            dbug("Invalid multibyte sequence at: %d\n", i);
            return 0;
        }
        i += next_len;
        total++;
    }
    return total;
}

/*! Returns the length of string, ignoring escape codes and the the null-terminator.

    \pi s   String to get the length for.
            \mustnullin
    \return The length of the string, as if it didn't contain escape codes.\n
            For non-escape-code strings, this is like `strlen()`.\n
            For `NULL` or "empty" strings, `0` is returned.

    \sa str_strip_codes
*/
size_t str_noncode_len(const char* s) {
    if (!s) return 0;
    if (s[0] == '\0') return 0;
    size_t i = 0, total = 0;
    while (s[i]) {
        if (s[i] == '\033') {
            // Skip past the code.
            while (!char_is_code_end(s[i++]));
            continue;
        }
        i++;
        total++;
    }
    return total;
}

/*! Convert a string into a representation of a string, by wrapping it in
    quotes and escaping characters that need escaping.

    \details
    If \p s is NULL, then an allocated string containing the string "NULL" is
    returned (without quotes).

    \details
    Escape codes will be escaped, so the terminal will ignore them if the
    result is printed.

    \pi     s The string to represent.
    \return An allocated string with the respresentation.\n
            \mustfree

    \sa char_should_escape char_escape_char

    \examplecodefor{str_repr,.c}
    char* s = str_repr("This\nhas \bspecial\tchars.");
    // The string `s` contains an escaped string, it *looks like* the definition,
    // but no real newlines, backspaces, or tabs are in it.
    assert(strcmp(s, "\"This\\nhas \\bspecial\\tchars.\"") == 0);
    free(s);
    \endexamplecode
*/
char* str_repr(const char* s) {
    if (!s) {
        char* nullrepr;
        asprintf_or_return(NULL, &nullrepr, "NULL");
        return nullrepr;
    }
    if (s[0] == '\0') {
        char* emptyrepr;
        asprintf_or_return(NULL, &emptyrepr, "\"\"");
        return emptyrepr;
    }
    size_t length = strlen(s);
    size_t esc_chars = 0;
    size_t i;
    for (i = 0; i < length; i++) {
        if (char_should_escape(s[i])) esc_chars++;
        else if (s[i] == '\033') esc_chars += 4;
    }
    size_t repr_length = length + (esc_chars * 2);
    // Make room for the wrapping quotes, and a null-terminator.
    repr_length += 3;
    char *repr = calloc(repr_length + 1, sizeof(char));
    size_t inew = 0;
    repr[0] = '"';
    for (i = 0, inew = 1; i < length; i++) {
        char c = s[i];
        if (char_should_escape(c)) {
            repr[inew++] = '\\';
            repr[inew++] = char_escape_char(c);
        } else if (c == '\033') {
            repr[inew++] = '\\';
            repr[inew++] = '0';
            repr[inew++] = '3';
            repr[inew++] = '3';
        } else {
            repr[inew++] = c;
        }
    }
    repr[inew] = '"';
    return repr;
}

/*! Right-justifies a string, ignoring escape codes when measuring the width.

    \pi s       The string to justify.\n
                \mustnullin
    \pi padchar The character to pad with. If '0', then `' '` is used.
    \pi width   The overall width for the resulting string.\n
                If set to '0', the terminal width will be used from colr_term_size().

    \return     An allocated string with the result,
                or `NULL` if \p s is `NULL` or the allocation failed.

    \sa str_center str_ljust colr_term_size
*/
char* str_rjust(const char* s, const char padchar, int width) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    char* start = result;
    int pos = 0;
    while (pos < diff) {
        result[pos++] = pad;
    }
    result = result + pos;
    sprintf(result, "%s", s);
    return start;
}

/*! Checks a string for a certain prefix substring.

    \details
    `prefix` \mustnull

    \pi s      The string to check.
    \pi prefix The prefix string to look for.

    \return True if the string `s` starts with prefix.
    \return False if one of the strings is null, or the prefix isn't found.
*/
bool str_starts_with(const char* s, const char* prefix) {
    if (!(s && prefix)) {
        // One of the strings is null.
        return false;
    }
    if ((s[0] == '\0') || (prefix[0] == '\0')) {
        // One of the strings is empty.
        return false;
    }
    size_t pre_len = strlen(prefix);
    for (size_t i = 0; i < pre_len; i++) {
        if (s[i] == '\0') {
            // Reached the end of s before the end of prefix.
            return false;
        }
        if (prefix[i] != s[i]) {
            // Character differs from the prefix.
            return false;
        }
    }
    return true;
}

/*! Strips escape codes from a string, resulting in a new allocated string.

    \pi s   The string to strip escape codes from.
            \mustnullin
    \return An allocated string with the result, or `NULL` if the allocation fails.\n
            \mustfree

    \sa str_noncode_len
*/
char* str_strip_codes(const char* s) {
    if (!s) return NULL;
    if (s[0] == '\0') return colr_empty_str();
    size_t length = strlen(s);
    char* final = calloc(length + 1, sizeof(char));
    size_t i = 0, pos = 0;
    while (s[i]) {
        if (s[i] == '\033') {
            // Skip past the code.
            while (!char_is_code_end(s[i++]));
            continue;
        }
        final[pos++] = s[i++];
    }
    return final;
}

/*! Allocate a new lowercase version of a string.

    \details
    \mustfree

    \pi s   The input string to convert to lower case.\n
            \mustnull
    \return The allocated string, or `NULL` if \p s is `NULL` or the allocation fails.
*/
char* str_to_lower(const char* s) {
    if (!s) return NULL;
    size_t length = strlen(s);
    char* out = calloc(length + 1, sizeof(char));
    if (!out) return NULL;
    if (s[0] == '\0') return out;

    size_t i = 0;
    while (s[i]) {
        out[i] = tolower(s[i]);
        i++;
    }
    return out;
}

/* ---------------------------- ColrC Functions ---------------------------- */

/*! Joins ColorArgs, ColorTexts, and strings into one long string.

    \details
    This will free() any ColorArgs and ColorTexts that are passed in. It is
    backing the colr() macro, and enables easy throwaway color values.

    \details
    Any plain strings that are passed in are left alone. It is up to the caller
    to free those. ColrC only manages the temporary Colr-based objects needed
    to build up these strings.

    \pi p   The first of any ColorArgs, ColorTexts, or strings to join.
    \pi ... Zero or more ColorArgs, ColorTexts, or strings to join.
    \return An allocated string with mixed escape codes/strings.\n
            CODE_RESET_ALL is appended to all the pieces that aren't plain
            strings. This allows easy part-colored messages, so there's no
            need to use CODE_RESET_ALL directly.\n
            \mustfree
*/
char* _colr(void *p, ...) {
    // Argument list must have ColorArg/ColorText with NULL members at the end.
    if (!p) {
        return colr_empty_str();
    }
    va_list args;
    va_start(args, p);
    va_list argcopy;
    va_copy(argcopy, args);
    size_t length = _colr_length(p, argcopy);
    va_end(argcopy);
    // If length was 1, there were no usable values in the argument list.
    if (length == 1) {
        va_end(args);
        return colr_empty_str();
    }
    // Allocate enough for the reset code at the end.
    char* final = calloc(length, sizeof(char));

    char* s;
    ColorArg *cargp = NULL;
    ColorText *ctextp = NULL;
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
        s = (char* )p;
    }
    strcat(final, s);
    if (cargp || ctextp) {
        // Free the temporary string created with Color(Arg/Text)_to_str().
        free(s);
    }

    void *arg = NULL;
    while ((arg = va_arg(args, void*))) {
        cargp = NULL;
        ctextp = NULL;
        bool is_string = false;
        // These ColorArgs/ColorTexts were heap allocated through the fore,
        // back, style, and ColrC macros. I'm going to free them, so the user
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
            s = (char* )arg;
            is_string = true;
        }
        strcat(final, s);
        // String was passed, add the reset code.
        if (is_string) str_append_reset(final);

        // Free the temporary string from those ColorArgs/ColorTexts.
        if (cargp || ctextp) free(s);
    }
    str_append_reset(final);
    va_end(args);
    return final;
}

/*! Parse arguments, just as in _colr(), but only return the length needed to
    allocate the resulting string.

    \details
    This allows _colr() to allocate once, instead of reallocating for each
    argument that is passed.

    \pi p    The first of any ColorArg, ColorText, or strings to join.
    \pi args A `va_list` with zero or more ColorArgs, ColorTexts, or strings to join.

    \return  The length (in bytes) needed to allocate a string built with _colr().

    \sa _colr
*/
size_t _colr_length(void *p, va_list args) {
    // Argument list must have ColorArg/ColorText with NULL members at the end.
    if (!p) return 0;
    ColorArg *cargp = NULL;
    ColorText *ctextp = NULL;
    size_t length = 1;
    if (ColorArg_is_ptr(p)) {
        // It's a ColorArg.
        cargp = p;
        length += ColorArg_length(*cargp);
    } else if (ColorText_is_ptr(p)) {
        ctextp = p;
        length += ColorText_length(*ctextp);
    } else {
        // It's a string, or it better be anyway.
        length += strlen((char* )p);
    }
    length += CODE_RESET_LEN;
    void *arg = NULL;
    while ((arg = va_arg(args, void*))) {
        cargp = NULL;
        ctextp = NULL;
        bool is_string = false;
        // These ColorArgs/ColorTexts were heap allocated through the fore,
        // back, style, and ColrC macros. I'm going to free them, so the user
        // doesn't have to keep track of all the temporary pieces that built
        // this string.
        if (ColorArg_is_ptr(arg)) {
            // It's a ColorArg.
            cargp = arg;
            length += ColorArg_length(*cargp);
        } else if (ColorText_is_ptr(arg)) {
            ctextp = arg;
            length += ColorText_length(*ctextp);
        } else {
            // It better be a string.
            length += strlen((char* )arg);
            is_string = true;
        }
        length += 1;
        // String was passed, add the reset code.
        if (is_string) length += CODE_RESET_LEN;
    }
    return length;
}

/*! Joins ColorArgs, ColorTexts, and strings into one long string separated
    by it's first argument.

    \details
    This will free() any ColorArgs and ColorTexts that are passed in. It is
    backing the colr() macro, and enables easy throwaway color values.

    \details
    Any plain strings that are passed in are left alone. It is up to the caller
    to free those. ColrC only manages the temporary Colr-based objects needed
    to build up these strings.

    \pi joinerp The joiner (any ColorArg, ColorText, or string).
    \pi ...     Zero or more ColorArgs, ColorTexts, or strings to join by the joiner.
    \return     An allocated string with mixed escape codes/strings.\n
                CODE_RESET_ALL is appended to all ColorText arguments.
                This allows easy part-colored messages.\n
                \mustfree
*/
char* _colr_join(void *joinerp, ...) {
    // Argument list must have ColorArg/ColorText with NULL members at the end.
    if (!joinerp) {
        return colr_empty_str();
    }
    va_list args;
    va_start(args, joinerp);
    va_list argcopy;
    va_copy(argcopy, args);
    size_t length = _colr_join_length(joinerp, argcopy);
    va_end(argcopy);
    // If length is 1, then no usable values were passed in.
    if (length == 1) {
        va_end(args);
        return colr_empty_str();
    }

    char* final = calloc(length, sizeof(char));
    char* joiner;
    ColorArg* joiner_cargp = NULL;
    ColorText* joiner_ctextp = NULL;
    char* piece;
    ColorArg* cargp = NULL;
    ColorText* ctextp = NULL;
    if (ColorArg_is_ptr(joinerp)) {
        // It's a ColorArg.
        joiner_cargp = joinerp;
        joiner = ColorArg_to_str(*joiner_cargp);
        ColorArg_free(joiner_cargp);
    } else if (ColorText_is_ptr(joinerp)) {
        joiner_ctextp = joinerp;
        joiner = ColorText_to_str(*joiner_ctextp);
        ColorText_free(joiner_ctextp);
    } else {
        // It's a string, or it better be anyway.
        joiner = (char* )joinerp;
    }
    int count = 0;
    void *arg = NULL;
    while ((arg = va_arg(args, void*))) {
        cargp = NULL;
        ctextp = NULL;
        // These ColorArgs/ColorTexts were heap allocated through the fore,
        // back, style, and ColrC macros. I'm going to free them, so the user
        // doesn't have to keep track of all the temporary pieces that built
        // this string.
        if (ColorArg_is_ptr(arg)) {
            // It's a ColorArg.
            cargp = arg;
            piece = ColorArg_to_str(*cargp);
            ColorArg_free(cargp);
        } else if (ColorText_is_ptr(arg)) {
            ctextp = arg;
            piece = ColorText_to_str(*ctextp);
            ColorText_free(ctextp);
        } else {
            // It better be a string.
            piece = (char* )arg;
        }
        if (count++) strcat(final, joiner);
        strcat(final, piece);

        // Free the temporary string from those ColorArgs/ColorTexts.
        if (cargp || ctextp) free(piece);
    }
    va_end(args);
    if (joiner_cargp) {
        free(joiner);
    }
    if (joiner_ctextp) {
        free(joiner);
    }
    str_append_reset(final);
    return final;
}

/*! Parse arguments, just as in _colr_join(), but only return the length needed to
    allocate the resulting string.

    \details
    This allows _colr_join() to allocate once, instead of reallocating for each
    argument that is passed.

    \pi joinerp The first of any ColorArg, ColorText, or strings to join.
    \pi args    A `va_list` with zero or more ColorArgs, ColorTexts, or strings to join.

    \return     The length (in bytes) needed to allocate a string built with _colr().

    \sa _colr
*/
size_t _colr_join_length(void *joinerp, va_list args) {
    // Argument list must have ColorArg/ColorText with NULL members at the end.

    // No joiner, no strings. Empty string will be returned, so just "\0".
    if (!joinerp) return 1;

    ColorArg* joiner_cargp = NULL;
    ColorText* joiner_ctextp = NULL;
    ColorArg* cargp = NULL;
    ColorText* ctextp = NULL;
    size_t length = 1;
    size_t joiner_len = 0;
    if (ColorArg_is_ptr(joinerp)) {
        // It's a ColorArg.
        joiner_cargp = joinerp;
        joiner_len = ColorArg_length(*joiner_cargp);
    } else if (ColorText_is_ptr(joinerp)) {
        joiner_ctextp = joinerp;
        joiner_len = ColorText_length(*joiner_ctextp);
    } else {
        // It's a string, or it better be anyway.
        joiner_len = strlen((char* )joinerp);
    }
    int count = 0;
    void *arg = NULL;
    while ((arg = va_arg(args, void*))) {
        count++;
        cargp = NULL;
        ctextp = NULL;
        if (ColorArg_is_ptr(arg)) {
            // It's a ColorArg.
            cargp = arg;
            length += ColorArg_length(*cargp);
        } else if (ColorText_is_ptr(arg)) {
            ctextp = arg;
            length += ColorText_length(*ctextp);
        } else {
            // It better be a string.
            length += strlen((char* )arg);
        }
        if (count > 1) {
            length += joiner_len;
        }
    }
    length += CODE_RESET_LEN;
    return length;
}
/*! Creates a string representation of a ArgType.

    \pi type An ArgType to get the type from.
    \return  A pointer to an allocated string.\n
             \mustfree

    \sa ArgType
*/
char* ArgType_repr(ArgType type) {
    char* typestr;
    switch (type) {
        case ARGTYPE_NONE:
            asprintf_or_return(NULL, &typestr, "ARGTYPE_NONE");
            break;
        case FORE:
            asprintf_or_return(NULL, &typestr, "FORE");
            break;
        case BACK:
            asprintf_or_return(NULL, &typestr, "BACK");
            break;
        case STYLE:
            asprintf_or_return(NULL, &typestr, "STYLE");
            break;
    }
    return typestr;
}

/*! Creates a string from an ArgType.

    \pi type An ArgType to get the type from.
    \return  A pointer to an allocated string.\n
             \mustfree

    \sa ArgType
*/
char* ArgType_to_str(ArgType type) {
    char* typestr;
    switch (type) {
        case ARGTYPE_NONE:
            asprintf_or_return(NULL, &typestr, "none");
            break;
        case FORE:
            asprintf_or_return(NULL, &typestr, "fore");
            break;
        case BACK:
            asprintf_or_return(NULL, &typestr, "back");
            break;
        case STYLE:
            asprintf_or_return(NULL, &typestr, "style");
            break;
    }
    return typestr;
}

/*! Create a ColorArg with ARGTYPE_NONE and ColorValue.type.TYPE_NONE.
    \details
    This is used to pass "empty" fore/back/style args to the \colrmacros,
    where `NULL` has a different meaning (end of argument list).

    \return `(ColorArg){.type=ARGTYPE_NONE, .value.type=TYPE_NONE}`

    \sa ColorArg_is_empty ColorValue_empty
*/
ColorArg ColorArg_empty(void) {
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=ARGTYPE_NONE,
        .value=ColorValue_empty()
    };
}

/*! Compares two ColorArg structs.
    \details
    They are considered "equal" if their `.type` and `.value` match.

    \pi a   First ColorArg to compare.
    \pi b   Second ColorArg to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_eq(ColorArg a, ColorArg b) {
    return (a.type == b.type) && ColorValue_eq(a.value, b.value);
}

/*! Free allocated memory for a ColorArg.

    \details
    This has no advantage over `free(colorarg)` right now, it is used in
    debugging, and may be extended in the future. It's better just to use it.

    \pi p ColorArg to free.

    \sa ColorArg
*/
void ColorArg_free(ColorArg *p) {
    if (!p) return;
    free(p);
}

/*! Explicit version of ColorArg_from_value that only handles BasicValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value BasicValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    BasicValue val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_BASIC, &val),
    };
}

/*! Explicit version of ColorArg_from_value that only handles ExtendedValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value ExtendedValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    ExtendedValue val = value;
    return (ColorArg){
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

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_RGB(ArgType type, RGB value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    RGB val = value;
    return (ColorArg){
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

    \sa ColorArg
*/
ColorArg ColorArg_from_str(ArgType type, char* colorname) {
    ColorValue cval = ColorValue_from_str(colorname);
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=cval
    };

}

/*! Explicit version of ColorArg_from_value that only handles StyleValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value StyleValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    StyleValue val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_STYLE, &val),
    };
}

/*! Used with the color_arg macro to dynamically create a ColorArg based
    on it's argument type.

    \pi type     ArgType value, to mark the type of ColorArg.
    \pi colrtype ColorType value, to mark the type of ColorValue.
    \pi p        A pointer to either a BasicValue, ExtendedValue, or a RGB.

    \return A ColorArg struct with the appropriate `.value.type` member set for
            the value that was passed. For invalid types the `.value.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXTENDED_RANGE
        - TYPE_INVALID_RGB_RANGE

    \sa ColorArg
*/
ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void *p) {
    if (!p) {
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=ARGTYPE_NONE,
            .value=ColorValue_from_value(TYPE_INVALID, NULL)
        };
    }
    ColorArg carg = {
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(colrtype, p),
    };
    if ((type == STYLE) && (carg.value.type == TYPE_INVALID)) {
        carg.value.type = TYPE_INVALID_STYLE;
    }
    return carg;
}

/*! Checks to see if a ColorArg is an empty placeholder.

    \details
    A ColorArg is empty if it's `.type` is set to `ARGTYPE_NONE`.

    \pi carg A ColorArg to check.
    \return  `true` if the ColorArg is considered "empty", otherwise `false`.
*/
bool ColorArg_is_empty(ColorArg carg) {
    return (carg.type == ARGTYPE_NONE) || ColorValue_is_empty(carg.value);
}

/*! Checks to see if a ColorArg holds an invalid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is invalid, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_invalid(ColorArg carg) {
    return !(bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type));
}

/*! Checks a void pointer to see if it contains a ColorArg struct.

    \details The first member of a ColorArg is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorArg, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_ptr(void *p) {
    if (!p) return false;
    // The head of a ColorArg is always a valid marker.
    ColorArg *cargp = p;
    return cargp->marker == COLORARG_MARKER;
}

/*! Checks to see if a ColorArg holds a valid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is valid, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_valid(ColorArg carg) {
    return bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type);
}


/*! Returns the length in bytes needed to allocate a string built with
    ColorArg_to_str().

    \pi carg ColorArg to use.

    \return  The length (`size_t`) needed to allocate a ColorArg's string,
             or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorArg
*/
size_t ColorArg_length(ColorArg carg) {
    // Empty color args turn into empty strings, so just "\0".
    if (ColorArg_is_empty(carg)) return 1;
    return ColorValue_length(carg.type, carg.value);
}

/*! Creates a string representation for a ColorArg.

    \details
    Allocates memory for the string representation.

    \pi carg ColorArg struct to get the representation for.
    \return Allocated string for the representation.\n
            \mustfree

    \sa ColorArg
*/
char* ColorArg_repr(ColorArg carg) {
    char* type = ArgType_repr(carg.type);
    char* value = ColorValue_repr(carg.value);
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "ColorArg {.type=%s, .value=%s}",
        type,
        value
    );
    free(type);
    free(value);
    return repr;
}

/*! Copies a ColorArg into memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi carg ColorArg to copy/allocate for.
    \return Pointer to a heap-allocated ColorArg.

    \sa ColorArg
*/
ColorArg *ColorArg_to_ptr(ColorArg carg) {
    ColorArg *p = malloc(sizeof(carg));
    carg.marker = COLORARG_MARKER;
    *p = carg;
    return p;
}

/*! Converts a ColorArg into an escape code string.

    \details
    Allocates memory for the string.

    \details
    If the ColorArg is empty (`ARGTYPE_NONE`), an empty string is returned.

    \details
    If the ColorValue is invalid, an empty string is returned.
    You must still free the empty string.

    \pi carg ColorArg to get the ArgType and ColorValue from.
    \return Allocated string for the escape code.\n
            \mustfree

    \sa ColorArg
*/
char* ColorArg_to_str(ColorArg carg) {
    if (ColorArg_is_empty(carg)) return colr_empty_str();
    return ColorValue_to_str(carg.type, carg.value);
}

/*! Creates an "empty" ColorJustify, with JUST_NONE set.

    \return An initialized ColorJustify, with no justification method set.

    \sa ColorJustify
*/
ColorJustify ColorJustify_empty(void) {
    return (ColorJustify){
        .marker=COLORJUSTIFY_MARKER,
        .method=JUST_NONE,
        .width=0,
        .padchar=0,
    };
}

/*! Compares two ColorJustify structs.
    \details
    They are considered "equal" if their member values match.

    \pi a   First ColorJustify to compare.
    \pi b   Second ColorJustify to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorJustify
*/
bool ColorJustify_eq(ColorJustify a, ColorJustify b) {
    return (
        (a.method == b.method) &&
        (a.width == b.width) &&
        (a.padchar == b.padchar)
    );
}

/*! Checks to see if a ColorJustify is "empty".

    \details
    A ColorJustify is considered "empty" if the `.method` member is set to
    `JUST_NONE`.

    \pi cjust The ColorJustify to check.
    \return   `true` if the ColorJustify is empty, otherwise `false`.

    \sa ColorJustify ColorJustify_empty
*/
bool ColorJustify_is_empty(ColorJustify cjust) {
    return cjust.method == JUST_NONE;
}

/*! Creates a ColorJustify.

    \details
    This is used to ensure every ColorJustify has it's `.marker` member set
    correctly.

    \pi method  ColorJustifyMethod to use.
    \pi width   Width for justification.
                If `0` is given, ColorText will use the width from colr_terminal_size().
    \pi padchar Padding character to use.
                If `0` is given, the default, space (`' '`), is used.

    \return     An initialized ColorJustify.
*/
ColorJustify ColorJustify_new(ColorJustifyMethod method, int width, char padchar) {
    return (ColorJustify){
        .marker=COLORJUSTIFY_MARKER,
        .method=method,
        .width=width,
        .padchar=padchar
    };
}

/*! Creates a string representation for a ColorJustify.

    \details
    Allocates memory for the string representation.

    \pi cjust ColorJustify struct to get the representation for.
    \return   Allocated string for the representation.\n
              \mustfree

    \sa ColorJustify
*/
char* ColorJustify_repr(ColorJustify cjust) {
    char* meth_repr = ColorJustifyMethod_repr(cjust.method);
    char* pad_repr = char_repr(cjust.padchar);
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "ColorJustify {.method=%s, .width=%d, .padchar=%s}",
        meth_repr,
        cjust.width,
        pad_repr
    );
    free(meth_repr);
    free(pad_repr);
    return repr;
}

/*! Creates a string representation for a ColorJustifyMethod.

    \details
    Allocates memory for the string representation.

    \pi meth ColorJustifyMethod to get the representation for.
    \return  Allocated string for the representation.\n
             \mustfree

    \sa ColorJustifyMethod
*/
char* ColorJustifyMethod_repr(ColorJustifyMethod meth) {
    char* repr;
    switch (meth) {
        case JUST_NONE:
            asprintf_or_return(NULL, &repr, "JUST_NONE");
            break;
        case JUST_LEFT:
            asprintf_or_return(NULL, &repr, "JUST_LEFT");
            break;
        case JUST_RIGHT:
            asprintf_or_return(NULL, &repr, "JUST_RIGHT");
            break;
        case JUST_CENTER:
            asprintf_or_return(NULL, &repr, "JUST_CENTER");
            break;
    }
    return repr;
}

/*! Creates an "empty" ColorText with pointers set to `NULL`.

    \return An initialized ColorText.
*/
ColorText ColorText_empty(void) {
    return (ColorText){
        .marker=COLORTEXT_MARKER,
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .just=ColorJustify_empty(),
    };
}

/*! Frees a ColorText and it's ColorArgs.

    \details
    The text member is left alone, because it wasn't created by ColrC.

    \pi p Pointer to ColorText to free, along with it's Colr-based members.

    \sa ColorText
*/
void ColorText_free(ColorText *p) {
    if (!p) return;
    ColorArg_free(p->fore);
    ColorArg_free(p->back);
    ColorArg_free(p->style);

    free(p);
}

/*! Builds a ColorText from 1 mandatory string, and optional fore, back, and
    style args (pointers to ColorArgs).
    \pi text Text to colorize (a regular string).
    \pi ... ColorArgs for fore, back, and style, in any order.
    \return An initialized ColorText struct.

    \sa ColorText
*/
ColorText ColorText_from_values(char* text, ...) {
    // Argument list must have ColorArg with NULL members at the end.
    ColorText ctext = ColorText_empty();
    ctext.text = text;
    va_list colrargs;
    va_start(colrargs, text);

    ColorArg *arg;
    while ((arg = va_arg(colrargs, ColorArg*))) {
        assert(ColorArg_is_ptr(arg));
        // It's a ColorArg.
        if (arg->type == FORE) {
            ctext.fore = arg;
        } else if (arg->type == BACK) {
            ctext.back = arg;
        } else if (arg->type == STYLE) {
            ctext.style = arg;
        } else if (ColorArg_is_empty(*arg)) {
            // Empty ColorArgs are assigned in the order they were passed in.
            if (!ctext.fore) {
                ctext.fore = arg;
            } else if (!ctext.back) {
                ctext.back = arg;
            } else if (!ctext.style) {
                ctext.style = arg;
            }
        }
    }
    va_end(colrargs);
    return ctext;
}

/*! Checks to see if a ColorText has a certain ColorArg value set.

    \details
    Uses ColorArg_eq() to inspect the `fore`, `back`, and `style` members.

    \pi ctext The ColorText to inspect.
    \pi carg  The ColorArg to look for.
    \return   `true` if the `fore`, `back`, or `style` arg matches `carg`,
              otherwise `false`.
*/
bool ColorText_has_arg(ColorText ctext, ColorArg carg) {
    return (
        (ctext.fore && ColorArg_eq(*(ctext.fore), carg)) ||
        (ctext.back && ColorArg_eq(*(ctext.back), carg)) ||
        (ctext.style && ColorArg_eq(*(ctext.style), carg))
    );
}

/*! Checks to see if a ColorText has no usable values.

    \details
    A ColorText is considered "empty" if the `.text`, `.fore`, `.back`, and
    `.style` pointers are `NULL`, and the `.just` member is set to an "empty"
    ColorJustify.

    \pi ctext The ColorText to check.
    \return   `true` if the ColorText is empty, otherwise `false`.

    \sa ColorText ColorText_empty
*/
bool ColorText_is_empty(ColorText ctext) {
    return (
        !(ctext.text || ctext.fore || ctext.back || ctext.style) &&
        ColorJustify_is_empty(ctext.just)
    );
}
/*! Checks a void pointer to see if it contains a ColorText struct.

    \details The first member of a ColorText is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorText, otherwise `false`.

    \sa ColorText
*/
bool ColorText_is_ptr(void *p) {
    if (!p) return false;
    // The head of a ColorText is always a valid marker.
    ColorText *ctextp = p;
    return ctextp->marker == COLORTEXT_MARKER;
}

/*! Returns the length in bytes needed to allocate a string built with
    ColorText_to_str() with the current `text`, `fore`, `back`, and `style` members.

    \pi ctext ColorText to use.

    \return   The length (`size_t`) needed to allocate a ColorText's string,
              or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorText
*/
size_t ColorText_length(ColorText ctext) {
    // Empty text yields an empty string, so just "\0".
    if (!ctext.text) return 1;
    size_t length = strlen(ctext.text);
    if (ctext.fore) length+=ColorArg_length(*(ctext.fore));
    if (ctext.back) length+=ColorArg_length(*(ctext.back));
    if (ctext.style) length+=ColorArg_length(*(ctext.style));
    // If ctext.style == RESET_ALL is the only thing set, this will not
    // append the CODE_RESET_ALL (because ctext.style == CODE_RESET_ALL == 0).
    if (ctext.style || ctext.fore || ctext.back) length += CODE_RESET_LEN;
    if (!ColorJustify_is_empty(ctext.just)) {
        // Justification will be used, calculate that in.
        size_t noncode_len = str_noncode_len(ctext.text);
        if (ctext.just.width == 0) {
            // Go ahead and set the actual width, to reduce calls to colr_term_size().
            TermSize ts = colr_term_size();
            ctext.just.width = ts.columns;
        }
        int diff = ctext.just.width - noncode_len;
        // 0 or negative difference means no extra chars are added anyway.
        length += colr_max(0, diff);
    }
    // And the null-terminator.
    length++;
    return length;
}

/*! Allocate a string representation for a ColorText.

    \pi ctext ColorText to get the string representation for.
    \return Allocated string for the ColorText.

    \sa ColorText
*/
char* ColorText_repr(ColorText ctext) {
    char* repr;
    char* stext = ctext.text ? str_repr(ctext.text) : NULL;
    char* sfore = ctext.fore ? ColorArg_repr(*(ctext.fore)) : NULL;
    char* sback = ctext.back ? ColorArg_repr(*(ctext.back)) : NULL;
    char* sstyle = ctext.style ? ColorArg_repr(*(ctext.style)) : NULL;
    char* sjust = ColorJustify_repr(ctext.just);
    asprintf_or_return(
        NULL,
        &repr,
        "ColorText {.text=%s, .fore=%s, .back=%s, .style=%s, .just=%s}",
        stext ? stext : "NULL",
        sfore ? sfore : "NULL",
        sback ? sback : "NULL",
        sstyle ? sstyle : "NULL",
        sjust ? sjust : "<couldn't allocate for .just repr>"
    );
    free(stext);
    free(sfore);
    free(sback);
    free(sstyle);
    free(sjust);
    return repr;
}

/*! Set the ColorJustify method for a ColorText, and return the ColorText.

    \details
    This is to facilitate the justification macros. If you already have a pointer
    to a ColorText, you can just do `ctext->just = just;`. The purpose of this
    is to allow `ColorText_set_just(ColorText_to_ptr(...), ...)` to work.

    \po ctext The ColorText to set the justification method for.
    \pi cjust The ColorJustify struct to use.

    \return   The same pointer that was given as `ctext`.
*/
ColorText* ColorText_set_just(ColorText* ctext, ColorJustify cjust) {
    if (!ctext) return ctext;
    ctext->just = cjust;
    return ctext;
}

/*! Initializes an existing ColorText from 1 mandatory string, and optional
    fore, back, and style args (pointers to ColorArgs).

    \po ctext A ColorText to initialize with values.
    \pi text  Text to colorize (a regular string).
    \pi ...   A `va_list` with ColorArgs pointers for fore, back, and style, in any order.
    \return   An initialized ColorText struct.

    \sa ColorText
*/
void ColorText_set_values(ColorText* ctext, char* text, ...) {
    // Argument list must have ColorArg with NULL members at the end.
    *ctext = ColorText_empty();
    ctext->text = text;

    va_list colrargs;
    va_start(colrargs, text);

    ColorArg *arg;
    while ((arg = va_arg(colrargs, ColorArg*))) {
        assert(ColorArg_is_ptr(arg));
        // It's a ColorArg.
        if (arg->type == FORE) {
            ctext->fore = arg;
        } else if (arg->type == BACK) {
            ctext->back = arg;
        } else if (arg->type == STYLE) {
            ctext->style = arg;
        } else if (ColorArg_is_empty(*arg)) {
            // Empty ColorArgs are assigned in the order they were passed in.
            if (!ctext->fore) {
                ctext->fore = arg;
            } else if (!ctext->back) {
                ctext->back = arg;
            } else if (!ctext->style) {
                ctext->style = arg;
            }
        }
    }
    va_end(colrargs);
}
/*! Copies a ColorText into allocated memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi ctext ColorText to copy/allocate for.
    \return Pointer to a heap-allocated ColorText.

    \sa ColorText
*/
ColorText* ColorText_to_ptr(ColorText ctext) {
    size_t length = sizeof(ColorText);
    if (ctext.text) length += strlen(ctext.text) + 1;
    ColorText *p = malloc(length);
    ctext.marker = COLORTEXT_MARKER;
    *p = ctext;
    return p;
}

/*! Stringifies a ColorText struct.

    \details
    You must free() the resulting string.
    \pi ctext ColorText to stringify.
    \return An allocated string. _You must `free()` it_.

    \sa ColorText
*/
char* ColorText_to_str(ColorText ctext) {
    // No text? No string.
    if (!ctext.text) return colr_empty_str();
    // Make room for any fore/back/style code combo plus the reset_all code.
    char* final = calloc(ColorText_length(ctext), sizeof(char));
    bool do_reset = (ctext.style || ctext.fore || ctext.back);
    if (ctext.style && !ColorArg_is_empty(*(ctext.style))) {
        char* stylecode = ColorArg_to_str(*(ctext.style));
        strcat(final, stylecode);
        free(stylecode);
    }
    if (ctext.fore && !ColorArg_is_empty(*(ctext.fore))) {
        char* forecode = ColorArg_to_str(*(ctext.fore));
        strcat(final, forecode);
        free(forecode);
    }
    if (ctext.back && !ColorArg_is_empty(*(ctext.back))) {
        char* backcode = ColorArg_to_str(*(ctext.back));
        strcat(final, backcode);
        free(backcode);
    }
    strcat(final, ctext.text);
    if (do_reset) str_append_reset(final);
    char* justified = NULL;
    switch (ctext.just.method) {
        // TODO: It would be nice to do this all in one pass, but this works.
        case JUST_NONE:
            break;
        case JUST_LEFT:
            justified = str_ljust(final, ctext.just.padchar, ctext.just.width);
            free(final);
            return justified;
        case JUST_RIGHT:
            justified = str_rjust(final, ctext.just.padchar, ctext.just.width);
            free(final);
            return justified;
        case JUST_CENTER:
            justified = str_center(final, ctext.just.padchar, ctext.just.width);
            free(final);
            return justified;
    }
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

    \examplecodefor{ColorType_from_str,.c}
    #include "colr.h"

    int main(int argc, char** argv) {
        char* userarg;
        if (argc == 1) {
            if (asprintf(&userarg, "%s", "123,54,25") < 1) return 1;
        } else {
            if (asprintf(&userarg, "%s",  argv[1]) < 1) return 1;
        }
        ColorType type = ColorType_from_str(userarg);
        if (!ColorType_is_invalid(type)) {
            char* repr = colr_repr(type);
            printf("User passed in a %s, %s\n", repr, userarg);
            free(repr);
        } else {
            printf("User passed in an invalid color name: %s\n", userarg);
        }
        free(userarg);
    }
    \endexamplecode

    \sa ColorType
*/
ColorType ColorType_from_str(const char* arg) {
    if (!arg) return TYPE_INVALID;
    if (arg[0] == '\0') return TYPE_INVALID;
    // Try rgb first.
    unsigned char r, g, b;
    int rgb_ret = rgb_from_str(arg, &r, &g, &b);
    if (rgb_ret == COLOR_INVALID_RANGE) {
        return TYPE_INVALID_RGB_RANGE;
    } else if (rgb_ret != COLOR_INVALID) {
        return TYPE_RGB;
    }
    // Try basic colors.
    if (BasicValue_from_str(arg) != BASIC_INVALID) {
        return TYPE_BASIC;
    }
    // Try styles.
    if (StyleValue_from_str(arg) != STYLE_INVALID) {
        return TYPE_STYLE;
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(arg);
    if (x_ret == COLOR_INVALID_RANGE) {
        return TYPE_INVALID_EXTENDED_RANGE;
    } else if (x_ret != COLOR_INVALID) {
        return TYPE_EXTENDED;
    }
    return TYPE_INVALID;
}

/*! Check to see if a ColorType value is considered invalid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered invalid, otherwise `false`.

    \sa ColorType
*/
bool ColorType_is_invalid(ColorType type) {
    return !(bool_colr_enum(type));
}

/*! Check to see if a ColorType value is considered valid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered valid, otherwise `false`.

    \sa ColorType
*/
bool ColorType_is_valid(ColorType type) {
    return bool_colr_enum(type);
}

/*! Creates a string representation of a ColorType.

    \pi type A ColorType to get the type from.
    \return  A pointer to an allocated string.
             \mustfree

    \sa ColorType
*/
char* ColorType_repr(ColorType type) {
    char* typestr;
    switch (type) {
        case TYPE_NONE:
            asprintf_or_return(NULL, &typestr, "TYPE_NONE");
            break;
        case TYPE_BASIC:
            asprintf_or_return(NULL, &typestr, "TYPE_BASIC");
            break;
        case TYPE_EXTENDED:
            asprintf_or_return(NULL, &typestr, "TYPE_EXTENDED");
            break;
        case TYPE_RGB:
            asprintf_or_return(NULL, &typestr, "TYPE_RGB");
            break;
        case TYPE_STYLE:
            asprintf_or_return(NULL, &typestr, "TYPE_STYLE");
            break;
        case TYPE_INVALID:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID");
            break;
        case TYPE_INVALID_STYLE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_STYLE");
            break;
        case TYPE_INVALID_EXTENDED_RANGE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_EXTENDED_RANGE");
            break;
        case TYPE_INVALID_RGB_RANGE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_RGB_RANGE");
            break;
    }
    return typestr;
}

/*! Create an "empty" ColorValue.

    \details
    This is used with ColorArg_empty() to build ColorArgs that don't do anything,
    where using `NULL` has a different meaning inside the \colrmacros.

    \return `(ColorValue){.type=TYPE_NONE, .basic=0, .ext=0, .rgb=(RGB){0, 0, 0}}`

    \sa ColorArg ColorArg_empty ColorArg_is_empty ColorValue_is_empty
*/
ColorValue ColorValue_empty(void) {
    return (ColorValue){
        .type=TYPE_NONE,
        .basic=basic(0),
        .ext=ext(0),
        .rgb=rgb(0, 0, 0),
        .style=RESET_ALL,
    };
}

/*! Compares two ColorValue structs.
    \details
    They are considered "equal" if all of their members match.

    \pi a   First ColorValue to compare.
    \pi b   Second ColorValue to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_eq(ColorValue a, ColorValue b) {
    return (
        (a.type == b.type) &&
        (a.basic == b.basic) &&
        (a.ext == b.ext) &&
        (a.style == b.style) &&
        RGB_eq(a.rgb, b.rgb)
    );
}

/*! Create a ColorValue from a known color name, or RGB string.

    \pi s    A string to parse the color name from (can be an RGB string).
    \return  A ColorValue (with no fore/back information, only the color type and value).

    \sa ColorValue
*/
ColorValue ColorValue_from_str(char* s) {
    if (!s || s[0] == '\0') return ColorValue_from_value(TYPE_INVALID, NULL);

    // Get the actual type, even if it's invalid.
    ColorType type = ColorType_from_str(s);
    if (ColorType_is_invalid(type)) {
        return ColorValue_from_value(type, NULL);
    }
    // Try rgb first.
    RGB rgb;
    int rgb_ret = RGB_from_str(s, &rgb);
    if (rgb_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_RGB_RANGE, NULL);
    } else if (rgb_ret != TYPE_INVALID) {
        return ColorValue_from_value(type, &rgb);
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(s);
    if (x_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_EXTENDED_RANGE, NULL);
    } else if (x_ret != COLOR_INVALID) {
        // Need to cast back into a real ExtendedValue now that I know it's
        // not invalid. Also, ColorValue_from_value expects a pointer, to
        // help with it's "dynamic" uses.
        ExtendedValue xval = ext(x_ret);
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
    if ( b_ret != BASIC_INVALID) {
        BasicValue bval = (BasicValue)b_ret;
        return ColorValue_from_value(type, &bval);
    }
    return ColorValue_from_value(TYPE_INVALID, NULL);
}

/*! Used with the color_val macro to dynamically create a ColorValue based
    on it's argument type.

    \pi type A ColorType value, to mark the type of ColorValue.
    \pi p    A pointer to either a BasicValue, ExtendedValue, or a RGB.

    \return A ColorValue struct with the appropriate `.type` member set for
            the value that was passed. For invalid types the `.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXTENDED_RANGE
        - TYPE_INVALID_RGB_RANGE

    \sa ColorValue
*/
ColorValue ColorValue_from_value(ColorType type, void *p) {
    if (!p) {
        return (ColorValue){.type=TYPE_INVALID};
    }
    if (
        type == TYPE_INVALID ||
        type == TYPE_INVALID_EXTENDED_RANGE ||
        type == TYPE_INVALID_RGB_RANGE
        ) {
        return (ColorValue){.type=type};
    }
    if (type == TYPE_BASIC) {
        BasicValue *bval = p;
        BasicValue use_bval = *bval;
        // NONE has special meaning. It's not invalid, it's just "no preference".
        if (use_bval == BASIC_NONE) use_bval = basic(RESET);
        return (ColorValue){.type=TYPE_BASIC, .basic=use_bval};
    } else if (type == TYPE_EXTENDED) {
        ExtendedValue *eval = p;
        return (ColorValue){.type=TYPE_EXTENDED, .ext=*eval};
    } else if (type == TYPE_STYLE) {
        StyleValue *sval = p;
        ColorType ctype = (*sval == STYLE_INVALID) ? TYPE_INVALID_STYLE : TYPE_STYLE;
        return (ColorValue){.type=ctype, .style=*sval};
    } else if (type == TYPE_RGB) {
        RGB *rgbval = p;
        return (ColorValue){.type=TYPE_RGB, .rgb=*rgbval};
    }
    return (ColorValue){.type=type};
}

/*! Checks to see if a ColorValue has a BasicValue set.

    \pi cval ColorValue to check.
    \pi bval BasicValue to look for.

    \return `true` if the ColorValue has the exact BasicValue set.

    \sa ColorValue
*/
bool ColorValue_has_BasicValue(ColorValue cval, BasicValue bval) {
    return (cval.type == TYPE_BASIC) && (cval.basic == bval);
}

/*! Checks to see if a ColorValue has a ExtendedValue set.

    \pi cval ColorValue to check.
    \pi eval ExtendedValue to look for.

    \return `true` if the ColorValue has the exact ExtendedValue set.

    \sa ColorValue
*/
bool ColorValue_has_ExtendedValue(ColorValue cval, ExtendedValue eval) {
    return (cval.type == TYPE_EXTENDED) && (cval.ext == eval);
}

/*! Checks to see if a ColorValue has a StyleValue set.

    \pi cval ColorValue to check.
    \pi sval StyleValue to look for.

    \return `true` if the ColorValue has the exact StyleValue set.

    \sa ColorValue
*/
bool ColorValue_has_StyleValue(ColorValue cval, StyleValue sval) {
    return (cval.type == TYPE_STYLE) && (cval.style == sval);
}

/*! Checks to see if a ColorValue has a RGB value set.

    \pi cval   ColorValue to check.
    \pi rgbval RGB value to look for.

    \return    `true` if the ColorValue has the exact RGB value set.

    \sa ColorValue
*/
bool ColorValue_has_RGB(ColorValue cval, RGB rgbval) {
    return (cval.type == TYPE_RGB) && RGB_eq(cval.rgb, rgbval);
}

/*! Checks to see if a ColorValue is an empty placeholder.

    \pi cval ColorValue to check.
    \return  `true` if the ColorValue is "empty", otherwise `false`.

    \sa ColorValue ColorValue_empty ColorArg_empty ColorArg_is_empty
*/
bool ColorValue_is_empty(ColorValue cval) {
    return (cval.type == TYPE_NONE) || (
        (cval.basic == basic(0)) &&
        (cval.ext == ext(0)) &&
        RGB_eq(cval.rgb, rgb(0, 0, 0)) &&
        (cval.style == RESET_ALL)
    );
}

/*! Checks to see if a ColorValue holds an invalid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is invalid, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_is_invalid(ColorValue cval) {
    return !(bool_colr_enum(cval.type));
}

/*! Checks to see if a ColorValue holds a valid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is valid, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_is_valid(ColorValue cval) {
    return bool_colr_enum(cval.type);
}

/*! Returns the length in bytes needed to allocate a string built with
    ColorValue_to_str() with the specified ArgType and ColorValue.

    \pi type ArgType (`FORE`, `BACK`, `STYLE`)
    \pi cval ColorValue to use.

    \return  The length (`size_t`) needed to allocate a ColorValue's string,
             or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorValue
*/
size_t ColorValue_length(ArgType type, ColorValue cval) {
    switch (type) {
        case FORE:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                // This case is not valid, but I will try to do the right thing.
                case TYPE_STYLE:
                    return STYLE_LEN;
                default:
                    // Empty string for invalid/empty values.
                    return 1;
                }
        case BACK:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                // This case is not even valid, but okay.
                case TYPE_STYLE:
                    return STYLE_LEN;
                default:
                    // Empty string for invalid/empty values.
                    return 1;
                }
        case STYLE:
            assert(cval.type == TYPE_STYLE);
            switch (cval.type) {
                case TYPE_STYLE:
                    return STYLE_LEN;
                // All of these other cases are a product of mismatched info.
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                default:
                    // Empty string for invalid/empty values.
                    return 1;
            }
        default:
            // Empty string for invalid/empty arg type.
            return 1;
    }
    // Unreachable.
    return 1;
}
/*! Creates a string representation of a ColorValue.

    \pi cval    A ColorValue to get the type and value from.
    \return     A pointer to an allocated string.\n
                \mustfree

    \sa ColorValue
*/
char* ColorValue_repr(ColorValue cval) {
    switch (cval.type) {
        case TYPE_RGB:
            return RGB_repr(cval.rgb);
        case TYPE_BASIC:
            return BasicValue_repr(cval.basic);
        case TYPE_EXTENDED:
            return ExtendedValue_repr(cval.ext);
        case TYPE_STYLE:
            return StyleValue_repr(cval.style);
        default:
            return ColorType_repr(cval.type);
    }
}

/*! Converts a ColorValue into an escape code string.

    \details
    Memory is allocated for the string.
    \mustfree

    \pi type ArgType (FORE, BACK, STYLE) to build the escape code for.
    \pi cval ColorValue to get the color value from.

    \return  An allocated string with the appropriate escape code.
             For invalid values, an empty string is returned.

    \sa ColorValue
*/
char* ColorValue_to_str(ArgType type, ColorValue cval) {
    char* codes;
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
    \return BasicValue value on success, or BASIC_INVALID on error.

    \sa BasicValue
*/
BasicValue BasicValue_from_str(const char* arg) {
    if (!arg) {
        return BASIC_INVALID;
    }
    char* arglower = str_to_lower(arg);
    if (!arglower) return BASIC_INVALID;
    for (size_t i=0; i < basic_names_len; i++) {
        if (!strcmp(arglower, basic_names[i].name)) {
            free(arglower);
            return basic_names[i].value;
        }
    }
    free(arglower);
    return BASIC_INVALID;
}

/*! Creates a string representation of a BasicValue.

    \pi bval    A BasicValue to get the value from.
    \return     A pointer to an allocated string.\n
                \mustfree

    \sa BasicValue
*/
char* BasicValue_repr(BasicValue bval) {
    char* repr;
    switch (bval) {
        case BASIC_INVALID:
            asprintf_or_return(NULL, &repr, "(BasicValue) BASIC_INVALID");
            break;
        case BASIC_NONE:
            asprintf_or_return(NULL, &repr, "(BasicValue) BASIC_NONE");
            break;
        case BLACK:
            asprintf_or_return(NULL, &repr, "(BasicValue) BLACK");
            break;
        case RED:
            asprintf_or_return(NULL, &repr, "(BasicValue) RED");
            break;
        case GREEN:
            asprintf_or_return(NULL, &repr, "(BasicValue) GREEN");
            break;
        case YELLOW:
            asprintf_or_return(NULL, &repr, "(BasicValue) YELLOW");
            break;
        case BLUE:
            asprintf_or_return(NULL, &repr, "(BasicValue) BLUE");
            break;
        case MAGENTA:
            asprintf_or_return(NULL, &repr, "(BasicValue) MAGENTA");
            break;
        case CYAN:
            asprintf_or_return(NULL, &repr, "(BasicValue) CYAN");
            break;
        case WHITE:
            asprintf_or_return(NULL, &repr, "(BasicValue) WHITE");
            break;
        case UNUSED:
            asprintf_or_return(NULL, &repr, "(BasicValue) UNUSED");
            break;
        case RESET:
            asprintf_or_return(NULL, &repr, "(BasicValue) RESET");
            break;
        case LIGHTBLACK:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTBLACK");
            break;
        case LIGHTRED:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTRED");
            break;
        case LIGHTGREEN:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTGREEN");
            break;
        case LIGHTYELLOW:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTYELLOW");
            break;
        case LIGHTBLUE:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTBLUE");
            break;
        case LIGHTMAGENTA:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTMAGENTA");
            break;
        case LIGHTCYAN:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTCYAN");
            break;
        case LIGHTWHITE:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTWHITE");
            break;
        default:
            // Should never happen, but the value will be known if it does.
            asprintf_or_return(NULL, &repr, "(BasicValue) %d", bval);
    }
    return repr;
}

/*! Converts a fore/back BasicValue to the actual 4bit ansi code number.

    \pi type ArgType (FORE/BACK).
    \pi bval BasicValue to convert.
    \return An integer usable with basic escape code fore/back colors.

    \sa BasicValue
*/
int BasicValue_to_ansi(ArgType type, BasicValue bval) {
    int use_value = (int)bval;
    if (bval < 0) {
        // Invalid or NONE, just use the RESET code.
        return (int)RESET;
    }
    if (bval < 10) {
        // Normal back colors.
        return use_value + (type == BACK ? 40 : 30);
    }
    // Bright back colors.
    return use_value + (type == BACK ? 90 : 80);
}

/*! Create an ExtendedValue from a hex string.

    \details
    This is not a 1:1 translation of hex to rgb. Use RGB_from_hex() for that.
    This will convert the hex string to the closest matching ExtendedValue value.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr Hex string to convert.
    \return    A value between 0 and 255 on success.
    \retval    COLOR_INVALID on error or bad values.

    \sa ExtendedValue
*/
int ExtendedValue_from_hex(const char* hexstr) {
    RGB rgb;
    if (RGB_from_hex(hexstr, &rgb) != 0) return COLOR_INVALID;
    return ExtendedValue_from_RGB(rgb);
}

/*! Create an ExtendedValue from a hex string, but return a default value if
    the hex string is invalid.

    \details
    This is not a 1:1 translation of hex to rgb. Use RGB_from_hex_default() for that.
    This will convert the hex string to the closest matching ExtendedValue value.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr         Hex string to convert.
    \pi default_value  ExtendedValue to use for bad hex strings.
    \return            An ExtendedValue on success, or `default_value` on error.

    \sa ExtendedValue
    \sa ExtendedValue_from_hex
*/
ExtendedValue ExtendedValue_from_hex_default(const char* hexstr, ExtendedValue default_value) {
    int ret = ExtendedValue_from_hex(hexstr);
    if (ret < 0) return default_value;
    return ret;
}

/*! Convert an RGB value into the closest matching ExtendedValue.

    \pi rgb RGB value to convert.
    \return An ExtendedValue that closely matches the original RGB value.

    \sa ExtendedValue
*/
ExtendedValue ExtendedValue_from_RGB(RGB rgb) {
    RGB closestrgb = RGB_to_term_RGB(rgb);
    for (size_t i = 0; i < ext2rgb_map_len; i++) {
        if (RGB_eq(closestrgb, ext2rgb_map[i])) {
            return ext(i);
        }
    }
    // Should never happen.
    return ext(0);
}

/*! Converts a known name, integer string (0-255), or a hex string, into an
    ExtendedValue suitable for the extended-value-based functions.

    \details
    Hex strings can be used:
        - "#ffffff" (Leading hash symbol is __NOT__ optional)
        - "#fff" (short-form)

    \details
    The `'#'` is not optional for hex strings because it is impossible to tell
    the difference between the hex value '111' and the extended value '111'
    without it.

    \pi arg Color name to find the ExtendedValue for.

    \return A value between 0 and 255 on success.
    \retval COLOR_INVALID on error or bad values.

    \sa ExtendedValue
*/
int ExtendedValue_from_str(const char* arg) {
    if (!arg) return COLOR_INVALID;
    if (arg[0] == '\0') return COLOR_INVALID;

    char* arglower = str_to_lower(arg);
    if (!arglower) return COLOR_INVALID;
    for (size_t i=0; i < extended_names_len; i++) {
        if (!strcmp(arglower, extended_names[i].name)) {
            // A known extended value name.
            free(arglower);
            return extended_names[i].value;
        }
    }
    if (arglower[0] == '#') {
        // Possibly a hex string.
        int hex_ret = ExtendedValue_from_hex(arglower);
        if (hex_ret >= 0) {
            // A valid hex string.
            return hex_ret;
        }
    }
    if (!str_is_digits(arg)) {
        if ((arg[0] == '-') && (strlen(arg) > 1) && str_is_digits(arg + 1)) {
            free(arglower);
            // Negative number given.
            return COLOR_INVALID_RANGE;
        }
        // Not a number at all.
        free(arglower);
        return COLOR_INVALID;
    }

    // Regular number, hopefully 0-255, but I'll check that in a second.
    size_t length = strnlen(arglower, 4);
    if (length > 3) {
        // Definitely not 0-255.
        free(arglower);
        return COLOR_INVALID_RANGE;
    }
    short int usernum;
    if (sscanf(arg, "%hd", &usernum) != 1) {
        // Zero, or more than one number provided.
        free(arglower);
        return COLOR_INVALID;
    }
    if (usernum < 0 || usernum > 255) {
        free(arglower);
        return COLOR_INVALID_RANGE;
    }
    // A valid number, 0-255.
    free(arglower);
    return (int)usernum;
}

/*! Creates a string representation of a ExtendedValue.

    \pi eval    A ExtendedValue to get the value from.
    \return     A pointer to an allocated string.\n
                \mustfree

    \sa ExtendedValue
*/
char* ExtendedValue_repr(int eval) {
    char* repr;
    switch (eval) {
        case COLOR_INVALID_RANGE:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) COLOR_INVALID_RANGE");
            break;
        case COLOR_INVALID:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) COLOR_INVALID");
            break;
        default:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) %d", eval);
    }
    return repr;
}

/*! Creates a string from an ExtendedValue's actual value, suitable for use
    with ExtendedValue_from_str().

    \pi eval    A ExtendedValue to get the value from.
    \return     A pointer to an allocated string, or `NULL` if the allocation fails.\n
                \mustfree

    \sa ExtendedValue
*/
char* ExtendedValue_to_str(ExtendedValue eval) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%d", eval);
    return repr;
}

/*! Convert a hex color into separate red, green, blue values.
    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \details
    Three-digit numbers are valid hex strings, so values like `011` are accepted
    and transformed into `#001111`.

    \pi hexstr String to convert into red, green, blue values.
               \mustnullin
    \po r      Pointer to an unsigned char for red value on success.
    \po g      Pointer to an unsigned char for green value on success.
    \po b      Pointer to an unsigned char for blue value on success.

    \retval 0 on success, with \p r, \p g, and \p b filled with the values.
    \retval COLOR_INVALID for non-hex strings.
*/
int rgb_from_hex(const char* hexstr, unsigned char* r, unsigned char* g, unsigned char* b) {
    if (!hexstr) return COLOR_INVALID;
    size_t length = strnlen(hexstr, 7);
    if ((length < 3) || (length > 7)) return COLOR_INVALID;
    // Strip leading #'s.
    char copy[] = "\0\0\0\0\0\0\0\0";
    inline_str_lstrip_char(copy, hexstr, length, '#');
    size_t copy_length = strlen(copy);
    if (copy_length < length - 1) {
        // There was more then one # symbol, I'm not gonna be *that* nice.
        return COLOR_INVALID;
    }
    unsigned int redval, greenval, blueval;
    switch (copy_length) {
        case 3:
            // Even though the `strlen()` is 3, there is room for 7.
            copy[5] = copy[2];
            copy[4] = copy[2];
            copy[3] = copy[1];
            copy[2] = copy[1];
            copy[1] = copy[0];
            /* fall through */
        case 6:
            if (sscanf(copy, "%02x%02x%02x", &redval, &greenval, &blueval) != 3) {

                return COLOR_INVALID;
            }
            break;
        default:
            // Not a valid length.
            return COLOR_INVALID;
    }

    *r = redval;
    *g = greenval;
    *b = blueval;
    return 0;
}

/*! Convert an RGB string into separate red, green, blue values.

    \details
    The format for RGB strings can be one of:
        - "RED,GREEN,BLUE"
        - "RED GREEN BLUE"
        - "RED:GREEN:BLUE"
        - "RED;GREEN;BLUE"
    Or hex strings can be used:
        - "#ffffff" (Leading hash symbol is __NOT__ optional)
        - "#fff" (short-form)

    \pi arg String to check for RGB values.
    \po r   Pointer to an unsigned char for red value on success.
    \po g   Pointer to an unsigned char for green value on success.
    \po b   Pointer to an unsigned char for blue value on success.

    \retval 0 on success, with \p r, \p g, and \p b filled with the values.
    \retval COLOR_INVALID for non-rgb strings.
    \retval COLOR_INVALID_RANGE for rgb values outside of 0-255.
*/
int rgb_from_str(const char* arg, unsigned char* r, unsigned char* g, unsigned char* b) {
    if (!arg) return COLOR_INVALID;
    const char* formats[] = {
        "%ld,%ld,%ld",
        "%ld %ld %ld",
        "%ld:%ld:%ld",
        "%ld;%ld;%ld",
        NULL
    };
    long userred, usergreen, userblue;
    int i = 0;
    while (formats[i]) {
        if (sscanf(arg, formats[i], &userred, &usergreen, &userblue) == 3) {
            // Found a match.
            if (userred < 0 || userred > 255) return COLOR_INVALID_RANGE;
            if (usergreen < 0 || usergreen > 255) return COLOR_INVALID_RANGE;
            if (userblue < 0 || userblue > 255) return COLOR_INVALID_RANGE;
            // Valid ranges, set values for out parameters.
            *r = (unsigned char)userred;
            *g = (unsigned char)usergreen;
            *b = (unsigned char)userblue;
            return 0;
        }
        i++;
    }
    return arg[0] == '#' ? rgb_from_hex(arg, r, g, b) : COLOR_INVALID;
}

/*! Compare two RGB structs.

    \pi a First RGB value to check.
    \pi b Second RGB value to check.
    \return `true` if \p a and \p b have the same `r`, `g`, and `b` values, otherwise `false`.

    \sa RGB
*/
bool RGB_eq(RGB a, RGB b) {
    return (
        (a.red == b.red) &&
        (a.green == b.green) &&
        (a.blue == b.blue)
    );
}

/*! Convert a hex color into an RGB value.
    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi arg String to check for RGB values.
            \mustnullin
    \po rgb Pointer to an RGB struct to fill in the values for.

    \retval 0 on success, with \p rgbval filled with the values.
    \retval COLOR_INVALID for non-hex strings.

    \sa RGB
*/
int RGB_from_hex(const char* arg, RGB *rgb) {
    if (!arg) return COLOR_INVALID;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    int ret = rgb_from_hex(arg, &r, &g, &b);
    if (ret) {
        // An error occurred.
        return ret;
    }
    rgb->red = r;
    rgb->green = g;
    rgb->blue = b;
    return 0;
}

/*! Convert a hex color into an RGB value, but use a default value when errors
    occur.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi arg           String to check for RGB values.
                      \mustnullin
    \po default_value An RGB value to use when errors occur.

    \return           A valid RGB value on success, or `default_value` on error.

    \sa RGB
    \sa hex
*/
RGB RGB_from_hex_default(const char* arg, RGB default_value) {
    if (!arg) return default_value;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    int ret = rgb_from_hex(arg, &r, &g, &b);
    if (ret) {
        // An error occurred.
        return default_value;
    }
    return rgb(r, g, b);
}
/*! Convert an RGB string into an RGB value.

    \details
    The format for RGB strings can be one of:
        - "RED,GREEN,BLUE"
        - "RED GREEN BLUE"
        - "RED:GREEN:BLUE"
        - "RED;GREEN;BLUE"
    Or hex strings can be used:
        - "#ffffff" (Leading hash symbol is __NOT__ optional)
        - "#fff" (short-form)

    \pi arg    String to check for RGB values.
               \mustnullin
    \po rgbval Pointer to an RGB struct to fill in the values for.

    \retval 0 on success, with \p rgbval filled with the values.
    \retval COLOR_INVALID for non-rgb strings.
    \retval COLOR_INVALID_RANGE for rgb values outside of 0-255.

    \examplecodefor{RGB_from_str,.c}
    RGB rgbval;
    int ret = RGB_from_str("123,0,234", &rgbval);
    if (ret != COLOR_INVALID) {
        char* s = colr(Colr("Test", fore(rgbval)));
        printf("%s\n", s);
        free(s);
    }
    \endexamplecode

    \sa RGB
*/
int RGB_from_str(const char* arg, RGB *rgbval) {
    if (!arg) return COLOR_INVALID;
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

/*! Converts an RGB value into a hex string.

    \pi rgb RGB value to convert.
    \return An allocated string.
            Returns `NULL` if the allocation failed.\n
            \mustfree

    \sa RGB
*/
char* RGB_to_hex(RGB rgb) {
    char* s;
    asprintf_or_return(NULL, &s, "%02x%02x%02x", rgb.red, rgb.green, rgb.blue);
    return s;
}

/*! Convert an RGB value into an RGB string suitable for input to RGB_from_str().

    \pi rgb RGB value to convert.
    \return An allocated string in the form `"red;green;blue"`.
            Returns `NULL` if the allocation failed.\n
            \mustfree
*/
char* RGB_to_str(RGB rgb) {
    char* s;
    asprintf_or_return(NULL, &s, "%03d;%03d;%03d", rgb.red, rgb.green, rgb.blue);
    return s;
}

/*! Convert an RGB value into it's nearest terminal-friendly RGB value.
    \details
    This is a helper for the 'to_term' functions.

    \pi rgb RGB to convert.
    \return A new RGB with values close to a terminal code color.

    \sa RGB
*/
RGB RGB_to_term_RGB(RGB rgb) {
    int incs[6] = {0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
    size_t inc_len = sizeof(incs) / sizeof(incs[0]);
    size_t inc_max = inc_len -1 ;
    unsigned char res[3] = {-1, -1, -1};
    size_t res_pos = 0;
    unsigned char parts[3] = {rgb.red, rgb.blue, rgb.green};
    size_t part_len = sizeof(parts) / sizeof(parts[0]);
    for (size_t part_num = 0; part_num < part_len; part_num++) {
        unsigned char part = parts[part_num];
        for (size_t inc_num = 0; inc_num < inc_max; inc_num++) {
            int s = incs[inc_num]; // smaller
            int b = incs[inc_num + 1]; // bigger
            if ((s <= part) && (part <= b)) {
                int s1 = abs(s - part);
                int b1 = abs(b - part);
                unsigned char closest = b;
                if (s1 < b1) closest = s;
                res[res_pos] = closest;
                res_pos++;
                break;
            }
        }
    }
    // Convert back into nearest hex value.
    return (RGB){.red=res[0], .blue=res[1], .green=res[2]};
}


/*! Creates a string representation for an RGB value.
    \details
    Allocates memory for the string representation.

    \pi rgb RGB struct to get the representation for.
    \return Allocated string for the representation.\n
            \mustfree

    \sa RGB
*/
char* RGB_repr(RGB rgb) {
    char* repr;
    asprintf_or_return(NULL,
        &repr,
        "RGB {.red=%d, .green=%d, .blue=%d}",
        rgb.red,
        rgb.green,
        rgb.blue
    );
    return repr;
}

/*! Convert a named argument to actual StyleValue enum value.

    \pi arg Style name to convert into a StyleValue.
    \return A usable StyleValue value on success, or STYLE_INVALID on error.

    \sa StyleValue
*/
StyleValue StyleValue_from_str(const char* arg) {
    if (!arg) {
        return STYLE_INVALID;
    }
    char* arglower = str_to_lower(arg);
    if (!arglower) return STYLE_INVALID;
    for (size_t i=0; i < style_names_len; i++) {
        if (!strcmp(arglower, style_names[i].name)) {
            free(arglower);
            return style_names[i].value;
        }
    }
    free(arglower);
    return STYLE_INVALID;
}

/*! Creates a string representation of a StyleValue.

    \pi sval    A StyleValue to get the value from.
    \return     A pointer to an allocated string.\n
                \mustfree

    \sa StyleValue
*/
char* StyleValue_repr(StyleValue sval) {
    char* repr;
    switch (sval) {
        case STYLE_INVALID:
            asprintf_or_return(NULL, &repr, "(StyleValue) STYLE_INVALID");
            break;
        case STYLE_NONE:
            asprintf_or_return(NULL, &repr, "(StyleValue) STYLE_NONE");
            break;
        case RESET_ALL:
            asprintf_or_return(NULL, &repr, "(StyleValue) RESET_ALL");
            break;
        case BRIGHT:
            asprintf_or_return(NULL, &repr, "(StyleValue) BRIGHT");
            break;
        case DIM:
            asprintf_or_return(NULL, &repr, "(StyleValue) DIM");
            break;
        case ITALIC:
            asprintf_or_return(NULL, &repr, "(StyleValue) ITALIC");
            break;
        case UNDERLINE:
            asprintf_or_return(NULL, &repr, "(StyleValue) UNDERLINE");
            break;
        case FLASH:
            asprintf_or_return(NULL, &repr, "(StyleValue) FLASH");
            break;
        case HIGHLIGHT:
            asprintf_or_return(NULL, &repr, "(StyleValue) HIGHLIGHT");
            break;
        case NORMAL:
            asprintf_or_return(NULL, &repr, "(StyleValue) NORMAL");
            break;
        case STRIKETHRU:
            asprintf_or_return(NULL, &repr, "(StyleValue) STRIKETHRU");
            break;
        case FRAME:
            asprintf_or_return(NULL, &repr, "(StyleValue) FRAME");
            break;
        case ENCIRCLE:
            asprintf_or_return(NULL, &repr, "(StyleValue) ENCIRCLE");
            break;
        case OVERLINE:
            asprintf_or_return(NULL, &repr, "(StyleValue) OVERLINE");
            break;
        default:
            // Should never happen, but at least the value will be known
            // if it does.
            asprintf_or_return(NULL, &repr, "(StyleValue) %d", sval);
    }
    return repr;
}

/*! Rainbow-ize some text using rgb back colors, lolcat style.

    \details
    This prepends a color code to every character in the input string.
    To handle multibyte characters, the string is first converted to
    `wchar_t*`. The end result is converted back into a regular `char*` string.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      The string to colorize.
               \mustnullin
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.\n
               \mustfree
               If the allocation fails, `NULL` is returned.
*/
char* rainbow_bg(const char* s, double freq, size_t offset) {
    return _rainbow(format_bg_RGB, s, freq, offset);
}

/*! This is exactly like rainbow_bg(), except it uses colors that are
    closer to the standard 256-color values.

    \details
    This prepends a color code to every character in the input string.
    To handle multibyte characters, the string is first converted to
    `wchar_t*`. The end result is converted back into a regular `char*` string.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      The string to colorize.
               \mustnullin
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.\n
               \mustfree
               If the allocation fails, `NULL` is returned.
*/
char* rainbow_bg_term(const char* s, double freq, size_t offset) {
    return _rainbow(format_bg_RGB_term, s, freq, offset);
}

/*! Rainbow-ize some text using rgb fore colors, lolcat style.

    \details
    This prepends a color code to every character in the input string.
    To handle multibyte characters, the string is first converted to
    `wchar_t*`. The end result is converted back into a regular `char*` string.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      The string to colorize.
               \mustnullin
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.\n
               \mustfree
               If the allocation fails, `NULL` is returned.
*/
char* rainbow_fg(const char* s, double freq, size_t offset) {
    return _rainbow(format_fg_RGB, s, freq, offset);
}

/*! This is exactly like rainbow_fg(), except it uses colors that are
    closer to the standard 256-color values.

    \details
    This prepends a color code to every character in the input string.
    To handle multibyte characters, the string is first converted to
    `wchar_t*`. The end result is converted back into a regular `char*` string.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      The string to colorize.
               \mustnullin
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \return    The allocated/formatted string on success.\n
               \mustfree
               If the allocation fails, `NULL` is returned.
*/
char* rainbow_fg_term(const char* s, double freq, size_t offset) {
    return _rainbow(format_fg_RGB_term, s, freq, offset);
}

/*! Handles wide character string conversion and character iteration for
    all of the rainbow_ functions.

    \pi fmter  A formatter function (RGB_fmter) that can create escape codes
               from RGB values.
    \pi s      The string to "rainbowize".
               \mustnullin
    \pi freq   The "tightness" for colors.
    \pi offset The starting offset into the rainbow.

    \return    An allocated string (`char*`) with the result.\n
               \mustfree
*/
char* _rainbow(RGB_fmter fmter, const char* s, double freq, size_t offset) {
    if (!s) {
        return NULL;
    }
    if (!offset) offset = 3;
    if (freq < 0.1) freq = 0.1;

    size_t mb_len = str_mb_len(s);
    if (mb_len == 0) return NULL;

    // There is an RGB code for every wide character in the string.
    size_t total_size = mb_len + (CODE_RGB_LEN * mb_len);
    char* out = calloc(total_size, sizeof(char));
    if (!out) return NULL;

    char codes[CODE_RGB_LEN];
    // Enough room for one (possibly multi-byte) character.
    size_t mb_char_len = 6;
    char mb_char[mb_char_len + 1];
    // Iterate over each multi-byte character.
    size_t i = 0;
    int char_len = 0;
    while ((char_len = mblen(s + i, mb_char_len))) {
        fmter(codes, rainbow_step(freq, offset + i));
        strcat(out, codes);
        snprintf(mb_char, char_len + 1, "%s", s + i);
        strcat(out, mb_char);
        i += char_len;
    }
    strcat(out, CODE_RESET_ALL);

    return out;
}

/*! A single step in rainbow-izing produces the next color in the "rainbow" as
    an RGB value.

    \pi freq Frequency ("tightness") of the colors.
    \pi offset Starting offset in the rainbow.

    \return  An RGB value with the next "step" in the "rainbow".
*/
RGB rainbow_step(double freq, size_t offset) {
    /*  A note about the libm (math.h) dependency:

        libm's sin() function works on every machine, gives better results
        than any hand-written sin() that I've found, and is faster.

        Something like this produces ugly "steps" in the rainbow, and `i`
        would have to be large enough for the entire string that is being
        "rainbowized". Just keep libm:
            float sin(float x) {
                float res = 0, pow = x, fact = 1;
                for(int i = 0; i < 5; ++i) {
                    res += pow / fact;
                    pow *= -1 * x * x;
                    fact *= (2 * (i + 1)) * (2 * (i + 1) + 1);
                }

                return res;
            }
    */
    double redval = sin(freq * offset + 0) * 127 + 128;
    double greenval = sin(freq * offset + 2 * M_PI / 3) * 127 + 128;
    double blueval = sin(freq * offset + 4 * M_PI / 3) * 127 + 128;
    return rgb(redval, greenval, blueval);
}
