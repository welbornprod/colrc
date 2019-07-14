#ifndef DOXYGEN_SKIP
//! \file

/*! Example implementation using colr.h, and a command-line tool
    to colorize text.

    \author Christopher Welborn
    \date 02-05-2017
*/
#include "colr_tool.h"

#define return_on_null(x) \
    do { \
        if (!(x)) { \
            printferr("Failed to allocate memory for arguments!\n"); \
            return 1; \
        } \
    } while (0)


int main(int argc, char* argv[]) {
    /* TODO: parse_args() for flag arguments, while keeping positionals. */
    char *locale = setlocale(LC_ALL, "");
    dbug("Using locale: %s\n", locale); (void)locale; // Stupid clang linter in sublime text.
    // Declare args and setup some defaults.
    char* textarg = NULL;
    char* forearg = NULL;
    char* backarg = NULL;
    char* stylearg = NULL;

    switch (argc) {
        case 5:
            return_on_null(asprintf(&stylearg, "%s", argv[4]));
            /* fall through */
        case 4:
            return_on_null(asprintf(&backarg, "%s", argv[3]));
            /* fall through */
        case 3:
            return_on_null(asprintf(&forearg, "%s", argv[2]));
            /* fall through */
        case 2:
            return_on_null(asprintf(&textarg, "%s", argv[1]));
            break;
        case 1:
            // No arguments.
            return_on_null(asprintf(&textarg, "%s", "-"));
            break;
        default:
            print_usage("Too many arguments!");
            return 1;
    }

    if (argeq(textarg, "-h", "--help")) return print_usage_full();
    if (colr_streq(textarg, "-basic")) return print_basic(true);
    if (colr_streq(textarg, "-bbasic") || colr_streq(textarg, "-basicb")) return print_basic(false);
    if colr_streq(textarg, "-rainbow") return print_rainbow_fore(false);
    if colr_streq(textarg, "-RAINBOW") return print_rainbow_fore(true);
    if (colr_streq(textarg, "-256")) return print_256(true);
    if (colr_streq(textarg, "-b256") || colr_streq(textarg, "-256b")) return print_256(false);
    if (colr_streq(textarg, "-rgb")) return print_rgb(true);
    if (colr_streq(textarg, "-brgb") || colr_streq(textarg, "-rgbb")) return print_rgb(false);

    if (colr_streq(textarg, "-")) {
        // Read from stdin.
        free(textarg);
        textarg = read_stdin_arg();
        if (!textarg) {
            printferr("\nFailed to allocate for stdin data!\n");
            return 1;
        }
    }
    // Rainbowize the text arg.
    bool do_rainbow = colr_streq(forearg, "rainbow");
    bool do_term_rainbow = colr_streq(forearg, "RAINBOW");
    if (do_rainbow || do_term_rainbow) {
        free(forearg);
        free(backarg);
        free(stylearg);
        char* rainbowized = do_rainbow ? rainbow_fg(textarg, 0.1, 3) : rainbow_term_fg(textarg, 0.1, 3);
        free(textarg);
        printf("%s\n", rainbowized);
        free(rainbowized);
        return 0;
    }

    StyleValue styleval = StyleValue_from_str(stylearg);
    ColorArg style_carg = style_arg(styleval);
    if (!validate_color_arg(style_carg, stylearg)) {
        free(forearg);
        free(backarg);
        free(stylearg);
        free(textarg);
        return 1;
    }
    dbug_repr("Style: %s\n", style_carg);

    ColorArg fore_carg = fore_arg(forearg);
    if (!validate_color_arg(fore_carg, forearg)) {
        free(forearg);
        free(backarg);
        free(stylearg);
        free(textarg);
        return 1;
    }
    dbug_repr("Fore: %s\n", fore_carg);

    ColorArg back_carg = back_arg(backarg);
    if (!validate_color_arg(back_carg, backarg)) {
        free(forearg);
        free(backarg);
        free(stylearg);
        free(textarg);
        return 1;
    }
    dbug_repr("Back: %s\n", back_carg);

    ColorText *ctext = Colr(textarg, &fore_carg, &back_carg, &style_carg);
    dbug_repr("ColorText: %s\n", *ctext);
    char* text = ColorText_to_str(*ctext);
    printf("%s\n", text);
    free(text);
    free(ctext);
    free(forearg);
    free(backarg);
    free(stylearg);
    free(textarg);

    return 0;
}

void dbug_args(char* text, char* fore, char* back, char* style) {
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


int print_256(bool do_fore) {
    /*  Print the 256 color range using either colrfgx or colorbgx.
        The function choice is passed as an argument.
    */
    char num[4];
    ColorArg carg;
    char* text;
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_fore ? fore_arg(ext(i)) : back_arg(ext(i));
        free(text);
        if (i < 16) {
            text = ColorArg_to_str(carg);
            printf("%s ", text);
            if ((i == 7) || (i == 15)) puts("\n");
        } else {
            text = ColorArg_to_str(carg);
            printf("%s ", text);
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                carg = do_fore ? fore_arg(ext(i)) : back_arg(ext(i));
                text = ColorArg_to_str(carg);
                printf("%s ", text);
                free(text);
            }
            puts("\n");
        }
    }
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_fore ? fore_arg(ext(i)) : back_arg(ext(i));
        text = ColorArg_to_str(carg);
        printf("%s ", text);
        free(text);
    }
    puts("\n");
    return 0;
}

