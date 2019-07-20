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
    // Needed for str_to_wide(), and wide_to_str(), and the rainbow() funcs.
    setlocale(LC_ALL, "");
    ColrToolOptions colropts = ColrToolOptions_new();
    int parse_ret = parse_args(argc, argv, &colropts);
    // print_opts_repr(colropts);
    if (parse_ret >= 0) return parse_ret;

    bool free_text = false;
    if (colr_streq(colropts.text, "-")) {
        // Read from stdin.
        colropts.text = read_stdin_arg();
        if (!colropts.text) {
            printferr("\nFailed to allocate for stdin data!\n");
            return 1;
        }
        free_text = true;
    }
    // Rainbowize the text arg.
    bool do_term_rainbow = false;
    if (colropts.rainbow_fore || colropts.rainbow_back) {
        do_term_rainbow = !colr_supports_rgb();
        // TODO: User args for these freq and offet values.
        double freq = 0.1;
        double offset = 3;
        char* rainbowized = (
            do_term_rainbow ?
                (colropts.rainbow_fore ?
                    rainbow_fg_term(colropts.text, freq, offset) :
                    rainbow_bg_term(colropts.text, freq, offset)
                ) :
                (colropts.rainbow_fore ?
                    rainbow_fg(colropts.text, freq, offset) :
                    rainbow_bg(colropts.text, freq, offset)
                )
        );
        // Some or all of the fore/back/style args are "empty" (not null).
        // They will not be used if they are empty, but they will be free'd.
        char* styled = colr(
            colropts.fore,
            colropts.back,
            colropts.style,
            rainbowized
        );
        free(rainbowized);
        printf("%s\n", styled);
        free(styled);
        return 0;
    }

    ColorText *ctext = Colr(
        colropts.text,
        colropts.fore,
        colropts.back,
        colropts.style
    );
    dbug_repr("ColorText: %s\n", *ctext);
    char* text = ColorText_to_str(*ctext);
    printf("%s\n", text);
    free(text);
    if (free_text) free(colropts.text);
    return 0;
}

ColrToolOptions ColrToolOptions_new(void) {
    return (ColrToolOptions){
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .rainbow_fore=false,
        .rainbow_back=false,
        .print_back=false,
        .print_256=false,
        .print_basic=false,
        .print_rainbow=false,
        .print_rgb=false,
        .print_rgb_term=false,
    };
}

char* ColrToolOptions_repr(ColrToolOptions colropts) {
    char* text_repr = colropts.text ? colr_repr(colropts.text) : NULL;
    char* fore_repr = colropts.fore ? colr_repr(*(colropts.fore)) : NULL;
    char* back_repr = colropts.back ? colr_repr(*(colropts.back)) : NULL;
    char* style_repr = colropts.style ? colr_repr(*(colropts.style)) : NULL;
    char* repr;
    asprintf(
        &repr,
        "ColrToolOptions(\n\
    .text=%s,\n\
    .fore=%s,\n\
    .back=%s,\n\
    .style=%s,\n\
    .rainbow_fore=%s,\n\
    .rainbow_back=%s,\n\
    .print_back=%s,\n\
    .print_256=%s,\n\
    .print_basic=%s,\n\
    .print_rgb=%s,\n\
    .print_rgb_term=%s,\n\
    .print_rainbow=%s,\n\
)",
        text_repr ? text_repr : "NULL",
        fore_repr ? fore_repr : "NULL",
        back_repr ? back_repr : "NULL",
        style_repr ? style_repr : "NULL",
        colropts.rainbow_fore ? "true" : "false",
        colropts.rainbow_back ? "true" : "false",
        colropts.print_back ? "true" : "false",
        colropts.print_256 ? "true" : "false",
        colropts.print_basic ? "true" : "false",
        colropts.print_rgb ? "true" : "false",
        colropts.print_rgb_term ? "true" : "false",
        colropts.print_rainbow ? "true" : "false"
    );
    if (text_repr) free(text_repr);
    if (fore_repr) free(fore_repr);
    if (back_repr) free(back_repr);
    if (style_repr) free(style_repr);

    return repr;
}


