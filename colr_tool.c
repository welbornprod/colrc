/* colr_example.c
    Example implementation using colr.h.

    -Christopher Welborn 02-05-2017
*/
#include "colr_tool.h"
#define return_on_null(x) do { \
    if (!(x)) { \
        printferr("Failed to allocate memory for arguments!\n"); \
        return 1; \
    } \
    } while (0)

int main(int argc, char *argv[]) {

    /* TODO: parse_args() for flag arguments, while keeping positionals. */

    // Declare args and setup some defaults.
    char textarg[MAX_TEXT_LEN + 1] = "-";
    char forearg[MAX_ARG_LEN + 1] = "reset";
    char backarg[MAX_ARG_LEN + 1] = "reset";
    char stylearg[MAX_ARG_LEN + 1] = "reset";

    switch (argc) {
        case 5:
            return_on_null(str_copy(stylearg, argv[4], MAX_ARG_LEN - 1));
            __attribute__((fallthrough));
        case 4:
            return_on_null(str_copy(backarg, argv[3], MAX_ARG_LEN - 1));
            __attribute__((fallthrough));
        case 3:
            return_on_null(str_copy(forearg, argv[2], MAX_ARG_LEN - 1));
            __attribute__((fallthrough));
        case 2:
            return_on_null(str_copy(textarg, argv[1], MAX_TEXT_LEN - 1));
            break;
        case 1:
            // No arguments.
            return_on_null(str_copy(textarg, "-", MAX_TEXT_LEN - 1));
            break;
        default:
            print_usage("Too many arguments!");
            return 1;
    }
    if (argeq(textarg, "-h", "--help")) {
        print_usage_full();
    } else if (streq(textarg, "-basic")) {
        print_basic();
    } else if (streq(textarg, "-build")) {
        example_color_build();
    } else if streq(textarg, "-rainbow") {
        print_rainbow_fore();
    } else if (streq(textarg, "-256")) {
        print_256(colrforex);
    } else if (streq(textarg, "-rgb")) {
        print_rgb(colrforeRGB);
    } else if (streq(textarg, "-brgb") || streq(textarg, "-rgbb")) {
        print_rgb(colrbgRGB);
    } else if (streq(textarg, "-b256") || streq(textarg, "-256b")) {
        print_256(colrbgx);
    } else {
        if (streq(textarg, "-")) {
            // Read from stdin.
            char *textargp = textarg;
            read_stdin_arg(textargp, MAX_TEXT_LEN);
        }
        // Rainbowize the text arg.
        if (streq(forearg, "rainbow")) {
            char *rainbowized = acolrforerainbow(textarg, 0.1, 3.0);
            printf("%s\n", rainbowized);
            free(rainbowized);
            return 0;
        }
        /* TODO: colorname_to_color (and colrize) are only for basic codes.
                 This tool should show colrizex and colrforergb.
                 See TODOS in colr.h, still need colrizergb() implementation.
        */
        Styles stylecode = stylename_to_style(stylearg);
        if (!validate_style_arg(stylecode, stylearg)) return 1;

        ColorNameType forenametype = colorname_type(forearg);
        if (!validate_color_arg("fore", forenametype, forearg)) return 1;
        ColorNameType backnametype = colorname_type(backarg);
        if (!validate_color_arg("back", backnametype, backarg)) return 1;

        /*  TODO: Should be able to mix basic and extended codes for fore and
                  back colors.
        */
        if (forenametype == COLORNAME_BASIC && backnametype == COLORNAME_BASIC) {
            Colors fore = colorname_to_color(forearg);
            Colors back = colorname_to_color(backarg);
            char *colorized = acolrize(textarg, fore, back, stylecode);
            printf("%s\n", colorized);
        } else if (forenametype == COLORNAME_EXTENDED && backnametype == COLORNAME_EXTENDED) {
            unsigned char forex = colorname_to_colorx(forearg);
            unsigned char backx = colorname_to_colorx(backarg);
            char *colorizedx = acolrizex(textarg, forex, backx, stylecode);
            printf("%s\n", colorizedx);
        } else {
            printferr("Cannot mix color types for fore/back!\n");
            return 1;
        }
    }
    return 0;
}

void
debug_args(char *text, char *fore, char *back, char *style) {
    /*  This just pretty-prints the arguments, to verify arg-parsing logic.
    */
    printferr("Arguments:\n\
    Text: \n\
        %s\n\
    Fore: %s\n\
    Back: %s\n\
    Style: %s\n\n", text, fore, back, style
    );
}

