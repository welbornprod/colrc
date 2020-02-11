#ifndef DOXYGEN_SKIP
//! \file

/*! Example implementation using colr.h, and a command-line tool
    to colorize text.

    \author Christopher Welborn
    \date 02-05-2017
*/
#include "colr_tool.h"


int main(int argc, char* argv[]) {
    ColrOpts opts = ColrOpts_new();
    int parse_ret = parse_args(argc, argv, &opts);
    // print_opts_repr(opts);
    // Any non-negative return means we should stop right here.
    if (parse_ret >= 0) {
        ColrOpts_cleanup(&opts);
        return parse_ret;
    }
    // Non-colorizing/formatting options.
    if (opts.list_codes) {
        return run_colr_cmd(list_codes, &opts);
    } else if (opts.strip_codes) {
        return run_colr_cmd(strip_codes, &opts);
    } else if (opts.translate_code) {
        return run_colr_cmd(translate_code, &opts);
    } else if (opts.is_disabled) {
        // No need to colorize anything, colors were disabled. We are `cat` now.
        return run_colr_cmd(print_plain, &opts);
    }

    ColorText* ctext = NULL;
    // Rainbowize the text arg.
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
    // Both types of ColorText allocation may have failed.
    if (!ctext) {
        ColrOpts_free_text(&opts);
        return_error("Failed to allocate for ColorText!\n");
    }

    ctext->just = opts.just;
    // dbug_repr("Using", *ctext);
    char* text = ColorText_to_str(*ctext);
    if (opts.free_colr_text) free(ctext->text);
    if (!text) {
        ColorText_free(ctext);
        return_error("Failed to allocate for colorized string!\n");
    }
    ColorText_free(ctext);
    if (text[0] == '\0') {
        printferr("No text to work with.\n");
        free(text);
        if (opts.free_text) free(opts.text);
        return EXIT_FAILURE;
    }
    fprintf(opts.out_stream, "%s", text);
    bool has_newline = (
        colr_str_ends_with(text, "\n") ||
        colr_str_ends_with(text, "\n" CODE_RESET_ALL)
    );
    if (!has_newline) {
        fprintf(opts.out_stream, "\n");
    }

    free(text);
    ColrOpts_free_text(&opts);
    return EXIT_SUCCESS;
}

/*! Free the ColorArgs and text used by colr-tool, if needed, and set them
    to `NULL`.

    \details
    This is used for clean-up before an early return (failures).
    \po opts ColrOpts to get the ColorArgs/text/options from.
*/
void ColrOpts_cleanup(ColrOpts* opts) {
    ColrOpts_free_args(opts);
    ColrOpts_free_text(opts);
}

/*! Free the ColorArgs used by colr-tool, if needed, and set them to `NULL`.

    \po opts ColrOpts to get the ColorArgs/options from.
*/
void ColrOpts_free_args(ColrOpts* opts) {
    if (opts->fore) {
        free(opts->fore);
        opts->fore = NULL;
    }
    if (opts->back) {
        free(opts->back);
        opts->back = NULL;
    }
    if (opts->style) {
        free(opts->style);
        opts->style = NULL;
    }
}

/*! Free the text used by colr-tool, if needed, and set it to `NULL`.

    \pi opts ColrOpts to get the text/options from.
*/
void ColrOpts_free_text(ColrOpts* opts) {
    if (opts->text && opts->free_text) {
        free(opts->text);
        opts->text = NULL;
        // This function will not try to double-free the text.
        opts->free_text = false;
    }
}

/*! Create a ColrOpts with all of the default values set.

    \return An initialized ColrOpts, with defaults set.
*/
ColrOpts ColrOpts_new(void) {
    return (ColrOpts){
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .just=ColorJustify_empty(),
        .filepath=NULL,
        .free_text=false,
        .free_colr_text=false,
        .rainbow_fore=false,
        .rainbow_back=false,
        .rainbow_term=false,
        .rainbow_freq=CT_DEFAULT_FREQ,
        .rainbow_offset=CT_DEFAULT_OFFSET,
        .rainbow_spread=CT_DEFAULT_SPREAD,
        .auto_disable=false,
        .is_disabled=false,
        .list_codes=false,
        .list_unique_codes=false,
        .strip_codes=false,
        .translate_code=false,
        .out_stream=stdout,
    };
}