int parse_args(int argc, char** argv, ColrToolOptions* colropts) {
    int c;
    char* unknownmsg = NULL;

    // Tell getopt that I'll handle the bad-argument messages.
    opterr = 0;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"version", no_argument, 0, 'v'},
            {"fore", required_argument, 0,  'f'},
            {"back", required_argument, 0, 'b'},
            {"style", required_argument, 0, 's'},
            {"basic", no_argument, 0, 0 },
            {"basicbg", no_argument, 0, 0 },
            {"256", no_argument, 0, 0},
            {"256bg", no_argument, 0, 0},
            {"rainbow", no_argument, 0, 0},
            {"rainbowbg", no_argument, 0, 0},
            {"rgb", no_argument, 0, 0},
            {"rgbbg", no_argument, 0, 0},
            {"termrgb", no_argument, 0, 0},
            {"termrgbbg", no_argument, 0, 0},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "hf:b:s:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                if (colr_streq(long_options[option_index].name, "basic")) {
                    colropts->print_basic = true;
                    return print_basic(false);
                } else if (colr_streq(long_options[option_index].name, "256")) {
                    colropts->print_256 = true;
                    return print_256(false);
                } else if (colr_streq(long_options[option_index].name, "rainbow")) {
                    colropts->print_rainbow = true;
                    return print_rainbow(false);
                } else if (colr_streq(long_options[option_index].name, "rgb")) {
                    colropts->print_rgb = true;
                    return print_rgb(false, false);
                } else if (colr_streq(long_options[option_index].name, "termrgb")) {
                    colropts->print_rgb_term = true;
                    return print_rgb(false, true);
                } else if (colr_streq(long_options[option_index].name, "basicbg")) {
                    colropts->print_back = true;
                    colropts->print_basic = true;
                    return print_basic(true);
                } else if (colr_streq(long_options[option_index].name, "256bg")) {
                    colropts->print_back = true;
                    colropts->print_256 = true;
                    return print_256(true);
                } else if (colr_streq(long_options[option_index].name, "rainbowbg")) {
                    colropts->print_back = true;
                    colropts->print_rainbow = true;
                    return print_rainbow(true);
                } else if (colr_streq(long_options[option_index].name, "rgbbg")) {
                    colropts->print_back = true;
                    colropts->print_rgb = true;
                    return print_rgb(true, false);
                } else if (colr_streq(long_options[option_index].name, "termrgbbg")) {
                    colropts->print_back = true;
                    colropts->print_rgb_term = true;
                    return print_rgb(true, true);
                } else {
                    printferr(
                        "Unhandled long-only option!: %s\n",
                        long_options[option_index].name
                    );
                    return 1;
                }
                break;
            case 'b':
                if (colr_streq(optarg, "rainbow")) {
                    colropts->rainbow_back = true;
                    colropts->back = ColorArg_to_ptr(ColorArg_empty());
                } else  {
                    colropts->back = back(optarg);
                    if (!validate_color_arg(*(colropts->back), optarg)) return 1;
                }
                break;
            case 'f':
                if (colr_streq(optarg, "rainbow")) {
                    colropts->rainbow_fore = true;
                    colropts->fore = ColorArg_to_ptr(ColorArg_empty());
                } else  {
                    colropts->fore = fore(optarg);
                    if (!validate_color_arg(*(colropts->fore), optarg)) return 1;
                }
                break;
            case 'h':
                print_usage_full();
                return 0;
            case 's':
                colropts->style = style(optarg);
                if (!validate_color_arg(*(colropts->style), optarg)) return 1;
                break;
            case 'v':
                print_version();
                return 0;
            case '?':
                asprintf(&unknownmsg, "Unknown argument: %c", optopt);
                print_usage(unknownmsg);
                free(unknownmsg);
                return 1;
                break;
            default:
                printferr("Unknown option!: %c\n", c);
                return 1;
        }
    }

    if (optind == argc) {
       print_usage("No text given!");
       return 1;
    }
    // Remaining non-option arguments.
    while (optind < argc) {
        if (!(colropts->text)) {
            colropts->text = argv[optind];
            optind++;
        } else if (!(colropts->fore)) {
            if (colr_streq(argv[optind], "rainbow")) {
                colropts->rainbow_fore = true;
            } else {
                colropts->fore = fore(argv[optind]);
                if (!validate_color_arg(*(colropts->fore), argv[optind])) return 1;
            }
            optind++;
        } else if (!(colropts->back)) {
            colropts->back = back(argv[optind]);
            if (!validate_color_arg(*(colropts->back), argv[optind])) return 1;
            optind++;
        } else if (!(colropts->style)) {
            colropts->style = style(argv[optind]);
            if (!validate_color_arg(*(colropts->style), argv[optind])) return 1;
            optind++;
        } else {
            print_usage("Too many arguments!");
            return 1;
        }
    }
    // Fill null args with "empty" args, so the colr macros know what to do.
    if (!(colropts->fore)) colropts->fore = ColorArg_to_ptr(ColorArg_empty());
    if (!(colropts->back)) colropts->back = ColorArg_to_ptr(ColorArg_empty());
    if (!(colropts->style)) colropts->style = ColorArg_to_ptr(ColorArg_empty());

    return -1;
}


