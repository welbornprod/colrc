#ifndef _COLORS_H_
#pragma GCC diagnostic ignored "-Wunused-macros"

#define _COLORS_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define COLR_VERSION "0.0.1"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* TODO: Refactor bg methods, they should be wrappable or easier to call.
		 The extended colors should be easier to mix with regular colors.
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
	RESET = 9
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

// Style code to reset all styling.
extern const char *STYLE_RESET_ALL;


const char *STYLE_RESET_ALL = "\033[0m";
const size_t STYLE_RESET_LEN = 7;

// Maximum length in chars for any combination of escape codes.
// Should be 8 + 8 + 8 + STYLE_RESET_LEN, but leaving extra room.
// Allocating for a string that will be colorized must account for this.
const size_t COLOR_LEN = 37;

// Maximim string length for a fore, back, or style name.
const size_t MAX_NAME_LEN = 10;

void str_tolower(char *out, char *s);

/* ------------------------------- Functions ------------------------------ */
void
color(char *out, Colors fore, char *s) {
	/*  Colorizes a string (`s`), with Fore code (`fore`), and copies the
		result into `out`.
	*/
	if (fore < 0) fore = RESET;
	size_t forecodelen = 10;
	char forecode[forecodelen];
	snprintf(forecode, forecodelen, "\033[%dm", fore + 30);
	size_t oldlen = strlen(s);
	size_t codeslen = forecodelen + STYLE_RESET_LEN;
	snprintf(
		out,
		oldlen + codeslen + 1,
		"%s%s%s", forecode, s, STYLE_RESET_ALL
	);
}


void
colorbg(char *out, Colors fore, Colors back, char *s) {
	/*  Same as `color()`, but use both fore and back colors.
	*/
	size_t forebackcodelen = 10;
	char forecode[forebackcodelen];
	if (fore < 0) fore = RESET;
	snprintf(forecode, forebackcodelen, "\033[%dm", fore + 30);
	char backcode[forebackcodelen];
	if (back < 0) back = RESET;
	snprintf(backcode, forebackcodelen, "\033[%dm", back + 40);
	size_t oldlen = strlen(s);
	size_t codeslen = strlen(forecode) + strlen(backcode) + STYLE_RESET_LEN;
	if (back == RESET) {
		// Put the back reset code last.
		snprintf(
			out,
			oldlen + codeslen + 1,
			"%s%s%s%s",
			forecode, backcode, s, STYLE_RESET_ALL
		);

	} else {
		snprintf(
			out,
			oldlen + codeslen + 1,
			"%s%s%s%s",
			backcode, forecode, s, STYLE_RESET_ALL
		);
	}

}

void
colorext(char *out, unsigned char num, char *s) {
	/*  Colorize a string using extended, 256, colors, and copy the
		result into `out`.
		Arguments:
			out  : Allocated memory to copy the result to.
				   *Must have enough room for `strlen(s) + COLOR_LEN`.
		    num  : Code number, 0-255 inclusive.
		    s    : String to colorize.
		    	   The Styles.RESET_ALL code is appended to the result.
	*/
	size_t forecodelen = 15;
	char forecode[forecodelen];
	snprintf(forecode, forecodelen, "\033[38;5;%dm", num);

	size_t oldlen = strlen(s);
	size_t codeslen = strlen(forecode) + STYLE_RESET_LEN;

	snprintf(
		out,
		oldlen + codeslen + 1,
		"%s%s%s",
		forecode, s, STYLE_RESET_ALL
	);
}

void
colorextbg(char *out, unsigned char num, char *s) {
	/*  Colorize a string using extended, 256, bg colors, and copy the
		result into `out`.
		Arguments:
			out  : Allocated memory to copy the result to.
				   *Must have enough room for `strlen(s) + COLOR_LEN`.
		    num  : Code number, 0-255 inclusive.
		    s    : String to colorize.
		    	   The Styles.RESET_ALL code is appended to the result.
	*/
	size_t backcodelen = 15;
	char backcode[backcodelen];
	snprintf(backcode, backcodelen, "\033[48;5;%dm", num);

	size_t oldlen = strlen(s);
	size_t codeslen = strlen(backcode) + STYLE_RESET_LEN;

	snprintf(
		out,
		oldlen + codeslen + 1,
		"%s%s%s",
		backcode, s, STYLE_RESET_ALL
	);
}

/* TODO: colorextstyle(), pass fore, back, and style using 256 colors. */

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
colorstyle(char *out, Colors fore, Colors back, Styles stylecode, char *s) {
	/*  Same as `color()`, but use all fore, back, and style.
	*/

	// Build forecolor only.
	size_t forebackcodelen = 10;
	char forecode[forebackcodelen];
	if (fore < 0) fore = RESET;
	snprintf(forecode, forebackcodelen, "\033[%dm", fore + 30);
	// Build backcolor only.
	char backcode[forebackcodelen];
	if (back < 0) back = RESET;
	snprintf(backcode, forebackcodelen, "\033[%dm", back + 40);
	// Build style only.
	size_t stylelen = 8;
	char stylecodes[stylelen];
	if (stylecode < 0) stylecode = RESET_ALL;
	snprintf(stylecodes, stylelen, "\033[%dm", stylecode);

	// Seperate reset codes from other codes by making sure normal codes
	// are used last.
	size_t codeslen = forebackcodelen + stylelen;
	char frontcodes[codeslen];
	frontcodes[0] = '\0';
	char endcodes[codeslen];
	endcodes[0] = '\0';
	strncat(fore == RESET ? frontcodes: endcodes, forecode, codeslen + 1);
	strncat(back == RESET ? frontcodes: endcodes, backcode, codeslen + 1);
	strncat(stylecode == RESET_ALL ? frontcodes: endcodes, stylecodes, codeslen + 1);

	size_t oldlen = strlen(s);
	codeslen =  strlen(frontcodes) + strlen(endcodes) + STYLE_RESET_LEN;
	snprintf(
		out,
		oldlen + codeslen + 1,
		"%s%s%s%s",
		frontcodes, endcodes, s, STYLE_RESET_ALL
	);
}


void
style(char *out, Styles style, char *s) {
	if (style < 0) style = RESET_ALL;
	size_t stylecodelen = 10;
	char stylecode[stylecodelen];
	snprintf(stylecode, stylecodelen, "\033[%dm", style);

	size_t oldlen = strlen(s);
	size_t codeslen = strlen(stylecode) + STYLE_RESET_LEN;
	snprintf(
		out,
		oldlen + codeslen + 1,
		"%s%s%s",
		stylecode, s, STYLE_RESET_ALL
	);
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