/*! Create a string representation for ColrOpts.

    \details
    This is used in debugging arg parsing.

    \pi opts The ColrOpts to get the representation for.
    \return  An allocated string with the result, or `NULL` if allocation failed.\n
             \mustfree
*/
char* ColrOpts_repr(ColrOpts opts) {
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
        "ColrOpts(\n\
    .text=%s,\n\
    .fore=%s,\n\
    .back=%s,\n\
    .style=%s,\n\
    .just=%s,\n\
    .filepath=%s,\n\
    .free_text=%s,\n\
    .free_colr_text=%s,\n\
    .rainbow_fore=%s,\n\
    .rainbow_back=%s,\n\
    .rainbow_term=%s,\n\
    .rainbow_freq=%lf,\n\
    .rainbow_offset=%lu,\n\
    .rainbow_spread=%lu,\n\
    .auto_disable=%s,\n\
    .is_disabled=%s,\n\
    .list_codes=%s,\n\
    .list_unique_codes=%s,\n\
    .strip_codes=%s,\n\
    .translate_code=%s,\n\
    .out_stream=%s,\n\
)",
        text_repr ? text_repr : "NULL",
        fore_repr ? fore_repr : "NULL",
        back_repr ? back_repr : "NULL",
        style_repr ? style_repr : "NULL",
        just_repr ? just_repr : "<couldn't allocate repr>",
        file_repr ? file_repr : "NULL",
        bool_str(opts.free_text),
        bool_str(opts.free_colr_text),
        bool_str(opts.rainbow_fore),
        bool_str(opts.rainbow_back),
        bool_str(opts.rainbow_term),
        opts.rainbow_freq,
        opts.rainbow_offset,
        opts.rainbow_spread,
        bool_str(opts.auto_disable),
        bool_str(opts.is_disabled),
        bool_str(opts.list_codes),
        bool_str(opts.list_unique_codes),
        bool_str(opts.strip_codes),
        bool_str(opts.translate_code),
        stream_name(opts.out_stream)
    );
    free(text_repr);
    free(fore_repr);
    free(back_repr);
    free(style_repr);
    free(file_repr);
    free(just_repr);
    return repr;
}

/*! Set `.is_disabled` if `.auto_disable` is set and `.out_stream` is not a tty.

    \pi opts ColrOpts to get the settings\FILE from.
    \return  The value of `opts.is_disabled` after setting it.
*/
bool ColrOpts_set_disabled(ColrOpts* opts) {
    opts->is_disabled = (
        opts->auto_disable &&
        !isatty(fileno(opts->out_stream))
    );
    return opts->is_disabled;
}