void
example_color_build(void) {
    /* Example colr.h usage, building a string from a mixture of color codes.
    */
    size_t length = 15; // strlen("This is a test.")
    size_t wordlen = 5; // 5 seperate colorized strings.
    // Allow enough space for color codes.
    size_t finallen = length + (wordlen * COLOR_LEN);
    char *s = (char*)calloc(finallen, sizeof(char));
    debug(
        "Building string of %ld characters for color building.\n",
        finallen
    );
    colrforecat(s, "This ", RED);
    printf("\n%s\n", s);
    colrizecat(s, "is ", BLUE, WHITE, BRIGHT);
    printf("\n%s\n", s);
    colrbgcat(s, "a ", MAGENTA);
    printf("\n%s\n", s);
    colrforecat(s, "test", GREEN);
    printf("\n%s\n", s);
    colrizecharcat(s, '.', LIGHTCYAN, RESET, NORMAL);
    printf("\n%s\n", s);
    debug("\nFinal string length was: %ld\n", strlen(s));

    free(s);
}

void
print_256(colorext_func func) {
    /*  Print the 256 color range using either colrforex or colorbgx.
        The function choice is passed as an argument.
    */
    char num[4];
    char text[4 + COLOR_LEN];
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        func(text, num, i);
        if (i < 16) {
            printf("%s ", text);
            if ((i == 7) || (i == 15)) puts("\n");
        } else {
            printf("%s ", text);
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                func(text, num, j);
                printf("%s ", text);
            }
            puts("\n");
        }
    }
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        func(text, num, i);
        printf("%s ", text);
    }
    puts("\n");
}

void
print_basic() {
    /* Print basic color names and escape codes. */
    // Allocate memory for an extended fore code string, and it's color name.
    size_t max_color_name_len = 12;
    char *name = calloc(COLOR_LEN + max_color_name_len, sizeof(char));
    print_fore_color(BLACK);
    print_fore_color(RED);
    print_fore_color(GREEN);
    print_fore_color(YELLOW);
    print_fore_color(BLUE);
    print_fore_color(MAGENTA);
    print_fore_color(CYAN);
    print_fore_color(WHITE);
    print_fore_color(UNUSED);
    print_fore_color(RESET);
    puts("\n");
    print_fore_color(XRED);
    print_fore_color(XGREEN);
    print_fore_color(XYELLOW);
    print_fore_color(XBLUE);
    print_fore_color(XMAGENTA);
    print_fore_color(XCYAN);
    print_fore_color(XNORMAL);
    puts("\n");
    print_fore_color(LIGHTRED);
    print_fore_color(LIGHTGREEN);
    print_fore_color(LIGHTYELLOW);
    print_fore_color(LIGHTBLUE);
    print_fore_color(LIGHTMAGENTA);
    print_fore_color(LIGHTCYAN);
    print_fore_color(LIGHTNORMAL);
    puts("\n");
    free(name);
}

void
print_rainbow_fore() {
    /* Demo the rainbow method. */
    char text[] = "This is a demo of the rainbow function.";
    char *textfmt = acolrforerainbow(text, 0.1, 30);
    printf("%s\n", textfmt);
    free(textfmt);
}

void
print_rgb(colorrgb_func func) {
    /*  Print part of the RGB range using either colrforergb, or .
        The function choice is passed as an argument.
    */
    char num[12];
    char text[14 + COLOR_RGB_LEN];
    int count = 0;
    for (int r = 0; r < 256; r = r + 32) {
        for (int g = 0; g < 256; g = g + 32) {
            for (int b = 0; b < 256; b = b + 64) {
                // Make the rgb text.
                snprintf(num, 12, "%03d;%03d;%03d", r, g, b);
                // Colorize it.
                struct RGB vals = {r, g, b};
                func(text, num, &vals);
                count++;
                printf("%s ", text);
                if (count > 3) {
                    puts("\n");
                    count = 0;
                }
            }
        }
    }
    puts("\n");
}

void
print_unrecognized_arg(const char *userarg) {
    /*   Print an error message, and the short usage string for an
        unrecognized argument.
    */
    char errmsg[] = "Unrecognized argument: ";
    size_t maxerrlen = strlen(errmsg) + strlen(userarg) + 1;
    char errfmt[maxerrlen];
    snprintf(errfmt, maxerrlen, "%s%s", errmsg, userarg);
    print_usage(errfmt);
}