int print_basic(bool do_fore) {
    /* Print basic color names and escape codes. */
    char* text = NULL;
    char* namefmt = NULL;
    for (size_t i = 0; i < basic_names_len; i++) {
        char* name = basic_names[i].name;
        BasicValue val = basic_names[i].value;
        if (colr_streq(name, "black")) {
            puts("");
        }
        BasicValue otherval = str_endswith(name, "black") ? WHITE : BLACK;
        asprintf(&namefmt, "%-14s", name);
        if (do_fore) {
            text = colr(fore(val), back(otherval), namefmt);
        } else {
            text = colr(back(val), fore(otherval), namefmt);
        }
        printf("%s", text);
        free(namefmt);
        free(text);
    }
    printf("%s\n", CODE_RESET_ALL);
    return 0;
}

int print_rainbow_fore(bool term_colors) {
    /* Demo the rainbow method. */
    char text[] = "This is a demo of the rainbow function.";
    char* textfmt = term_colors ? rainbow_term_fg(text, 0.1, 3) : rainbow_fg(text, 0.1, 3);
    printf("%s\n", textfmt);
    free(textfmt);
    return 0;
}

int print_rgb(bool do_fore) {
    /*  Print part of the RGB range using either colrfgrgb, or .
        The function choice is passed as an argument.
    */
    char num[12];
    char* text;
    int count = 0;
    for (int r = 0; r < 256; r = r + 32) {
        for (int g = 0; g < 256; g = g + 32) {
            for (int b = 0; b < 256; b = b + 64) {
                // Make the rgb text.
                snprintf(num, 12, "%03d;%03d;%03d", r, g, b);
                // Colorize it.
                RGB vals = {r, g, b};
                RGB othervals = do_fore ? (RGB){0, 0, 0} : (RGB){255, 255, 255};
                if (do_fore) {
                    text = colr(fore(vals), back(othervals), num);
                } else {
                    text = colr(back(vals), fore(othervals), num);
                }
                count++;
                printf("%s ", text);
                free(text);
                if (count > 3) {
                    puts("\n");
                    count = 0;
                }
            }
        }
    }
    puts("\n");
    return 0;
}

void print_unrecognized_arg(const char* userarg) {
    /*   Print an error message, and the short usage string for an
        unrecognized argument.
    */
    char errmsg[] = "Unrecognized argument: ";
    size_t maxerrlen = strlen(errmsg) + strlen(userarg) + 1;
    char errfmt[maxerrlen];
    snprintf(errfmt, maxerrlen, "%s%s", errmsg, userarg);
    print_usage(errfmt);
}


int print_usage(const char* reason) {
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
    return 0;
}


int print_usage_full() {
    /* Print the usage string. */
    print_usage(NULL);
    printf("\
\n\
    Commands:\n\
        -basic            : Print all basic color names and colors.\n\
        -256              : Print all extended color names and colors.\n\
        -256b, b256       : Print all extended back color names and colors.\n\
        -rainbow          : Print a rainbow example.\n\
        -rgb              : Print some rgb codes.\n\
        -brgb, -rgbb      : Print some rgb back codes.\n\
\n\
    Options:\n\
        TEXT              : Text to colorize.\n\
                            Default: stdin\n\
        FORE              : Fore color name/value for text.\n\
                            If set to 'rainbow', the text will be rainbowized.\n\
        BACK              : Back color name/value for text.\n\
        STYLE             : Style name for text.\n\
        -h, --help        : Print this message and exit.\n\
        -v, --version     : Show version and exit.\n\
    ");
    puts("\n");
    return 0;
}

char* read_stdin_arg(void) {
    /*  Read stdin data into `textarg`.
        This only reads up to `length - 1` characters.
    */
    char line[1024];
    size_t line_length = 1024;
    char* buffer = NULL;
    if (isatty(fileno(stdin)) && isatty(fileno(stderr))) {
        dbug("\nReading from stdin until EOF (Ctrl + D)...\n");
    }
    while ((fgets(line, line_length, stdin))) {
        if (!buffer) {
            // First line.
            asprintf(&buffer, "%s", line);
        } else {
            char* oldbuffer = buffer;
            asprintf(&buffer, "%s%s", buffer, line);
            free(oldbuffer);
        }
    }
    return buffer;
}


bool validate_color_arg(ColorArg carg, const char* name) {
    /*  Checks `nametype` for TYPE_INVALID*, and prints the usage string
        with a warning message if it is invalid.
        If the code is not invalid, it simply returns `true`.
    */
    if (!name) {
        #ifdef DEBUG
        char* argtype = ArgType_to_str(carg.type);
        dbug("No %s arg given.\n", argtype);
        free(argtype);
        #endif
        return true;
    }
    char* errmsg;
    char argtype[6] = "fore";
    if (carg.type == BACK) sprintf(argtype, "%s", "back");

    switch (carg.value.type) {
        case TYPE_INVALID_RGB_RANGE:
            asprintf(&errmsg, "Invalid range (0-255) for %s RGB color: %s", argtype, name);
            break;
        case TYPE_INVALID_EXTENDED_RANGE:
            asprintf(&errmsg, "Invalid range (0-255) for extended %s color: %s", argtype, name);
            break;
        case TYPE_INVALID:
            asprintf(&errmsg, "Invalid %s color name: %s", argtype, name);
            break;
        case TYPE_INVALID_STYLE:
            asprintf(&errmsg, "Invalid style name: %s", name);
            break;
        default:
            // Valid color arg passed.
            // dbug("Valid color arg passed for %s: %s\n", type, name);
            return true;
    }

    // Print the error message that was built.
    print_usage(errmsg);
    free(errmsg);
    return false;
}

#endif // DOXYGEN_SKIP