/*! Get or set the text to work with, based on options.

    \details
    If `opts->filepath` is not `NULL`, it is read from a file.

    \details
    If `opts->text == "-"`, it is read from stdin.

    \details
    If processing-options are set (`.list_codes`, `.strip_codes`, `.translate_code`),
    the default is to read from stdin.

    \po opts A ColrOpts to set the text for.
    \return  `true` if text was set (or already set), otherwise `false`.
*/
bool ColrOpts_set_text(ColrOpts* opts) {
    if (opts->filepath) {
        // Read from file.
        opts->text = read_file_arg(opts->filepath);
        opts->free_text = true;
        if (!opts->text) {
            printferr("\nFailed to allocate for file data!\n");
            return false;
        }
        return true;
    }
    if (colr_str_starts_with(opts->text, "\\-")) {
        // User has some text that starts with "-", and they escaped it to
        // bypass argument parsing. Try to do the right thing and strip the
        // backslash.
        opts->text = opts->text + 1;
    }
    // Do stdin if explicitly requested.
    bool do_stdin = colr_str_eq(opts->text, "-");
    // Do stdin if there is no text, and one of the commands is being used.
    bool is_cmd = (opts->list_codes || opts->strip_codes || opts->translate_code);
    if (!opts->text && is_cmd) do_stdin = true;

    if (do_stdin) {
        // Fill text with stdin if a marker argument was used.
        // Read from stdin.
        opts->text = read_stdin_arg();
        opts->free_text = true;
        if (!opts->text) {
            printferr("\nFailed to allocate for stdin data!\n");
            return false;
        }
    }
    return opts->text ? true : false;
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
    \return      `true` if the filepath is not `NULL`/empty and is readable, otherwise `false`.
*/
bool file_exists(const char* filepath) {
    if (!filepath || filepath[0] == '\0') return false;
    return (access(filepath, R_OK) == 0);
}

/*! List all escape-codes found in the text and return an exit status code.

    \pi opts Pointer to ColrOpts to get the text/options from.
    \return  `EXIT_SUCCESS` on success, otherwise `EXIT_FAILURE`
*/
int list_codes(ColrOpts* opts) {
    if (!opts->text) {
        printferr("\nNo text to examine!\n");
        return EXIT_FAILURE;
    } else if (opts->text[0] == '\0') {
        printferr("\nText was empty!\n");
        return EXIT_FAILURE;
    }
    ColorArg** carg_list = ColorArgs_from_str(opts->text, opts->list_unique_codes);

    if (!carg_list) {
        printferr("\nNo codes found.\n");
        return EXIT_FAILURE;
    }
    // Iterate over the ColorArg list.
    for (size_t i = 0; carg_list[i]; i++) {
        char* carg_example = ColorArg_example(
            *(carg_list[i]),
            !opts->is_disabled
        );
        if (!carg_example) continue;
        fprintf(opts->out_stream, "%s\n", carg_example);
        free(carg_example);
    }
    // Free the ColorArgs, and the list of pointers.
    ColorArgs_list_free(carg_list);
    return EXIT_SUCCESS;
}

int parse_arg_char(char** argv, const char* long_name, const char c, ColrOpts* opts) {
    int argval_just;
    double argval_freq;
    size_t argval_offset;
    size_t argval_spread;

    switch (c) {
        case 0:
            return parse_arg_long(long_name, opts);
            break;
        case 'a':
            opts->auto_disable = true;
            break;
        case 'b':
            if (opts->back) {
                // Happens when --rainbow is used.
                printferr("BACK was set with --rainbow.\n");
                return EXIT_FAILURE;
            }
            if (colr_str_either(optarg, "rainbow", "rainbowterm")) {
                opts->rainbow_term = colr_str_eq(optarg, "rainbowterm");
                opts->rainbow_back = true;
                opts->back = ColorArg_to_ptr(ColorArg_empty());
            } else  {
                opts->back = back(optarg);
                if (!validate_color_arg(*(opts->back), optarg)) return EXIT_FAILURE;
            }
            break;
        case 'c':
            if (opts->just.method != JUST_NONE) {
                printferr("Justification was already set with: %s\n", just_arg_str(opts->just));
                return EXIT_FAILURE;
            }
            if (!parse_int_arg(optarg, &argval_just)) {
                printferr("Invalid number for --center: %s\n", optarg);
                return EXIT_FAILURE;
            }
            opts->just.method = JUST_CENTER;
            opts->just.width = argval_just;
            break;
        case 'e':
            opts->out_stream = stderr;
            break;
        case 'F':
            if (colr_str_eq(optarg, "-")) {
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
            if (opts->fore) {
                // Happens when --rainbow is used.
                printferr("FORE was set with --rainbow.\n");
                return EXIT_FAILURE;
            }
            if (colr_str_either(optarg, "rainbow", "rainbowterm")) {
                opts->rainbow_term = colr_str_eq(optarg, "rainbowterm");
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
                printferr("Justification was already set with: %s\n", just_arg_str(opts->just));
                return EXIT_FAILURE;
            }
            if (!parse_int_arg(optarg, &argval_just)) {
                printferr("Invalid number for --ljust: %s\n", optarg);
                return EXIT_FAILURE;
            }
            opts->just.method = JUST_LEFT;
            opts->just.width = argval_just;
            break;
        case 'o':
            if (!parse_size_arg(optarg, &argval_offset)) {
                printferr("Invalid value for --offset: %s\n", optarg);
                return EXIT_FAILURE;
            }
            opts->rainbow_offset = argval_offset;
            break;
        case 'q':
            if (!parse_double_arg(optarg, &argval_freq)) {
                printferr("Invalid value for --frequency: %s\n", optarg);
                return EXIT_FAILURE;
            }
            // Clamp the value, because it produces ugly results.
            if (argval_freq < CT_MIN_FREQ) {
                argval_freq = CT_MIN_FREQ;
            } else if (argval_freq > CT_MAX_FREQ) {
                argval_freq = CT_MAX_FREQ;
            }
            opts->rainbow_freq = argval_freq;
            break;
        case 'R':
            if (!opts->fore)  {
                opts->rainbow_fore = true;
                opts->fore = ColorArg_to_ptr(ColorArg_empty());
            } else if (!opts->back) {
                opts->rainbow_back = true;
                opts->back = ColorArg_to_ptr(ColorArg_empty());
            } else {
                printferr("FORE and BACK are already set, can't use --rainbow.\n");
                return EXIT_FAILURE;
            }
            break;
        case 'r':
            if (opts->just.method != JUST_NONE) {
                printferr("Justification was already set with: %s\n", just_arg_str(opts->just));
                return EXIT_FAILURE;
            }
            if (!parse_int_arg(optarg, &argval_just)) {
                printferr("Invalid number for --rjust: %s\n", optarg);
                return EXIT_FAILURE;
            }
            opts->just.method = JUST_RIGHT;
            opts->just.width = argval_just;
            break;
        case 's':
            opts->style = style(optarg);
            if (!validate_color_arg(*(opts->style), optarg)) return EXIT_FAILURE;
            break;
        case 't':
            opts->translate_code = true;
            break;
        case 'u':
            opts->list_codes = true;
            opts->list_unique_codes = true;
            break;
        case 'v':
            print_version();
            return EXIT_SUCCESS;
        case 'w':
            if (!parse_size_arg(optarg, &argval_spread)) {
                printferr("Invalid value for --spread: %s\n", optarg);
                return EXIT_FAILURE;
            }
            opts->rainbow_spread = argval_spread;
            break;

        case 'x':
            opts->strip_codes = true;
            break;
        case 'z':
            opts->list_codes = true;
            break;
        case '?':
            print_usage_errmsg("Unknown argument: -%c", optopt);
            return EXIT_FAILURE;
        case ':':
            print_usage_errmsg("Missing value for: %s", argv[optind - 1]);
            return EXIT_FAILURE;
        default:
            print_usage_errmsg("Unknown option!: %c\n", c);
            return EXIT_FAILURE;
    }
    // The argument was okay.
    return -1;
}

int parse_arg_long(const char* long_name, ColrOpts* opts) {
    if (colr_str_eq(long_name, "basic")) {
        return print_basic(opts, false);
    } else if (colr_str_eq(long_name, "256")) {
        return print_256(opts, false);
    } else if (colr_str_eq(long_name, "names")) {
        return print_names(opts, false);
    } else if (colr_str_eq(long_name, "rainbowize")) {
        return print_rainbow(opts, false);
    } else if (colr_str_eq(long_name, "rgb")) {
        return print_rgb(opts, false, false);
    } else if (colr_str_eq(long_name, "rgbterm")) {
        return print_rgb(opts, false, true);
    } else if (colr_str_eq(long_name, "basicbg")) {
        return print_basic(opts, true);
    } else if (colr_str_eq(long_name, "256bg")) {
        return print_256(opts, true);
    } else if (colr_str_eq(long_name, "namesrgb")) {
        return print_names(opts, true);
    } else if (colr_str_eq(long_name, "rainbowizebg")) {
        return print_rainbow(opts, true);
    } else if (colr_str_eq(long_name, "rgbbg")) {
        return print_rgb(opts, true, false);
    } else if (colr_str_eq(long_name, "rgbtermbg")) {
        return print_rgb(opts, true, true);
    }
    // If I didn't handle every long-option case something is very wrong.
    printferr(
        "Developer Error: Unhandled long-only option!: %s\n",
        long_name
    );
    return EXIT_FAILURE;
}
/*! Parse user arguments into a ColrOpts struct.

    \details
    This handles long-only options like --basic, --rainbowize, and intializes
    the ColrOpts with usable (possibly empty) ColorArgs for fore, back,
    and style. It also handles reading stdin, or the file argument, and sets
    `.text` to the correct content to be colorized.

    \pi argc Argument count from `main()`.
    \pi argv Arguments from `main()`.
    \po opts A ColrOpts struct to modify.

    \returns `-1` on success (text to colorize). Any return `> -1` means that
             the program should stop, and exit with that value as the exit code.
*/
int parse_args(int argc, char** argv, ColrOpts* opts) {
    // Tell getopt that I'll handle the bad-argument messages.
    opterr = 0;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        // Colorizing/formatting options.
        {"fore", required_argument, 0,  'f'},
        {"back", required_argument, 0, 'b'},
        {"style", required_argument, 0, 's'},
        {"file", required_argument, 0, 'F'},
        {"ljust", required_argument, 0, 'l'},
        {"rjust", required_argument, 0, 'r'},
        {"center", required_argument, 0, 'c'},
        {"rainbow", no_argument, 0, 'R'},
        // Commands
        {"listcodes", no_argument, 0, 'z'},
        {"stripcodes", no_argument, 0, 'x'},
        {"translate", no_argument, 0, 't'},
        // Command options.
        {"auto-disable", no_argument, 0, 'a'},
        {"err", no_argument, 0, 'e'},
        {"unique", no_argument, 0, 'u'},
        // Rainbow options.
        {"frequency", required_argument, 0, 'q'},
        {"offset", required_argument, 0, 'o'},
        {"spread", required_argument, 0, 'w'},
        // Example Commands.
        {"basic", no_argument, 0, 0 },
        {"basicbg", no_argument, 0, 0 },
        {"256", no_argument, 0, 0},
        {"256bg", no_argument, 0, 0},
        {"names", no_argument, 0, 0},
        {"namesrgb", no_argument, 0, 0},
        {"rainbowize", no_argument, 0, 0},
        {"rainbowizebg", no_argument, 0, 0},
        {"rgb", no_argument, 0, 0},
        {"rgbbg", no_argument, 0, 0},
        {"rgbterm", no_argument, 0, 0},
        {"rgbtermbg", no_argument, 0, 0},
        {0, 0, 0, 0}
    };
    while (1) {
        int c;
        int option_index = 0;
        c = getopt_long(
            argc,
            argv,
            ":aehRtuvxzb:c:F:f:l:o:q:r:s:w:",
            long_options,
            &option_index
        );
        if (c == -1) break;

        int ret = parse_arg_char(argv, long_options[option_index].name, c, opts);
        if (ret > -1) {
            // Bad argument, or other failure.
            return ret;
        }
    }
    // Remaining non-option arguments.
    bool no_colr_opts = (
        opts->list_codes ||
        opts->strip_codes ||
        opts->translate_code
    );
    while (optind < argc) {
        if (!opts->text && !opts->filepath) {
            // No filepath to read from, if argv[optind] is "-" stdin will be
            // used. If argv[optind] startswith "\\-", the "\\" will be stripped.
            // This happens in ColrOpts_set_text(), below.
            opts->text = argv[optind];

            optind++;
            if (no_colr_opts) {
                // No color options are needed for this operation.
                break;
            }
        } else if (!opts->fore) {
            if (colr_str_eq(argv[optind], "rainbow")) {
                opts->rainbow_fore = true;
            } else if (colr_str_eq(argv[optind], "rainbowterm")) {
                opts->rainbow_fore = true;
                opts->rainbow_term = true;
            } else {
                opts->fore = fore(argv[optind]);
                if (!validate_color_arg(*(opts->fore), argv[optind])) return EXIT_FAILURE;
            }
            optind++;
        } else if (!opts->back) {
            if (colr_str_eq(argv[optind], "rainbow")) {
                opts->rainbow_back = true;
            } else if (colr_str_eq(argv[optind], "rainbowterm")) {
                opts->rainbow_back = true;
                opts->rainbow_term = true;
            } else {
                opts->back = back(argv[optind]);
                if (!validate_color_arg(*(opts->back), argv[optind])) return EXIT_FAILURE;
            }
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

    if (!ColrOpts_set_text(opts)) {
        printferr("\nNo text to work with!\n");
        return EXIT_FAILURE;
    }
    ColrOpts_set_disabled(opts);
    return -1;
}

/*! Parse a user argument as a double, and set `value` to the parsed double
    value.

    \details
    On error, a message is printed and `false` is returned.

    \pi s        A string to parse.
    \po value    Pointer to a double, to set the final double value.
    \return      `true` on success, with `value` set, otherwise `false`.
*/
bool parse_double_arg(const char* s, double* value) {
    if (!s || s[0] == '\0') return false;
    if (sscanf(s, "%lf", value) != 1) return false;
    return true;
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

/*! Parse a user argument as a size_t, and set `value` to the parsed value.
    value.

    \details
    On error, a message is printed and `false` is returned.

    \pi s        A string to parse.
    \po value    Pointer to a size_t, to set the final integer value.
    \return      `true` on success, with `value` set, otherwise `false`.
*/
bool parse_size_arg(const char* s, size_t* value) {
    if (!s || s[0] == '\0') return false;
    return sscanf(s, "%zu", value) == 1;
}

/*! Print the 256 color range using either colrfgx or colorbgx.
    The function choice is passed as an argument.
*/
int print_256(ColrOpts* opts, bool do_back) {
    ColrOpts_set_disabled(opts);
    char num[4];
    ColorArg* carg;
    char* text;
    for (int i = 0; i < 56; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back(ext(i)) : fore(ext(i));
        if (i < 16) {
            text = colr_cat(carg, num);
            fprintf(opts->out_stream, "%s ", text);
            if ((i == 7) || (i == 15)) fprintf(opts->out_stream, "\n");
            free(text);
        } else {
            // Print the number as is.
            text = colr_cat(carg, num);
            fprintf(opts->out_stream, "%s ", text);
            free(text);
            // Print the other 5 in the group.
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                carg = do_back ? back(ext(j)) : fore(ext(j));
                text = colr_cat(carg, num);
                fprintf(opts->out_stream, "%s ", text);
                free(text);
            }
            fprintf(opts->out_stream, "\n");
        }
    }
    // Print the grayscale numbers.
    for (int i = 232; i < 256; i++) {
        snprintf(num, 4, "%03d", i);
        carg = do_back ? back(ext(i)) : fore(ext(i));
        text = colr_cat(carg, num);
        fprintf(opts->out_stream, "%s ", text);
        free(text);
    }
    fprintf(opts->out_stream, "\n");
    return EXIT_SUCCESS;
}

/*! Print basic color names and escape codes.
*/
int print_basic(ColrOpts* opts, bool do_back) {
    ColrOpts_set_disabled(opts);
    for (size_t i = 0; i < basic_names_len; i++) {
        char* namefmt = NULL;
        char* text = NULL;
        char* name = basic_names[i].name;
        BasicValue val = basic_names[i].value;
        if (colr_str_either(name, "black", "lightblack")) {
            fprintf(opts->out_stream, "\n");
        }
        BasicValue otherval = colr_str_ends_with(name, "black") ? WHITE : BLACK;
        asprintf_or_return(1, &namefmt, "%-14s", name);
        if (do_back) {
            text = colr_cat(back(val), fore(otherval), namefmt);
        } else {
            text = colr_cat(fore(val), back(otherval), namefmt);
        }
        fprintf(opts->out_stream, "%s", text);
        free(namefmt);
        free(text);
    }
    fprintf(opts->out_stream, "%s\n", CODE_RESET_ALL);
    return EXIT_SUCCESS;
}

/*! Just print `opts.text` and return an exit status code.
    \pi opts Pointer to ColrOpts to get text/options from.
    \return  `EXIT_SUCCESS` on success, otherwise `EXIT_FAILURE`.
*/
int print_plain(ColrOpts* opts) {
    if (!opts->text) {
        printferr("\nNo text to work with!\n");
        return EXIT_FAILURE;
    }
    fprintf(opts->out_stream, "%s\n", opts->text);
    return EXIT_SUCCESS;
}

/*! Print a single name/color from colr_name_data.

    \pi index  The index into colr_name_data.
    \pi do_rgb Whether to use RGB codes.
*/
void print_name(ColrOpts* opts, size_t index, bool do_rgb) {
    if (index >= colr_name_data_len) return;
    ColorNameData item = colr_name_data[index];
    char* name = item.name;
    char* numblock = NULL;
    if_not_asprintf(&numblock, "   %03d   ", item.ext) {
        return;
    }

    RGB foreval = RGB_inverted(RGB_monochrome(item.rgb));
    ExtendedValue forevalext = foreval.red > 128 ? XWHITE : XBLACK;
    // Use RGB if requested.
    char* block = colr_cat(
        Colr(
            numblock,
            do_rgb ?
                fore(foreval) :
                fore(forevalext),
            do_rgb ?
                back(item.rgb) :
                back(ext(item.ext)),
                style(BRIGHT)
            )
    );
    free(numblock);
    fprintf(opts->out_stream, "%21s: %s", name, block);
    free(block);
}

/*! Demo of the known-name database.
*/
int print_names(ColrOpts* opts, bool do_rgb) {
    ColrOpts_set_disabled(opts);
    size_t third_length = colr_name_data_len / 3;
    size_t printed = 0;
    for (size_t i = 0; i < third_length; i++) {
        print_name(opts, i, do_rgb);
        printed++;
        size_t second = third_length + i;
        if (second >= colr_name_data_len) continue;
        print_name(opts, second, do_rgb);
        printed++;
        size_t third = third_length + third_length + i;
        if (third >= colr_name_data_len) continue;
        print_name(opts, third, do_rgb);
        printed++;
        fprintf(opts->out_stream, "\n");
    }
    // Print remaining names, should be 3 or less.
    while (printed < colr_name_data_len) {
        print_name(opts, printed, do_rgb);
        printed++;
    }
    fprintf(opts->out_stream, "\n\n");
    return EXIT_SUCCESS;
}

/*! Demo the rainbow method.
*/
int print_rainbow(ColrOpts* opts, bool do_back) {
    ColrOpts_set_disabled(opts);
    char text[] = "This is a demo of the rainbow function.";
    char* rainbowtxt;
    if (do_back) {
        rainbowtxt = colr_supports_rgb() ?
            rainbow_bg(text, CT_DEFAULT_FREQ, CT_DEFAULT_OFFSET, CT_DEFAULT_SPREAD) :
            rainbow_bg_term(text, CT_DEFAULT_FREQ, CT_DEFAULT_OFFSET, CT_DEFAULT_SPREAD);
    } else {
        rainbowtxt = colr_supports_rgb() ?
            rainbow_fg(text, CT_DEFAULT_FREQ, CT_DEFAULT_OFFSET, CT_DEFAULT_SPREAD) :
            rainbow_fg_term(text, CT_DEFAULT_FREQ, CT_DEFAULT_OFFSET, CT_DEFAULT_SPREAD);
    }
    char* textfmt = colr_cat(
        do_back ? fore(BLACK) : back(RESET),
        do_back ? style(BRIGHT) : style(NORMAL),
        rainbowtxt
    );
    free(rainbowtxt);
    fprintf(opts->out_stream, "%s\n", textfmt);
    free(textfmt);
    return EXIT_SUCCESS;
}

/*! Print part of the RGB range using either colrfgrgb, or .
    The function choice is passed as an argument.
*/
int print_rgb(ColrOpts* opts, bool do_back, bool term_rgb) {
    ColrOpts_set_disabled(opts);
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
                        text = colr_cat(back(extval), fore(otherval), num);
                    } else {
                        text = colr_cat(fore(extval), back(otherval), num);
                    }
                } else {
                    // Make the rgb text.
                    num = RGB_to_str(vals);
                    // Colorize it.
                    RGB othervals = do_back ? (RGB){255, 255, 255} : (RGB){0, 0, 0};
                    if (do_back) {
                        text = colr_cat(back(vals), fore(othervals), num);
                    } else {
                        text = colr_cat(fore(vals), back(othervals), num);
                    }
                }
                free(num);
                count++;
                fprintf(opts->out_stream, "%s ", text);
                free(text);
                if (count > 3) {
                    fprintf(opts->out_stream, "\n");
                    count = 0;
                }
            }
        }
    }
    fprintf(opts->out_stream, "\n");
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
        colr --basic | --256 | --names | --rainbowize | --rgb | --rgbterm\n\
        colr (-t | -x | -z [-u]) [-a] [TEXT]\n\
        colr [TEXT] [-R] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
             [-a] [-c num | -l num | -r num] [-o num] [-q num]\n\
        colr [-F file] [-R] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
             [-a] [-c num | -l num | -r num] [-o num] [-q num]\n\
    ", NAME, VERSION);
    return EXIT_SUCCESS;
}


