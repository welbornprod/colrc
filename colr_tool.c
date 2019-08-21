#ifndef DOXYGEN_SKIP
//! \file

/*! Example implementation using colr.h, and a command-line tool
    to colorize text.

    \author Christopher Welborn
    \date 02-05-2017
*/
#include "colr_tool.h"


int main(int argc, char* argv[]) {
    // Needed for str_to_wide(), and wide_to_str(), and the rainbow() funcs.
    setlocale(LC_ALL, "");
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
    } else if (opts.is_disabled) {
        // No need to colorize anything, colors were disabled. We are `cat` now.
        return run_colr_cmd(print_plain, &opts);
    }

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
    if (!colr_str_ends_with(text, "\n" CODE_RESET_ALL)) {
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
        .rainbow_freq=0.1,
        .rainbow_offset=3,
        .auto_disable=false,
        .is_disabled=false,
        .list_codes=false,
        .list_unique_codes=false,
        .strip_codes=false,
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
    .auto_disable=%s,\n\
    .is_disabled=%s,\n\
    .list_codes=%s,\n\
    .list_unique_codes=%s,\n\
    .strip_codes=%s,\n\
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
        bool_str(opts.auto_disable),
        bool_str(opts.is_disabled),
        bool_str(opts.list_codes),
        bool_str(opts.list_unique_codes),
        bool_str(opts.strip_codes),
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
    If processing-options are set (`.list_codes`, `.strip_codes`), the default
    is to read from stdin.

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
    bool do_stdin = colr_str_eq(opts->text, "-");
    if (!opts->text && (opts->list_codes || opts->strip_codes)) do_stdin = true;
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
    \return      `true` if the filepath is not `NULL`/empty and exists, otherwise `false`.
*/
bool file_exists(const char* filepath) {
    if (!filepath || filepath[0] == '\0') return false;
    struct stat st;
    if (stat(filepath, &st) < 0) return false;

    return ((st.st_mode & S_IFMT) == S_IFREG);
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

/*! Parse user arguments into a ColrOpts struct.

    \details
    This handles long-only options like --basic, --rainbow, and intializes
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
        // Commands
        {"listcodes", no_argument, 0, 'z'},
        {"stripcodes", no_argument, 0, 'x'},
        // Command options.
        {"auto-disable", no_argument, 0, 'a'},
        {"err", no_argument, 0, 'e'},
        {"unique", no_argument, 0, 'u'},
        // Rainbow options.
        {"frequency", required_argument, 0, 'q'},
        {"offset", required_argument, 0, 'o'},
        // Commands.
        {"basic", no_argument, 0, 0 },
        {"basicbg", no_argument, 0, 0 },
        {"256", no_argument, 0, 0},
        {"256bg", no_argument, 0, 0},
        {"names", no_argument, 0, 0},
        {"namesrgb", no_argument, 0, 0},
        {"rainbow", no_argument, 0, 0},
        {"rainbowbg", no_argument, 0, 0},
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
            ":aehuvxzb:c:F:f:l:o:q:r:s:",
            long_options,
            &option_index
        );
        if (c == -1) break;

        int argval_just;
        double argval_freq;
        size_t argval_offset;

        switch (c) {
            case 0:
                if (colr_str_eq(long_options[option_index].name, "basic")) {
                    return print_basic(opts, false);
                } else if (colr_str_eq(long_options[option_index].name, "256")) {
                    return print_256(opts, false);
                } else if (colr_str_eq(long_options[option_index].name, "names")) {
                    return print_names(opts, false);
                } else if (colr_str_eq(long_options[option_index].name, "rainbow")) {
                    return print_rainbow(opts, false);
                } else if (colr_str_eq(long_options[option_index].name, "rgb")) {
                    return print_rgb(opts, false, false);
                } else if (colr_str_eq(long_options[option_index].name, "rgbterm")) {
                    return print_rgb(opts, false, true);
                } else if (colr_str_eq(long_options[option_index].name, "basicbg")) {
                    return print_basic(opts, true);
                } else if (colr_str_eq(long_options[option_index].name, "256bg")) {
                    return print_256(opts, true);
                } else if (colr_str_eq(long_options[option_index].name, "namesrgb")) {
                    return print_names(opts, true);
                } else if (colr_str_eq(long_options[option_index].name, "rainbowbg")) {
                    return print_rainbow(opts, true);
                } else if (colr_str_eq(long_options[option_index].name, "rgbbg")) {
                    return print_rgb(opts, true, false);
                } else if (colr_str_eq(long_options[option_index].name, "rgbtermbg")) {
                    return print_rgb(opts, true, true);
                } else {
                    printferr(
                        "Developer Error: Unhandled long-only option!: %s\n",
                        long_options[option_index].name
                    );
                    return EXIT_FAILURE;
                }
                break;
            case 'a':
                opts->auto_disable = true;
                break;
            case 'b':
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
                if (argval_freq < 0.1) {
                    argval_freq = 0.1;
                } else if (argval_freq > 1.0) {
                    argval_freq = 1.0;
                }
                opts->rainbow_freq = argval_freq;
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
            case 'u':
                opts->list_codes = true;
                opts->list_unique_codes = true;
                break;
            case 'v':
                print_version();
                return EXIT_SUCCESS;
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
    }
    // Remaining non-option arguments.
    while (optind < argc) {
        if (!opts->text && !opts->filepath) {
            // If a file path is set, the text will come later.
            opts->text = argv[optind];
            optind++;
            if (opts->list_codes || opts->strip_codes) {
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
            text = colr(carg, num);
            fprintf(opts->out_stream, "%s ", text);
            if ((i == 7) || (i == 15)) fprintf(opts->out_stream, "\n");
            free(text);
        } else {
            // Print the number as is.
            text = colr(carg, num);
            fprintf(opts->out_stream, "%s ", text);
            free(text);
            // Print the other 5 in the group.
            int j = i;
            for (int k=0; k < 5; k++) {
                j = j + 36;
                snprintf(num, 4, "%03d", j);
                carg = do_back ? back(ext(j)) : fore(ext(j));
                text = colr(carg, num);
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
        text = colr(carg, num);
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
            text = colr(back(val), fore(otherval), namefmt);
        } else {
            text = colr(fore(val), back(otherval), namefmt);
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
    char* block = colr(
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
    fprintf(opts->out_stream, "\n");
    if (printed != colr_name_data_len) {
        // Should never happen unless colr_name_data is updated.
        printferr("\nSome names are missing from this print-out.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*! Demo the rainbow method.
*/
int print_rainbow(ColrOpts* opts, bool do_back) {
    ColrOpts_set_disabled(opts);
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
        colr --basic | --256 | --names | --rainbow | --rgb | --rgbterm\n\
        colr (-x | -z [-u]) [-a] [TEXT]\n\
        colr [TEXT] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
             [-a] [-c num | -l num | -r num] [-o num] [-q num]\n\
        colr [-F file] [FORE | -f color] [BACK | -b color] [STYLE | -s style]\n\
             [-a] [-c num | -l num | -r num] [-o num] [-q num]\n\
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
        --256[bg]            : Print all extended color values and colors.\n\
        --names[rgb]         : Print extra extended/rgb names and colors.\n\
                               If 'rgb' is appended to the argument, RGB colors\n\
                               will be used.\n\
        --rainbow[bg]        : Print a rainbow example.\n\
        --rgb[bg]            : Print some rgb codes.\n\
        --rgbterm[bg]        : Print some 256-compatible rgb codes.\n\
\n\
    If 'bg' is appended to a command argument, back colors will be used.\n\
\n\
    Options:\n\
        TEXT                    : Text to colorize.\n\
                                  Use \"-\" or \"-F -\" to force stdin.\n\
                                  Default: stdin\n\
        FORE                    : Fore color name/value for text.\n\
                                  If set to 'rainbow', the text will be rainbowized.\n\
                                  If set to 'rainbowterm', 256-color codes are used,\n\
                                  instead of RGB.\n\
        BACK                    : Back color name/value for text.\n\
                                  If set to 'rainbow', the back colors will be rainbowized.\n\
                                  If set to 'rainbowterm', 256-color codes are used,\n\
                                  instead of RGB.\n\
        STYLE                   : Style name for text.\n\
        -a,--auto-disable       : Disable colored output when not outputting to a terminal.\n\
        -b val,--back val       : Specify the back color explicitly, in any order.\n\
        -c num,--center num     : Center-justify the resulting text using the specified width.\n\
                                  If \"0\" is given, the terminal-width will be used.\n\
        -F file,--file file     : Read text from a file.\n\
                                  Use \"-F -\" to force stdin.\n\
        -f val,--fore val       : Specify the fore color explicitly, in any order.\n\
        -h, --help              : Print this message and exit.\n\
        -l num,--ljust num      : Left-justify the resulting text using the specified width.\n\
                                  If \"0\" is given, the terminal-width will be used.\n\
        -o num,--offset num     : Starting offset into the rainbow if \"rainbow\"\n\
                                  is used as a fore/back color.\n\
                                  This will \"shift\" the starting color of the\n\
                                  rainbow.\n\
                                  Values must be 0 or positive.\n\
                                  Default: 3\n\
        -q num,--frequency num  : Frequency when \"rainbow\" is used as a fore/back color.\n\
                                  Higher numbers cause more contrast.\n\
                                  Lower numbers cause \"smoother\" gradients.\n\
                                  Values can be: 0.1-1.0\n\
                                  Default: 0.1\n\
        -r num,--rjust num      : Right-justify the resulting text using the specified width.\n\
                                  If \"0\" is given, the terminal-width will be used.\n\
        -s val,--style val      : Specify the style explicitly, in any order.\n\
        -u,--unique             : Only list unique escape codes with -z.\n\
        -v,--version            : Show version and exit.\n\
        -x,--stripcodes         : Strip escape codes from the text.\n\
        -z,--listcodes          : List escape codes found in the text.\n\
\n\
    When the flag arguments are used (-f, -b, -s), the order does not matter\n\
    and any of them may be omitted. The remaining non-flag arguments are parsed\n\
    in order (text, fore, back, style).\n\
\n\
    Color values can be one of:\n\
        A known name.      Use --names to list all known color names.\n\
                           \"none\", \"red\", \"blue\", \"lightblue\", \"black\", etc.\n\
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
    char* rainbowized = func(opts->text, opts->rainbow_freq, opts->rainbow_offset);
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
            free(argtype);
            break;
        case TYPE_INVALID_EXT_RANGE:
            asprintf_or_return(
                false,
                &errmsg,
                "Invalid range (0-255) for extended %s color: %s",
                argtype,
                name
            );
            free(argtype);
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
            free(argtype);
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