/*! Print the 256 color range using either colrfgx or colorbgx.
    The function choice is passed as an argument.
*/
int print_256(bool do_back) {
    char num[4];
    ColorArg* carg;
    char* text;
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back(ext(i)) : fore(ext(i));
        if (i < 16) {
            text = colr(carg, num);
            printf("%s ", text);
            if ((i == 7) || (i == 15)) puts("\n");
            free(text);
        } else {
            text = colr(carg, num);
            printf("%s ", text);
            free(text);
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                carg = do_back ? back(ext(i)) : fore(ext(i));
                text = colr(carg, num);
                printf("%s ", text);
                free(text);
            }
            puts("\n");
        }
    }
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back(ext(i)) : fore(ext(i));
        text = colr(carg, num);
        printf("%s ", text);
        free(text);
    }
    puts("\n");
    return 0;
}

/*! Print basic color names and escape codes.
*/
int print_basic(bool do_back) {
    for (size_t i = 0; i < basic_names_len; i++) {
    char* namefmt = NULL;
    char* text = NULL;
        char* name = basic_names[i].name;
        BasicValue val = basic_names[i].value;
        if (colr_streq(name, "black")) {
            puts("");
        }
        BasicValue otherval = str_ends_with(name, "black") ? WHITE : BLACK;
        asprintf(&namefmt, "%-14s", name);
        if (do_back) {
            text = colr(back(val), fore(otherval), namefmt);
        } else {
            text = colr(fore(val), back(otherval), namefmt);
        }
        printf("%s", text);
        free(namefmt);
        free(text);
    }
    printf("%s\n", CODE_RESET_ALL);
    return 0;
}

/*! Demo the rainbow method.
*/
int print_rainbow(bool do_back) {
    char text[] = "This is a demo of the rainbow function.";
    char* rainbowtxt;
    if (do_back) {
        rainbowtxt = colr_supports_rgb() ? rainbow_bg(text, 0.1, 3) : rainbow_bg_term(text, 0.1, 3);
    } else {
        rainbowtxt = colr_supports_rgb() ? rainbow_fg(text, 0.1, 3) : rainbow_fg_term(text, 0.1, 3);
    }
    char* textfmt = colr(
        do_back ? fore(BLACK) : back(RESET),
        do_back ? style(BRIGHT) : style(NORMAL),
        rainbowtxt
    );
    free(rainbowtxt);
    printf("%s\n", textfmt);
    free(textfmt);
    return 0;
}

