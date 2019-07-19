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
    ColrToolOptions colr_opts = ColrToolOptions_new();
    if (!parse_args(argc, argv, &colr_opts)) return 1;

    if (colr_opts.print_basic) return print_basic(colr_opts.print_back);
    if (colr_opts.print_256) return print_256(colr_opts.print_back);
    if (colr_opts.print_rgb) return print_rgb(colr_opts.print_back);
    if (colr_opts.print_rainbow) return print_rainbow_fore();
    bool free_text = false;
    if (colr_streq(colr_opts.text, "-")) {
        // Read from stdin.
        colr_opts.text = read_stdin_arg();
        if (!colr_opts.text) {
            printferr("\nFailed to allocate for stdin data!\n");
            return 1;
        }
        free_text = true;
    }
    // Rainbowize the text arg.
    bool do_term_rainbow = false;
    if (colr_opts.rainbow_fore) {
        do_term_rainbow = !colr_supports_rgb();
        char* rainbowized = (
            do_term_rainbow ?
            rainbow_fg_term(colr_opts.text, 0.1, 3) :
            rainbow_fg(colr_opts.text, 0.1, 3)
        );
        // TODO: Stylized rainbows, with optional back colors.
        printf("%s\n", rainbowized);
        free(rainbowized);
        return 0;
    }

    ColorText *ctext = Colr(
        colr_opts.text,
        colr_opts.fore,
        colr_opts.back,
        colr_opts.style
    );
    dbug_repr("ColorText: %s\n", *ctext);
    char* text = ColorText_to_str(*ctext);
    printf("%s\n", text);
    free(text);
    if (free_text) free(colr_opts.text);
    return 0;
}

ColrToolOptions ColrToolOptions_new(void) {
    return (ColrToolOptions){
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .print_back=false,
        .print_256=false,
        .print_basic=false,
        .print_rainbow=false,
        .print_rgb=false,
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
    .print_back=%s,\n\
    .print_256=%s,\n\
    .print_basic=%s,\n\
    .print_rgb=%s,\n\
    .print_rainbow=%s,\n\
)",
        text_repr ? text_repr : "NULL",
        fore_repr ? fore_repr : "NULL",
        back_repr ? back_repr : "NULL",
        style_repr ? style_repr : "NULL",
        colropts.rainbow_fore ? "true" : "false",
        colropts.print_back ? "true" : "false",
        colropts.print_256 ? "true" : "false",
        colropts.print_basic ? "true" : "false",
        colropts.print_rgb ? "true" : "false",
        colropts.print_rainbow ? "true" : "false"
    );
    if (text_repr) free(text_repr);
    if (fore_repr) free(fore_repr);
    if (back_repr) free(back_repr);
    if (style_repr) free(style_repr);

    return repr;
}


bool parse_args(int argc, char** argv, ColrToolOptions* colropts) {
    int c;
    char* unknownmsg = NULL;

    // Tell getopt that I'll handle the bad-argument messages.
    opterr = 0;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
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
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "hf:b:s:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                if (colr_streq(long_options[option_index].name, "basic")) {
                    colropts->print_basic = true;
                } else if (colr_streq(long_options[option_index].name, "256")) {
                    colropts->print_256 = true;
                } else if (colr_streq(long_options[option_index].name, "rainbow")) {
                    colropts->print_rainbow = true;
                } else if (colr_streq(long_options[option_index].name, "rgb")) {
                    colropts->print_rgb = true;
                } else if (colr_streq(long_options[option_index].name, "basicbg")) {
                    colropts->print_back = true;
                    colropts->print_basic = true;
                } else if (colr_streq(long_options[option_index].name, "256bg")) {
                    colropts->print_back = true;
                    colropts->print_256 = true;
                } else if (colr_streq(long_options[option_index].name, "rainbowbg")) {
                    colropts->print_back = true;
                    colropts->print_rainbow = true;
                } else if (colr_streq(long_options[option_index].name, "rgbbg")) {
                    colropts->print_back = true;
                    colropts->print_rgb = true;
                } else {
                    printferr(
                        "Unhandled long-only option!: %s\n",
                        long_options[option_index].name
                    );
                }
                break;

            case 'h':
                print_usage_full();
                return false;
            case 'f':
                if (colr_streq(optarg, "rainbow")) {
                    colropts->rainbow_fore = true;
                } else  {
                    colropts->fore = fore(optarg);
                    if (!validate_color_arg(*(colropts->fore), optarg)) return false;
                }
                break;
            case 'b':
                colropts->back = back(optarg);
                if (!validate_color_arg(*(colropts->back), optarg)) return false;
                break;

            case 's':
                colropts->style = style(optarg);
                if (!validate_color_arg(*(colropts->style), optarg)) return false;
                break;
            case '?':
                asprintf(&unknownmsg, "Unknown argument: %c", optopt);
                print_usage(unknownmsg);
                free(unknownmsg);
                return false;
                break;
            default:
                printferr("Unknown option!: %c\n", c);
                return false;
        }
    }
    if (optind == argc) {
       print_usage("No text given!");
       return false;
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
                if (!validate_color_arg(*(colropts->fore), argv[optind])) return false;
            }
            optind++;
        } else if (!(colropts->back)) {
            colropts->back = back(argv[optind]);
            if (!validate_color_arg(*(colropts->back), argv[optind])) return false;
            optind++;
        } else if (!(colropts->style)) {
            colropts->style = style(argv[optind]);
            if (!validate_color_arg(*(colropts->style), argv[optind])) return false;
            optind++;
        } else {
            print_usage("Too many arguments!");
            return false;
        }
    }
    return true;
}


/*! Print the 256 color range using either colrfgx or colorbgx.
    The function choice is passed as an argument.
*/
int print_256(bool do_back) {
    char num[4];
    ColorArg carg;
    char* text;
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back_arg(ext(i)) : fore_arg(ext(i));
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
                carg = do_back ? back_arg(ext(i)) : fore_arg(ext(i));
                text = ColorArg_to_str(carg);
                printf("%s ", text);
                free(text);
            }
            puts("\n");
        }
    }
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back_arg(ext(i)) : fore_arg(ext(i));
        text = ColorArg_to_str(carg);
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
int print_rainbow_fore(void) {
    char text[] = "This is a demo of the rainbow function.";
    char* textfmt =colr_supports_rgb() ? rainbow_fg(text, 0.1, 3) : rainbow_fg_term(text, 0.1, 3);
    printf("%s\n", textfmt);
    free(textfmt);
    return 0;
}

/*! Print part of the RGB range using either colrfgrgb, or .
    The function choice is passed as an argument.
*/
int print_rgb(bool do_back) {
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
                RGB othervals = do_back ? (RGB){255, 255, 255} : (RGB){0, 0, 0};
                if (do_back) {
                    text = colr(back(vals), fore(othervals), num);
                } else {
                    text = colr(fore(vals), back(othervals), num);
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


/*! Print the short usage string with optional `reason`
*/
int print_usage(const char* reason) {
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


/*! Print the usage string.
*/
int print_usage_full() {
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