void
print_usage(const char *reason) {
    /* Print the short usage string with optional `reason` */
    if (reason) {
        printferr("\n%s\n\n", reason);
    }
    printf("%s v. %s\n\
    Usage:\n\
        colr -h | -v\n\
        colr -basic | -build | -256 | -b256 | -256b | -rainbow | -rgb | -brgb\n\
        colr [TEXT] [FORE] [BACK] [STYLE]\n\
    ", NAME, VERSION);
}


void
print_usage_full() {
    /* Print the usage string. */
    print_usage(NULL);
    printf("\
\n\
    Commands:\n\
        -basic            : Print all basic color names and colors.\n\
        -build            : Run color building example.\n\
        -256              : Print all extended color names and colors.\n\
        -256b, b256       : Print all extended back color names and colors.\n\
        -rainbow          : Print a rainbow example.\n\
        -rgb              : Print some rgb codes.\n\
        -brgb, -rgbb      : Print some rgb back codes.\n\
\n\
    Options:\n\
        TEXT              : Text to colorize.\n\
                            Default: stdin\n\
        FORE              : Fore color for text.\n\
                            If set to 'rainbow', the text will be rainbowized.\n\
                            Default: reset\n\
        BACK              : Back color for text.\n\
                            Default: reset\n\
        STYLE             : Style for text.\n\
                            Default: reset\n\
        -h, --help        : Print this message and exit.\n\
        -v, --version     : Show version and exit.\n\
    ");
    puts("\n");
}

void
read_stdin_arg(char *textarg, size_t length) {
    /*  Read stdin data into `textarg`.
        This only reads up to `length - 1` characters.
    */
    textarg[0] = '\0';
    size_t totallen = 0;
    char line[length];
    if (isatty(fileno(stdin)) && isatty(fileno(stderr))) {
        debug("\nReading from stdin until EOF (Ctrl + D)...\n");
    }
    while (totallen <= length) {
        if (fgets(line, length - totallen, stdin) == NULL) {
            // Never happens if len(stdin_data) > length
            break;
        }
        size_t linelen = strlen(line);
        if (linelen == 0) {
            // Happens if len(stdin_data) > length
            break;
        }
        size_t possiblelen = totallen + linelen;
        if (possiblelen > length) {
            break;
        }
        strncat(textarg, line, length - strlen(textarg));
        totallen = strlen(textarg);
    }
}

bool
validate_color_arg_OLD(char *type, Colors code, char *name) {
    /*  Checks `code` for COLOR_INVALID, and prints the usage string with a
        warning message if it is invalid.
        If the code is not invalid, it simply returns true.
    */
    if (code == COLOR_INVALID) {
        char errmsg[MAX_ERR_LEN];
        snprintf(errmsg, MAX_ERR_LEN, "Invalid %s color name: %s", type, name);
        print_usage(errmsg);
        return false;
    }
    return true;
}


bool
validate_color_arg(const char *type, ColorNameType nametype, const char *name) {
    /*  Checks `nametype` for COLORNAME_INVALID*, and prints the usage string
        with a warning message if it is invalid.
        If the code is not invalid, it simply returns `true`.
    */
    char errmsg[MAX_ERR_LEN];
    switch (nametype) {
        case COLORNAME_INVALID_RGB_RANGE:
            snprintf(errmsg, MAX_ERR_LEN, "Invalid range (0-255) for %s RGB color: %s", type, name);
            break;
        case COLORNAME_INVALID_EXTENDED_RANGE:
            snprintf(errmsg, MAX_ERR_LEN, "Invalid range (0-255) for extended %s color: %s", type, name);
            break;
        case COLORNAME_INVALID:
            snprintf(errmsg, MAX_ERR_LEN, "Invalid %s color name: %s", type, name);
            break;
        default:
            // Valid color arg passed.
            // debug("Valid color arg passed for %s: %s\n", type, name);
            return true;
    }

    // Print the error message that was built.
    print_usage(errmsg);
    return false;
}

bool
validate_style_arg(Styles code, char *name) {
    /*  Checks `code` for STYLE_INVALID, and prints the usage string with a
        warning message if it is invalid.
        If the code is not invalid, it simply returns true.
    */
    if (code == STYLE_INVALID) {
        char errmsg[255];
        snprintf(errmsg, MAX_ERR_LEN, "Invalid style name: %s", name);
        print_usage(errmsg);
        return false;
    }
    return true;
}