/*! Print part of the RGB range using either colrfgrgb, or .
    The function choice is passed as an argument.
*/
int print_rgb(bool do_back, bool term_rgb) {
    char* num;
    char* text;
    int count = 0;
    for (int r = 0; r < 256; r = r + 32) {
        for (int g = 0; g < 256; g = g + 32) {
            for (int b = 0; b < 256; b = b + 64) {
                RGB vals = {r, g, b};
                if (term_rgb) {
                    // Make the extended value text.
                    ExtendedValue extval = ExtendedValue_from_RGB(vals);
                    num = ExtendedValue_to_str(extval);
                    ExtendedValue otherval = do_back ? XWHITE : XBLACK;
                    if (do_back) {
                        text = colr(back(extval), fore(otherval), num);
                    } else {
                        text = colr(fore(extval), back(otherval), num);
                    }
                } else {
                    // Make the rgb text.
                    num = RGB_to_str(vals);
                    // Colorize it.
                    RGB othervals = do_back ? (RGB){255, 255, 255} : (RGB){0, 0, 0};
                    if (do_back) {
                        text = colr(back(vals), fore(othervals), num);
                    } else {
                        text = colr(fore(vals), back(othervals), num);
                    }
                }
                free(num);
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


/*! Print the short usage string with optional `reason`
*/
int print_usage(const char* reason) {
    if (reason) {
        printferr("\n%s\n\n", reason);
    }
    printf("%s v. %s\n\
    Usage:\n\
        colr -h | -v\n\
        colr --basic | --256 | --rainbow | --rgb | --termrgb\n\
        colr [TEXT] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
    ", NAME, VERSION);
    return 0;
}


/*! Print the usage string.
*/
int print_usage_full() {
    print_usage(NULL);
    printf("\
\n\
    Commands:\n\
        --basic[bg]       : Print all basic color names and colors.\n\
        --256[bg]         : Print all extended color names and colors.\n\
        --rainbow[bg]     : Print a rainbow example.\n\
        --rgb[bg]         : Print some rgb codes.\n\
        --termrgb[bg]     : Print some 256-compatible rgb codes.\n\
\n\
    If 'bg' is appended to a command argument, back colors will be used.\n\
\n\
    Options:\n\
        TEXT                : Text to colorize.\n\
                              Default: stdin\n\
        FORE                : Fore color name/value for text.\n\
                              If set to 'rainbow', the text will be rainbowized.\n\
        BACK                : Back color name/value for text.\n\
                              If set to 'rainbow', the back colors will be rainbowized.\n\
        STYLE               : Style name for text.\n\
        -b val,--back val   : Specify the back color explicitly, in any order.\n\
        -f val,--fore val   : Specify the fore color explicitly, in any order.\n\
        -h, --help          : Print this message and exit.\n\
        -s val,--style val  : Specify the style explicitly, in any order.\n\
        -v, --version       : Show version and exit.\n\
\n\
    When the flag arguments are used (-f, -b, -s), the order does not matter\n\
    and any of them may be omitted. The remaining non-flag arguments are parsed\n\
    in order (text, fore, back, style).\n\
\n\
    Color values can be one of:\n\
        A known name.      \"none\", \"red\", \"blue\", \"lightblue\", \"black\", etc.\n\
                           \"rainbow\" causes fore or back colors to be rainbowized.\n\
        A 256-color value. 0-255\n\
        An RGB string.     \"R;G;B\", \"R:G:B\", \"R,G,B\", or \"R G B\".\n\
        A hex color.       \"#ffffff\", or the short-form \"#fff\"\n\
    ");
    puts("\n");
    return 0;
}

/*! Print the ColrC version.
*/
int print_version(void) {
    printf(NAME " v. " VERSION "\n");
    return 0;
}

/*! Read stdin data into `textarg`.
    This only reads up to `length - 1` characters.
*/
char* read_stdin_arg(void) {
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


/*! Checks `nametype` for TYPE_INVALID*, and prints the usage string
    with a warning message if it is invalid.
    If the code is valid, it simply returns `true`.
*/
bool validate_color_arg(ColorArg carg, const char* name) {
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
