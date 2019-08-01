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
            return EXIT_FAILURE; \
        } \
    } while (0)


int main(int argc, char* argv[]) {
    // Needed for str_to_wide(), and wide_to_str(), and the rainbow() funcs.
    setlocale(LC_ALL, "");
    ColrToolOptions opts = ColrToolOptions_new();
    int parse_ret = parse_args(argc, argv, &opts);
    // print_opts_repr(opts);
    // Any non-negative return means we should stop right here.
    if (parse_ret >= 0) return parse_ret;

    ColorText* ctext = NULL;
    // Rainbowize the text arg.
    // TODO: Let ColorText/colr handle "rainbow" as an official color name.
    if (opts.rainbow_fore || opts.rainbow_back) {
        ctext = rainbowize(&opts);
    } else {
        // Colorize text using fore, back, or style.
        ctext = Colr(
            opts.text,
            opts.fore,
            opts.back,
            opts.style
        );
    }
    return_error_if_null(ctext, "Failed to allocate for ColorText!\n");

    ctext->just = opts.just;
    dbug_repr("Using", *ctext);
    char* text = ColorText_to_str(*ctext);
    if (!text) {
        ColorText_free(ctext);
        return_error("Failed to allocate for colorized string!\n");
    }
    ColorText_free(ctext);
    printf("%s\n", text);
    free(text);
    if (opts.free_text) free(opts.text);
    return EXIT_SUCCESS;
}

