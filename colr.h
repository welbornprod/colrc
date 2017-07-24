#ifndef _COLORS_H_
#pragma GCC diagnostic ignored "-Wunused-macros"

#define _COLORS_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define COLR_VERSION "0.2.0"

#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef M_PI
	// For the rainbow functions.
	#define M_PI (3.14159265358979323846)
#endif

/* TODO: The extended colors should be easier to mix with regular colors.
		 e.g.: color(RED, colorbg(BLUE, style(BRIGHT, "test")))
		   or: color("test", RED || 255, BLUE || 255, BRIGHT)
*/
typedef enum Colors_t {
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

} Colors;

typedef enum Styles_t {
	STYLE_INVALID = -2,
	STYLE_NONE = -1,
	RESET_ALL = 0,
	BRIGHT = 1,
	DIM = 2,
	ITALIC = 3,
	UNDERLINE = 4,
	FLASH = 5,
	HIGHLIGHT = 6,
	NORMAL = 7
} Styles;

union Codes {
	Colors colors;
	unsigned char num;
};

// Style code to reset all styling.
extern const char *STYLE_RESET_ALL;


// Convenience const, because this is used a lot.
const char *STYLE_RESET_ALL = "\033[0m";

// Maximum length for a basic fore/back escape code, including '\0'.
const size_t CODE_LEN = 10;
// Maximum length for an extended fore/back escape code, including '\0'.
const size_t CODEX_LEN = 15;

// Maximum length for a style escape code, including '\0'.
const size_t STYLE_LEN = 8;

// Maximum length in chars for any combination of basic/extended escape codes.
// Should be (CODEX_LEN * 2) + STYLE_LEN.
// Allocating for a string that will be colorized must account for this.
const size_t COLOR_LEN = 40;

// Maximum length in chars for an RGB fore/back escape code.
const size_t CODE_RGB_LEN = 23;

// Maximim string length for a fore, back, or style name.
const size_t MAX_NAME_LEN = 10;

void format_fore(char*, Colors);
void format_forex(char*, unsigned char);
void format_fore_rgb(char*, unsigned char, unsigned char, unsigned char);
void format_bg(char*, Colors);
void format_bgx(char*, unsigned char);
void format_bg_rgb(char*, unsigned char, unsigned char, unsigned char);
void format_rainbow_fore(char*, double, size_t);
void format_style(char*, Styles);
void str_tolower(char *out, char *s);

/* ------------------------------- Functions ------------------------------ */

