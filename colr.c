/* colr_example.c
    Example implementation using colr.h.

    -Christopher Welborn 02-05-2017
*/
#include "colr_tool.h"

int main(int argc, char *argv[]) {

    /* TODO: parse_args() for flag arguments, while keeping positionals. */

    // Declare args and setup some defaults.
    char textarg[MAX_TEXT_LEN + 1];
    strncpy(textarg, "-", MAX_TEXT_LEN -1);
    char forearg[MAX_ARG_LEN + 1];
    strncpy(forearg, "reset", MAX_ARG_LEN -1);
    char backarg[MAX_ARG_LEN];
    strncpy(backarg, "reset", MAX_ARG_LEN -1);
    char stylearg[MAX_ARG_LEN];
    strncpy(stylearg, "reset", MAX_ARG_LEN -1);

    switch (argc) {
        case 5: strncpy(stylearg, argv[4], MAX_ARG_LEN - 1);
        case 4: strncpy(backarg, argv[3], MAX_ARG_LEN - 1);
        case 3: strncpy(forearg, argv[2], MAX_ARG_LEN - 1);
        case 2:
            strncpy(textarg, argv[1], MAX_TEXT_LEN - 1);
            break;
        case 1:
            // No arguments.
            strncpy(textarg, "-", MAX_TEXT_LEN - 1);
            break;
        default:
            print_usage("Too many arguments!");
            return 1;
    }

    if (streq(textarg, "256")) {
        print_256(colorext);
    } else if (streq(textarg, "b256") || streq(textarg, "256b")) {
        print_256(colorextbg);
    } else {
        if (streq(textarg, "-")) {
            // Read from stdin.
            char *textargp = textarg;
            read_stdin_arg(textargp, MAX_TEXT_LEN);
        }
        Colors fore = colorname_to_color(forearg);
        Colors back = colorname_to_color(backarg);
        Styles stylecode = stylename_to_style(stylearg);
        if (!validate_color_arg("fore", fore, forearg)) return 1;
        if (!validate_color_arg("back", back, backarg)) return 1;
        if (!validate_style_arg(stylecode, stylearg)) return 1;
        char colorized[MAX_TEXT_LEN + COLOR_LEN];
        colorstyle(colorized, fore, back, stylecode, textarg);
        printf("%s\n", colorized);
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
print_256(colorext_func func) {
    /*  Print the 256 color range using either colorext or colorextbg.
        The function choice is passed as an argument.
    */
    char num[4];
    char text[4 + COLOR_LEN];
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        func(text, i, num);
        if (i < 16) {
            printf("%s ", text);
            if ((i == 7) || (i == 15)) puts("\n");
        } else {
            printf("%s ", text);
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                func(text, j, num);
                printf("%s ", text);
            }
            puts("\n");
        }
    }
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        func(text, i, num);
        printf("%s ", text);
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
        colr [TEXT] [FORE] [BACK] [STYLE]\n\
    ", NAME, VERSION);
}


void
print_usage_full() {
    /* Print the usage string. */
    print_usage(NULL);
    printf("\
\n\
    Options:\n\
        TEXT              : Text to colorize.\n\
                            Default: stdin\n\
        FORE              : Fore color for text.\n\
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
validate_color_arg(char *type, Colors code, char *name) {
    /*  Checks `code` for COLOR_INVALID, and prints the usage string with a
        warning message if it is invalid.
        If the code is not invalid, it simply returns true.
    */
    if (code == COLOR_INVALID) {
        char errmsg[255];
        snprintf(errmsg, MAX_ERR_LEN, "Invalid %s color name: %s", type, name);
        print_usage(errmsg);
        return false;
    }
    return true;
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