/*! Print the usage string.
*/
int print_usage_full() {
    print_usage(NULL);
    char* help_lines[] = {
"\n",
"    Commands:\n",
"        --basic[bg]          : Print all basic color names and colors.\n",
"        --256[bg]            : Print all extended color values and colors.\n",
"        --names[rgb]         : Print extra extended/rgb names and colors.\n",
"                               If 'rgb' is appended to the argument, RGB colors\n",
"                               will be used.\n",
"        --rainbowize[bg]     : Print a rainbow example.\n",
"        --rgb[bg]            : Print some rgb codes.\n",
"        --rgbterm[bg]        : Print some 256-compatible rgb codes.\n",
"\n",
"    If 'bg' is appended to a command argument, back colors will be used.\n",
"\n",
"    Options:\n",
"        TEXT                    : Text to colorize.\n",
"                                  Use \"-\" or \"-F -\" to force stdin.\n",
"                                  Default: stdin\n",
"        FORE                    : Fore color name/value for text.\n",
"                                  If set to 'rainbow', the text will be rainbowized.\n",
"                                  If set to 'rainbowterm', 256-color codes are used,\n",
"                                  instead of RGB.\n",
"        BACK                    : Back color name/value for text.\n",
"                                  If set to 'rainbow', the back colors will be rainbowized.\n",
"                                  If set to 'rainbowterm', 256-color codes are used,\n",
"                                  instead of RGB.\n",
"        STYLE                   : Style name for text.\n",
"        -a,--auto-disable       : Disable colored output when not outputting to a terminal.\n",
"        -b val,--back val       : Specify the back color explicitly, in any order.\n",
"        -c num,--center num     : Center-justify the resulting text using the specified width.\n",
"                                  If \"0\" is given, the terminal-width will be used.\n",
"        -F file,--file file     : Read text from a file.\n",
"                                  Use \"-F -\" to force stdin.\n",
"        -f val,--fore val       : Specify the fore color explicitly, in any order.\n",
"        -h, --help              : Print this message and exit.\n",
"        -l num,--ljust num      : Left-justify the resulting text using the specified width.\n",
"                                  If \"0\" is given, the terminal-width will be used.\n",
"        -o num,--offset num     : Starting offset into the rainbow if \"rainbow\"\n",
"                                  is used as a fore/back color.\n",
"                                  This will \"shift\" the starting color of the\n",
"                                  rainbow.\n",
"                                  Values must be 0 or positive.\n",
"                                  Default: " colr_macro_str(CT_DEFAULT_OFFSET) "\n",
"        -q num,--frequency num  : Frequency when \"rainbow\" is used as a fore/back color.\n",
"                                  Higher numbers cause more contrast.\n",
"                                  Lower numbers cause \"smoother\" gradients.\n",
"                                  Values can be: " colr_macro_str(CT_MIN_FREQ) "-" colr_macro_str(CT_MAX_FREQ) "\n",
"                                  Default: " colr_macro_str(CT_DEFAULT_FREQ) "\n",
"        -R,--rainbow            : Same as -f rainbow or -b rainbow, depending\n",
"                                  on whichever one hasn't been set.\n",
"                                  This is for Colr.py compatibility.\n",
"        -r num,--rjust num      : Right-justify the resulting text using the specified width.\n",
"                                  If \"0\" is given, the terminal-width will be used.\n",
"        -s val,--style val      : Specify the style explicitly, in any order.\n",
"        -t,--translate          : Translate a color into all three color types.\n",
"        -u,--unique             : Only list unique escape codes with -z.\n",
"        -v,--version            : Show version and exit.\n",
"        -x,--stripcodes         : Strip escape codes from the text.\n",
"        -z,--listcodes          : List escape codes found in the text.\n",
"\n",
"    When the flag arguments are used (-f, -b, -s), the order does not matter\n",
"    and any of them may be omitted. The remaining non-flag arguments are parsed\n",
"    in order (text, fore, back, style).\n",
"\n",
"    Color values can be one of:\n",
"        A known name.      Use --names to list all known color names.\n",
"                           \"none\", \"red\", \"blue\", \"lightblue\", \"black\", etc.\n",
"                           \"rainbow\" causes fore or back colors to be rainbowized.\n",
"        A 256-color value. 0-255\n",
"        An RGB string.     \"R;G;B\", \"R:G:B\", \"R,G,B\", or \"R G B\".\n",
"        A hex color.       \"#ffffff\", or the short-form \"#fff\"\n",
NULL
    };
    size_t i = 0;
    while (help_lines[i]) {
        printf("%s", help_lines[i++]);
    }

    puts("\n");
    return EXIT_SUCCESS;
}