void
colrbg(char *out, char *s, Colors back) {
	/*  Prepend back color codes to a string, and copy the result into `out`.
		The STYLE_RESET_ALL code is already appended to the result.
		Arguments:
			out  : Memory allocated for the result.
				   *Must have enough room for `strlen(s) + COLOR_LEN`.
			s    : The string to colorize.
				   *Must be null-terminated.
			back : `Colors` code to use.
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
colrbgrgb(char *out, char *s, unsigned char red, unsigned char green, unsigned char blue) {
	/*  Colorize a string using true color, rgb back colors and copy the
		result into `out`.
		The Styles.RESET_ALL code is already appended to the result.
		Arguments:
			out   : Allocated memory to copy the result to.
				    *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s     : String to colorize.
		    	    *Must be null-terminated.
		    red   : Value for red.
		    green : Value for green.
		    blue  : Value for blue.
	*/
	char forecode[CODE_RGB_LEN];
	format_bg_rgb(out, red, green, blue);
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
colrbgx(char *out, char *s, unsigned char num) {
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
colrfore(char *out, char *s, Colors fore) {
	/*  Prepend fore color codes to a string, and copy the result into `out`.
		The STYLE_RESET_ALL code is already appended to the result.
		Arguments:
			out  : Memory allocated for the result.
				   *Must have enough room for `strlen(s) + COLOR_LEN`.
			s    : The string to colorize.
				   *Must be null-terminated.
			fore : `Colors` code to use.
	*/
	char forecode[CODEX_LEN];
	format_fore(forecode, fore);
	size_t oldlen = strlen(s);
	size_t codeslen = CODEX_LEN + STYLE_LEN;
	snprintf(
		out,
		oldlen + codeslen,
		"%s%s%s", forecode, s, STYLE_RESET_ALL
	);
}

void
colrforerainbow(char *out, char *s, double freq, size_t offset) {
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
	out[0] = '\0';
	// Enough room for the escape code and one character.
	char codes[CODE_RGB_LEN];
	size_t singlecharlen = CODE_RGB_LEN + 1;
	char singlechar[singlecharlen];
	size_t oldlen = strlen(s);
	for (size_t i = 0; i < oldlen; i++) {
		format_rainbow_fore(codes, freq, offset + i);
		snprintf(singlechar, singlecharlen, "%s%c", codes, s[i]);
		strncat(out, singlechar, CODE_RGB_LEN);
	}
	strncat(out, STYLE_RESET_ALL, STYLE_LEN);
}

void
colrforergb(char *out, char *s, unsigned char red, unsigned char green, unsigned char blue) {
	/*  Colorize a string using true color, rgb fore colors and copy the
		result into `out`.
		The Styles.RESET_ALL code is already appended to the result.
		Arguments:
			out   : Allocated memory to copy the result to.
				    *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s     : String to colorize.
		    	    *Must be null-terminated.
		    red   : Value for red.
		    green : Value for green.
		    blue  : Value for blue.
	*/
	char forecode[CODE_RGB_LEN];
	format_fore_rgb(out, red, green, blue);
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
colrforex(char *out, char *s, unsigned char num) {
	/*  Colorize a string using extended, 256 colors, and copy the
		result into `out`.
		The Styles.RESET_ALL code is already appended to the result.
		Arguments:
			out  : Allocated memory to copy the result to.
				   *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s    : String to colorize.
		    	   *Must be null-terminated.
		    num  : Code number, 0-255 inclusive.
	*/
	char forecode[CODEX_LEN];
	format_forex(forecode, num);
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
colrize(char *out, char *s, Colors fore, Colors back, Styles style) {
	/*  Prepends escape codes for fore, back, and style to `s` and copies
		the result into `out`.
		Arguments:
			out       : Allocated memory to copy the result to.
				        *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s         : String to colorize.
		    	        *Must be null-terminated.
		    fore      : Colors value to use for fore.
		    back      : Colors value to use for background.
		    style : Styles value to use.
	*/

	// Build forecolor only.
	char forecode[CODEX_LEN];
	format_fore(forecode, fore);
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
	strncat(fore == RESET ? frontcodes: endcodes, forecode, CODEX_LEN);
	strncat(back == RESET ? frontcodes: endcodes, backcode, CODEX_LEN);
	strncat(style == RESET_ALL ? frontcodes: endcodes, stylecode, STYLE_LEN);

	size_t oldlen = strlen(s);
	codeslen =  strlen(frontcodes) + strlen(endcodes) + STYLE_LEN;
	snprintf(
		out,
		oldlen + codeslen,
		"%s%s%s%s",
		frontcodes, endcodes, s, STYLE_RESET_ALL
	);
}

char*
acolrize(char *s, Colors fore, Colors back, Styles style) {
	/*  Prepends escape codes for fore, back, and style to `s` and copies
		the result into an allocated string.
		The string must be freed by the caller.
		Arguments:
			out   : Allocated memory to copy the result to.
				    *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s     : String to colorize.
		    	    *Must be null-terminated.
		    fore  : Colors value to use for fore.
		    back  : Colors value to use for background.
		    style : Styles value to use.
	*/
	char *out = malloc(sizeof(char) * (strlen(s) + COLOR_LEN));
	colrize(out, s, fore, back, style);
	return out;
}

/* TODO: colrizergb, to set the fore/back using rgb codes. Style is the same.
*/

void
colrizex(
	char *out,
	char *s,
	unsigned char forenum, unsigned char backnum, Styles style) {
	/*  Prepends escape codes for extended fore, back, and style to `s` and
		copies the result into an allocated string.
		The string must be freed by the caller.
		Arguments:
			out       : Allocated memory to copy the result to.
				        *Must have enough room for `strlen(s) + COLOR_LEN`.
		    s         : String to colorize.
		    	        *Must be null-terminated.
		    fore      : Colors value to use for fore.
		    back      : Colors value to use for background.
		    style : Styles value to use.
	*/
	// Build forecolor only.
	char forecode[CODEX_LEN];
	format_forex(forecode, forenum);
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
	strncat(frontcodes, forecode, CODEX_LEN);
	strncat(frontcodes, backcode, CODEX_LEN);
	strncat(
		(style == RESET_ALL ? frontcodes: endcodes),
		stylecodes,
		STYLE_LEN
	);

	size_t oldlen = strlen(s);
	codeslen =  strlen(frontcodes) + strlen(endcodes) + STYLE_LEN;
	snprintf(
		out,
		oldlen + codeslen,
		"%s%s%s%s",
		frontcodes, endcodes, s, STYLE_RESET_ALL
	);
}

char*
acolrizex(
		char *s,
		unsigned char forenum, unsigned char backnum, Styles stylecode) {
	/*  Prepends escape codes for extended fore, back, and style to `s` and
		copies the result into an allocated string.
		The string must be freed by the caller.
		Arguments:
		    s         : String to colorize.
		    	        *Must be null-terminated.
		    fore      : Colors value to use for fore.
		    back      : Colors value to use for background.
		    stylecode : Styles value to use.
	*/
	char *out = malloc(sizeof(char) * (strlen(s) + COLOR_LEN));
	colrizex(out, s, forenum, backnum, stylecode);
	return out;
}

Colors
colorname_to_color(char *arg) {
    /*  Convert named argument to actual Colors enum value.
        Returns a Colors value on success, or COLOR_INVALID on error.
    */
    char arglower[MAX_NAME_LEN];
    str_tolower(arglower, arg);

    if (!strcmp(arglower, "none")) return COLOR_NONE;
    if (!strcmp(arglower, "black")) return BLACK;
    if (!strcmp(arglower, "red")) return RED;
    if (!strcmp(arglower, "green")) return GREEN;
    if (!strcmp(arglower, "yellow")) return YELLOW;
    if (!strcmp(arglower, "blue")) return BLUE;
    if (!strcmp(arglower, "magenta")) return MAGENTA;
    if (!strcmp(arglower, "cyan")) return CYAN;
    if (!strcmp(arglower, "white")) return WHITE;
    if (!strcmp(arglower, "reset")) return RESET;
    return COLOR_INVALID;
}

void
colrstyle(char *out, Styles style, char *s) {
	/*  Prepend style codes to a string, and copy the result into `out`.
		The STYLE_RESET_ALL code is already appended to the result.
		Arguments:
			out   : Memory allocated for the result.
				    *Must have enough room for `strlen(s) + COLOR_LEN`.
			s     : The string to colorize.
				    *Must be null-terminated.
			style : `Styles` code to use.
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

inline void
format_bg(char *out, Colors value) {
	/*  Create an escape code for a background color.
		Arguments:
			out   : Memory allocated for the escape code string.
				    *Must have enough room for `CODEX_LEN`.
		    value : Colors value to use for background.
	*/
	if (value > 9) {
		format_bgx(out, value - (value > 16 ? 8: 9));
	} else {
		snprintf(out, CODE_LEN, "\033[%dm", (value < 0 ? RESET: value) + 40);
	}
}

inline void
format_bgx(char *out, unsigned char num) {
	/*  Create an escape code for an extended background color.
		Arguments:
			out   : Memory allocated for the escape code string.
				    *Must have enough room for `CODEX_LEN`.
		    value : Value to use for background.
	*/
	snprintf(out, CODEX_LEN, "\033[48;5;%dm", num);
}

inline void
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

inline void
format_fore(char *out, Colors value) {
	/*  Create an escape code for a fore color.
		Arguments:
			out   : Memory allocated for the escape code string.
				    *Must have enough room for `CODEX_LEN`.
		    value : Colors value to use for fore.
	*/
	if (value > 9) {
		format_forex(out, value - (value > 16 ? 8: 9));
	} else {
		snprintf(out, CODE_LEN, "\033[%dm", (value < 0 ? RESET: value) + 30);
	}
}

inline void
format_forex(char *out, unsigned char num) {
	/*  Create an escape code for an extended fore color.
		Arguments:
			out   : Memory allocated for the escape code string.
				    *Must have enough room for `CODEX_LEN`.
		    value : Value to use for fore.
	*/
	snprintf(out, CODEX_LEN, "\033[38;5;%dm", num);
}

inline void
format_fore_rgb(char *out, unsigned char red, unsigned char green, unsigned char blue) {
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
    format_fore_rgb(
    	out,
    	(unsigned char)red,
    	(unsigned char)green,
    	(unsigned char)blue
    );
}

inline void
format_style(char *out, Styles style) {
	/*  Create an escape code for a style.
		Arguments:
			out   : Memory allocated for the escape code string.
				    *Must have enough room for `STYLE_LEN`.
		    style : Styles value to use for style.
	*/
	snprintf(out, STYLE_LEN, "\033[%dm", style < 0 ? RESET_ALL: style);
}

void
str_tolower(char *out, char *s) {
    int length = 0;
    for (int i = 0; s[i]; i++) {
        length++;
        out[i] = tolower(s[i]);
    }
    out[length] = '\0';
}

Styles
stylename_to_style(char *arg) {
    /*  Convert named argument to actual Styles enum value.
        Returns a Styles value on success, or STYLE_INVALID on error.
    */
    char arglower[MAX_NAME_LEN];
    str_tolower(arglower, arg);

    if (!strcmp(arglower, "none")) return STYLE_NONE;
    if (!strcmp(arglower, "reset")) return RESET_ALL;
    if (!strcmp(arglower, "bright")) return BRIGHT;
    if (!strcmp(arglower, "dim")) return DIM;
    if (!strcmp(arglower, "italic")) return ITALIC;
    if (!strcmp(arglower, "underline")) return UNDERLINE;
    if (!strcmp(arglower, "flash")) return FLASH;
    if (!strcmp(arglower, "highlight")) return HIGHLIGHT;
    if (!strcmp(arglower, "normal")) return NORMAL;
    return STYLE_INVALID;
}
#endif // _COLORS_H