ColrToolOptions ColrToolOptions_new(void) {
    return (ColrToolOptions){
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .just=ColorJustify_empty(),
        .filepath=NULL,
        .free_text=false,
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

char* ColrToolOptions_repr(ColrToolOptions opts) {
    char* text_repr = opts.text ? colr_repr(opts.text) : NULL;
    char* fore_repr = opts.fore ? colr_repr(*(opts.fore)) : NULL;
    char* back_repr = opts.back ? colr_repr(*(opts.back)) : NULL;
    char* style_repr = opts.style ? colr_repr(*(opts.style)) : NULL;
    char* file_repr = opts.filepath ? colr_repr(opts.filepath) : NULL;
    char* just_repr = colr_repr(opts.just);
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "ColrToolOptions(\n\
    .text=%s,\n\
    .fore=%s,\n\
    .back=%s,\n\
    .style=%s,\n\
    .just=%s,\n\
    .filepath=%s,\n\
    .free_text=%s,\n\
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
        just_repr ? just_repr : "<couldn't allocate repr>",
        file_repr ? file_repr : "NULL",
        ct_bool_str(opts.free_text),
        ct_bool_str(opts.rainbow_fore),
        ct_bool_str(opts.rainbow_back),
        ct_bool_str(opts.print_back),
        ct_bool_str(opts.print_256),
        ct_bool_str(opts.print_basic),
        ct_bool_str(opts.print_rgb),
        ct_bool_str(opts.print_rgb_term),
        ct_bool_str(opts.print_rainbow)
    );
    free(text_repr);
    free(fore_repr);
    free(back_repr);
    free(style_repr);
    free(file_repr);
    free(just_repr);
    return repr;
}

/*! Checks to see if a directory path exists.

    \pi dirpath File path to check.
    \return      `true` if the dirpath is not `NULL`/empty and exists, otherwise `false`.
*/
bool dir_exists(const char* dirpath) {
    if (!dirpath || dirpath[0] == '\0') return false;
    struct stat st;
    if (stat(dirpath, &st) < 0) return false;

    return ((st.st_mode & S_IFMT) == S_IFDIR);
}

/*! Checks to see if a file path exists.

    \pi filepath File path to check.
    \return      `true` if the filepath is not `NULL`/empty and exists, otherwise `false`.
*/
bool file_exists(const char* filepath) {
    if (!filepath || filepath[0] == '\0') return false;
    struct stat st;
    if (stat(filepath, &st) < 0) return false;

    return ((st.st_mode & S_IFMT) == S_IFREG);
}

/*! Parse user arguments into a ColrToolOptions struct.

    \details
    This handles long-only options like --basic, --rainbow, and intializes
    the ColrToolOptions with usable (possibly empty) ColorArgs for fore, back,
    and style. It also handles reading stdin, or the file argument, and sets
    `.text` to the correct content to be colorized.

    \pi argc Argument count from `main()`.
    \pi argv Arguments from `main()`.
    \po opts A ColrToolOptions struct to modify.

    \returns `-1` on success (text to colorize). Any return `> -1` means that
             the program should stop, and exit with that value as the exit code.
*/
int parse_args(int argc, char** argv, ColrToolOptions* opts) {
    char* unknownmsg = NULL;
    // Tell getopt that I'll handle the bad-argument messages.
    opterr = 0;

    while (1) {
        int c;
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"version", no_argument, 0, 'v'},
            {"fore", required_argument, 0,  'f'},
            {"back", required_argument, 0, 'b'},
            {"style", required_argument, 0, 's'},
            {"file", required_argument, 0, 'F'},
            {"ljust", required_argument, 0, 'l'},
            {"rjust", required_argument, 0, 'r'},
            {"center", required_argument, 0, 'c'},
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

        c = getopt_long(argc, argv, "hvF:f:b:s:l:r:c:", long_options, &option_index);
        if (c == -1) break;

        int justargval;
        switch (c) {
            case 0:
                if (colr_streq(long_options[option_index].name, "basic")) {
                    opts->print_basic = true;
                    return print_basic(false);
                } else if (colr_streq(long_options[option_index].name, "256")) {
                    opts->print_256 = true;
                    return print_256(false);
                } else if (colr_streq(long_options[option_index].name, "rainbow")) {
                    opts->print_rainbow = true;
                    return print_rainbow(false);
                } else if (colr_streq(long_options[option_index].name, "rgb")) {
                    opts->print_rgb = true;
                    return print_rgb(false, false);
                } else if (colr_streq(long_options[option_index].name, "termrgb")) {
                    opts->print_rgb_term = true;
                    return print_rgb(false, true);
                } else if (colr_streq(long_options[option_index].name, "basicbg")) {
                    opts->print_back = true;
                    opts->print_basic = true;
                    return print_basic(true);
                } else if (colr_streq(long_options[option_index].name, "256bg")) {
                    opts->print_back = true;
                    opts->print_256 = true;
                    return print_256(true);
                } else if (colr_streq(long_options[option_index].name, "rainbowbg")) {
                    opts->print_back = true;
                    opts->print_rainbow = true;
                    return print_rainbow(true);
                } else if (colr_streq(long_options[option_index].name, "rgbbg")) {
                    opts->print_back = true;
                    opts->print_rgb = true;
                    return print_rgb(true, false);
                } else if (colr_streq(long_options[option_index].name, "termrgbbg")) {
                    opts->print_back = true;
                    opts->print_rgb_term = true;
                    return print_rgb(true, true);
                } else {
                    printferr(
                        "Developer Error: Unhandled long-only option!: %s\n",
                        long_options[option_index].name
                    );
                    return EXIT_FAILURE;
                }
                break;
            case 'b':
                if (colr_streq(optarg, "rainbow")) {
                    opts->rainbow_back = true;
                    opts->back = ColorArg_to_ptr(ColorArg_empty());
                } else  {
                    opts->back = back(optarg);
                    if (!validate_color_arg(*(opts->back), optarg)) return EXIT_FAILURE;
                }
                break;
            case 'c':
                if (opts->just.method != JUST_NONE) {
                    printferr("Justification was already set with: %s\n", ct_just_arg_str(opts->just));
                    return EXIT_FAILURE;
                }
                if (!parse_int_arg(optarg, &justargval)) {
                    printferr("Invalid number for --center: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                opts->just.method = JUST_CENTER;
                opts->just.width = justargval;
                break;
            case 'F':
                if (colr_streq(optarg, "-")) {
                    // Another way to read stdin data, with --file -.
                    opts->text = "-";
                } else {
                    if (!file_exists(optarg)) {
                        printferr("File does not exist: %s\n", optarg);
                        return EXIT_FAILURE;
                    }
                    opts->filepath = optarg;
                }
                break;
            case 'f':
                if (colr_streq(optarg, "rainbow")) {
                    opts->rainbow_fore = true;
                    opts->fore = ColorArg_to_ptr(ColorArg_empty());
                } else  {
                    opts->fore = fore(optarg);
                    if (!validate_color_arg(*(opts->fore), optarg)) return EXIT_FAILURE;
                }
                break;
            case 'h':
                print_usage_full();
                return EXIT_SUCCESS;
            case 'l':
                if (opts->just.method != JUST_NONE) {
                    printferr("Justification was already set with: %s\n", ct_just_arg_str(opts->just));
                    return EXIT_FAILURE;
                }
                if (!parse_int_arg(optarg, &justargval)) {
                    printferr("Invalid number for --ljust: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                opts->just.method = JUST_LEFT;
                opts->just.width = justargval;
                break;
            case 'r':
                if (opts->just.method != JUST_NONE) {
                    printferr("Justification was already set with: %s\n", ct_just_arg_str(opts->just));
                    return EXIT_FAILURE;
                }
                if (!parse_int_arg(optarg, &justargval)) {
                    printferr("Invalid number for --rjust: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                opts->just.method = JUST_RIGHT;
                opts->just.width = justargval;
                break;
            case 's':
                opts->style = style(optarg);
                if (!validate_color_arg(*(opts->style), optarg)) return EXIT_FAILURE;
                break;
            case 'v':
                print_version();
                return EXIT_SUCCESS;
            case '?':
                asprintf_or_return(1, &unknownmsg, "Unknown argument: %c", optopt);
                print_usage(unknownmsg);
                free(unknownmsg);
                return EXIT_FAILURE;
                break;
            default:
                printferr("Unknown option!: %c\n", c);
                return EXIT_FAILURE;
        }
    }
    if (optind == argc) {
       print_usage("No text given!");
       return EXIT_FAILURE;
    }
    // Remaining non-option arguments.
    while (optind < argc) {
        if (!opts->text && !opts->filepath) {
            // If a file path is set, the text will come later.
            opts->text = argv[optind];
            optind++;
        } else if (!opts->fore) {
            if (colr_streq(argv[optind], "rainbow")) {
                opts->rainbow_fore = true;
            } else {
                opts->fore = fore(argv[optind]);
                if (!validate_color_arg(*(opts->fore), argv[optind])) return EXIT_FAILURE;
            }
            optind++;
        } else if (!opts->back) {
            opts->back = back(argv[optind]);
            if (!validate_color_arg(*(opts->back), argv[optind])) return EXIT_FAILURE;
            optind++;
        } else if (!opts->style) {
            opts->style = style(argv[optind]);
            if (!validate_color_arg(*(opts->style), argv[optind])) return EXIT_FAILURE;
            optind++;
        } else {
            print_usage("Too many arguments!");
            return EXIT_FAILURE;
        }
    }
    // Fill null args with "empty" args, so the colr macros know what to do.
    if (!opts->fore) opts->fore = ColorArg_to_ptr(ColorArg_empty());
    if (!opts->back) opts->back = ColorArg_to_ptr(ColorArg_empty());
    if (!opts->style) opts->style = ColorArg_to_ptr(ColorArg_empty());

    // Fill text with stdin if a marker argument was used.
    if (colr_streq(opts->text, "-")) {
        // Read from stdin.
        opts->text = read_stdin_arg();
        opts->free_text = true;
        if (!opts->text) {
            printferr("\nFailed to allocate for stdin data!\n");
            return EXIT_FAILURE;
        }
    } else if (opts->filepath) {
        // Read from file.
        opts->text = read_file_arg(opts->filepath);
        opts->free_text = true;
        if (!opts->text) {
            printferr("\nFailed to allocate for file data!\n");
            return EXIT_FAILURE;
        }
    }
    return -1;
}

/*! Parse a user argument as an integer, and set `value` to the parsed integer
    value.

    \details
    On error, a message is printed and `false` is returned.

    \pi s        A string to parse.
    \po value    Pointer to an int, to set the final integer value.
    \return      `true` on success, with `value` set, otherwise `false`.
*/
bool parse_int_arg(const char* s, int* value) {
    if (!s || s[0] == '\0') return false;
    return sscanf(s, "%d", value) == 1;
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
    return EXIT_SUCCESS;
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
        asprintf_or_return(1, &namefmt, "%-14s", name);
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
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
             [-c num | -l num | -r num]\n\
        colr [-F file] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
             [-c num | -l num | -r num]\n\
    ", NAME, VERSION);
    return EXIT_SUCCESS;
}


/*! Print the usage string.
*/
int print_usage_full() {
    print_usage(NULL);
    printf("\
\n\
    Commands:\n\
        --basic[bg]          : Print all basic color names and colors.\n\
        --256[bg]            : Print all extended color names and colors.\n\
        --rainbow[bg]        : Print a rainbow example.\n\
        --rgb[bg]            : Print some rgb codes.\n\
        --termrgb[bg]        : Print some 256-compatible rgb codes.\n\
\n\
    If 'bg' is appended to a command argument, back colors will be used.\n\
\n\
    Options:\n\
        TEXT                 : Text to colorize.\n\
                               Use \"-\" or \"-F -\" to force stdin.\n\
                               Default: stdin\n\
        FORE                 : Fore color name/value for text.\n\
                               If set to 'rainbow', the text will be rainbowized.\n\
        BACK                 : Back color name/value for text.\n\
                               If set to 'rainbow', the back colors will be rainbowized.\n\
        STYLE                : Style name for text.\n\
        -b val,--back val    : Specify the back color explicitly, in any order.\n\
        -c num,--center num  : Center-justify the resulting text using the specified width.\n\
                               If \"0\" is given, the terminal-width will be used.\n\
        -F file,--file file  : Read text from a file.\n\
                               Use \"-F -\" to force stdin.\n\
        -f val,--fore val    : Specify the fore color explicitly, in any order.\n\
        -h, --help           : Print this message and exit.\n\
        -l num,--ljust num   : Left-justify the resulting text using the specified width.\n\
                               If \"0\" is given, the terminal-width will be used.\n\
        -r num,--rjust num   : Right-justify the resulting text using the specified width.\n\
                               If \"0\" is given, the terminal-width will be used.\n\
        -s val,--style val   : Specify the style explicitly, in any order.\n\
        -v, --version        : Show version and exit.\n\
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
    return EXIT_SUCCESS;
}

/*! Print the ColrC version.
*/
int print_version(void) {
    printf(NAME " v. " VERSION "\n");
    return EXIT_SUCCESS;
}

/*! Rainbowize some text with options from ColrToolOptions.

    \pi opts An initialized ColrToolOptions.
    \return  An allocated ColorText, with a rainbowized `.text` member.
*/
ColorText* rainbowize(ColrToolOptions* opts) {
   bool do_term_rainbow = !colr_supports_rgb();
    // TODO: User args for these freq and offet values.
    double freq = 0.1;
    double offset = 3;
    char* rainbowized = (
        do_term_rainbow ?
            (opts->rainbow_fore ?
                rainbow_fg_term(opts->text, freq, offset) :
                rainbow_bg_term(opts->text, freq, offset)
            ) :
            (opts->rainbow_fore ?
                rainbow_fg(opts->text, freq, offset) :
                rainbow_bg(opts->text, freq, offset)
            )
    );
    // Text was allocated from stdin input, it's safe to free.
    if (opts->free_text) {
        free(opts->text);
        opts->text = NULL;
    }
    // Some or all of the fore/back/style args are "empty" (not null).
    // They will not be used if they are empty, but they will be free'd.
    return Colr(
        rainbowized,
        opts->fore,
        opts->back,
        opts->style
    );
}

/*! Read file data and return an allocated string.
*/
char* read_file(FILE* fp) {
    size_t line_length = 1024;
    char line[line_length];
    size_t buffer_length = 4096;
    char* buffer = NULL;
    size_t total = 0;
    while ((fgets(line, line_length, fp))) {
        total += strlen(line);
        if (!buffer) {
            // First line.
            buffer = calloc(line_length, sizeof(char));
        } else if (total >= buffer_length) {
            buffer_length *= 2;
            buffer = realloc(buffer, buffer_length);
        }
        strcat(buffer, line);
    }
    if (ferror(fp)) {
        perror("Cannot read file");
        return NULL;
    }
    if (fp != stdin) fclose(fp);
    return buffer;
}
/*! Read file data, and return an allocated string.

    \pi filepath The file path to read.
    \return      An allocated string with the file data on success, or `NULL`
                 on error (possibly with an error message printed).
*/
char* read_file_arg(const char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        perror("Cannot open file");
        return NULL;
    }
    return read_file(fp);
}

/*! Read stdin data and return an allocated string.
*/
char* read_stdin_arg(void) {
    if (isatty(fileno(stdin)) && isatty(fileno(stderr))) {
        dbug("\nReading from stdin until EOF (Ctrl + D)...\n");
    }
    return read_file(stdin);
}


/*! Checks `nametype` for TYPE_INVALID*, and prints the usage string
    with a warning message if it is invalid.
    If the code is valid, it simply returns `true`.
*/
bool validate_color_arg(ColorArg carg, const char* name) {
    if (!name) {
        #if defined(DEBUG) && defined(dbug)
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
            asprintf_or_return(
                false,
                &errmsg,
                "Invalid range (0-255) for %s RGB color: %s",
                argtype,
                name
            );
            break;
        case TYPE_INVALID_EXTENDED_RANGE:
            asprintf_or_return(
                false,
                &errmsg,
                "Invalid range (0-255) for extended %s color: %s",
                argtype,
                name
            );
            break;
        case TYPE_INVALID:
            asprintf_or_return(
                false,
                &errmsg,
                "Invalid %s color name: %s",
                argtype,
                name
            );
            break;
        case TYPE_INVALID_STYLE:
            asprintf_or_return(
                false,
                &errmsg,
                "Invalid style name: %s",
                name
            );
            break;
        default:
            // Valid color arg passed.
            return true;
    }

    // Print the error message that was built.
    print_usage(errmsg);
    free(errmsg);
    return false;
}

#endif // DOXYGEN_SKIP