/*! Print the ColrC version.
*/
int print_version(void) {
    printf(NAME " v. " VERSION "\n");
    return EXIT_SUCCESS;
}

/*! Rainbowize some text with options from ColrOpts.

    \pi opts An initialized ColrOpts.
    \return  An allocated ColorText, with a rainbowized `.text` member.
*/
ColorText* rainbowize(ColrOpts* opts) {
    bool do_term_rainbow = opts->rainbow_term || !colr_supports_rgb();
    rainbow_creator func = (
        do_term_rainbow ?
            (opts->rainbow_fore ? rainbow_fg_term : rainbow_bg_term) :
            (opts->rainbow_fore ? rainbow_fg : rainbow_bg)
    );
    char* rainbowized = func(
        opts->text,
        opts->rainbow_freq,
        opts->rainbow_offset,
        opts->rainbow_spread
    );
    if (!rainbowized) return NULL;
    // Text was allocated from stdin input, it's safe to free.
    if (opts->free_text) {
        free(opts->text);
        // Don't use or free the text again.
        opts->text = NULL;
        opts->free_text = false;
    }

    opts->free_colr_text = true;
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
    size_t buffer_length = line_length;
    char* buffer = NULL;
    size_t total = 0;
    while ((fgets(line, line_length, fp))) {
        total += strlen(line);
        if (!buffer) {
            // First line.
            buffer = calloc(buffer_length, sizeof(char));
        } else if (total >= buffer_length) {
            buffer_length *= 3;
            buffer = realloc(buffer, buffer_length);
            if (!buffer) {
                printferr("Failed to reallocate for file data!\n");
                return NULL;
            }
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
        printferr("\nReading from stdin until EOF (Ctrl + D)...\n");
    }
    return read_file(stdin);
}

/*! Run a simple colr-tool command, that doesn't use the ColorArgs in
    ColrOpts.

    \pi func The command function to run.
    \pi opts Pointer to ColrOpts for text/options.
    \return  Same as `func()`, usually `EXIT_SUCCESS` on success, otherwise `EXIT_FAILURE`.
*/
int run_colr_cmd(colr_tool_cmd func, ColrOpts* opts) {
    int ret = func(opts);
    ColrOpts_free_text(opts);
    ColrOpts_free_args(opts);
    return ret;
}
/*! Strip escape codes from `opts->text` and return an exit status code.

    \pi opts Pointer to ColrOpts to get the text from.
    \return  `EXIT_SUCCESS` on success, otherwise `EXIT_FAILURE`
*/
int strip_codes(ColrOpts* opts) {
    if (!opts->text) {
        printferr("\nNo text to strip!\n");
        return EXIT_FAILURE;
    } else if (opts->text[0] == '\0') {
        printferr("\nText was empty!\n");
        return EXIT_FAILURE;
    }
    char* stripped = colr_str_strip_codes(opts->text);
    if (!stripped) {
        printferr("\nFailed to create stripped text!\n");
        return EXIT_FAILURE;
    } else if (stripped[0] == '\0') {
        // Empty string was given.
        free(stripped);
        fprintf(opts->out_stream, "\n");
        return EXIT_SUCCESS;
    }
    size_t length = strlen(stripped);
    if (stripped[length - 1] == '\n') {
        fprintf(opts->out_stream, "%s", stripped);
    } else {
        // Add a newline, for prettier output.
        fprintf(opts->out_stream, "%s\n", stripped);
    }
    free(stripped);
    return EXIT_SUCCESS;
}

/*! Translate a user's color argument into all color types.

    \pi opts ColrOpts to get the text/options from.
    \return  `EXIT_SUCCESS` on success, otherwise `EXIT_FAILURE`.
*/
int translate_code(ColrOpts* opts) {
    ColorArg* carg = fore(opts->text);
    if (!validate_color_arg(*carg, opts->text)) {
        free(carg);
        return EXIT_FAILURE;
    }
    ColorValue cval = carg->value;
    free(carg);
    BasicValue bval = cval.basic;
    ExtendedValue eval = cval.ext;
    RGB rgbval = cval.rgb;

    if (cval.type == TYPE_BASIC) {
        dbug("Using BasicValue for: %s\n", opts->text);
        rgbval = RGB_from_BasicValue(bval);
        eval = ExtendedValue_from_BasicValue(bval);
    } else if (cval.type == TYPE_EXTENDED) {
        rgbval = RGB_from_ExtendedValue(eval);
        bval = BASIC_NONE;
    } else if (cval.type == TYPE_RGB) {
        eval = ExtendedValue_from_RGB(rgbval);
        bval = BASIC_NONE;
    } else {
        printferr("Invalid color: %s\n", opts->text);
        return EXIT_FAILURE;
    }

    if (bval == BASIC_NONE) {
        dbug("Converting to BasicValue from ExtendedValue: %d\n", eval);
        // Only convert the first 16 ext values to basic.
        BasicValue trybval = BasicValue_from_str(opts->text);
        if (BasicValue_is_invalid(trybval)){
            trybval = BASIC_NONE;
            if (eval < 8) trybval = basic(eval);
            else if (eval < 16) trybval = basic(eval + 2);
            else {
                for (size_t i = 0; i < colr_name_data_len; i++) {
                    ColorNameData item = colr_name_data[i];
                    if (item.ext == eval) {
                        // Try the normal color names and light color names.
                        trybval = BasicValue_from_str(item.name);
                        dbug("FOUND IT: %d\n", trybval);
                        break;
                    }
                }
                if (BasicValue_is_invalid(trybval)) trybval = BASIC_NONE;
            }
        }
        bval = trybval;
    }

    char* bstr = NULL;
    if (bval != BASIC_NONE) bstr = BasicValue_to_str(bval);
    if (!bstr) asprintf_or_return(EXIT_FAILURE, &bstr, "?");
    char* estr = ExtendedValue_to_str(eval);
    if (!estr) asprintf_or_return(EXIT_FAILURE, &estr, "?");
    char* rgbstr = RGB_to_str(rgbval);
    if (!rgbstr) asprintf_or_return(EXIT_FAILURE, &rgbstr, "?");
    char* hexstr = RGB_to_hex(rgbval);
    if (!hexstr) asprintf_or_return(EXIT_FAILURE, &hexstr, "?");
    char* blbl = Colr_str("basic:", style(UNDERLINE));
    if (!blbl) asprintf_or_return(EXIT_FAILURE, &blbl, "basic:");
    char* elbl = Colr_str("ext:", style(UNDERLINE));
    if (!elbl) asprintf_or_return(EXIT_FAILURE, &elbl, "ext:");
    char* rgblbl = Colr_str("rgb:", style(UNDERLINE));
    if (!rgblbl) asprintf_or_return(EXIT_FAILURE, &rgblbl, "rgb:");
    char* hexlbl = Colr_str("hex:", style(UNDERLINE));
    if (!hexlbl) asprintf_or_return(EXIT_FAILURE, &hexlbl, "hex:");

    char* transtr = NULL;
    asprintf_or_return(
        EXIT_FAILURE,
        &transtr,
        "%s %-12s %s %-3s %s %-11s %s %-7s",
        blbl,
        bstr,
        elbl,
        estr,
        rgblbl,
        rgbstr,
        hexlbl,
        hexstr
    );
    free(bstr);
    free(estr);
    free(rgbstr);
    free(hexstr);
    free(blbl);
    free(elbl);
    free(rgblbl);
    free(hexlbl);
    // Always use the ExtendedValue.
    colr_print(Colr("●", fore(ext(eval))));
    printf(" - %s\n", transtr);
    free(transtr);
    return EXIT_SUCCESS;
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
        return false;
    }
    char* errmsg;
    char* argtype = ArgType_to_str(carg.type);
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
        case TYPE_INVALID_EXT_RANGE:
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
                "Invalid %s %sname: %s",
                argtype,
                (carg.type == BACK) || (carg.type == FORE) ? "color " : "",
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
            free(argtype);
            return true;
    }

    // Print the error message that was built.
    free(argtype);
    print_usage(errmsg);
    free(errmsg);
    return false;
}

#endif // DOXYGEN_SKIP
