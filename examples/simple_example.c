#include "colr.h"

int main(int argc, char** argv) {
    // Print-related macros, using Colr() to build colorized text:
    puts("\nColrC supports ");
    char* joined = colr_join(
        ", ",
        Colr("basic", fore(WHITE)),
        Colr("extended (256)", fore(ext(155))),
        Colr("rgb", fore(rgb(155, 25, 195))),
        Colr("hex", fore(hex("#ff00bb"))),
        Colr("extended hex", fore(ext_hex("#ff00bb"))),
        Colr("color names", fore("dodgerblue"), back("aliceblue")),
        Colr("and styles.", style(BRIGHT))
    );
    printf("%s\n", joined);
    free(joined);

    colr_puts(
        "Strings and ",
        Colr("colors", fore(LIGHTBLUE)),
        " can be mixed in any order."
    );

    // Create a string, using colr_cat(), instead of colr_puts() or colr_print().
    char* mystr = colr_cat(Colr("Don't want to print this.", style(UNDERLINE)));
    printf("\nNow I do: %s\n", mystr);
    free(mystr);

    // Create a ColorText, on the heap, for use with colr_cat(), colr_print(),
    // or colr_puts().
    ColorText* ctext = NULL;
    if (argc == 1) {
        ctext = Colr("<nothing>", fore(RED));
    } else {
        ctext = Colr(argv[1], fore(GREEN));
    }
    char* userstr = colr_cat("Argument: ", ctext);
    puts(userstr);
    // colr_cat() already called ColorText_free(ctext).
    free(userstr);

    // Colorize an existing string by replacing a word.
    char* logtext = "[warning] This is awesome.";
    char* colorized = colr_replace(
        logtext,
        "warning",
        Colr("warning", fore(YELLOW))
    );
    // Failed to allocate for new string?
    if (!colorized) return EXIT_FAILURE;
    puts(colorized);
    // You have to free the resulting string.
    free(colorized);

    // Or colorize an existing string by replacing a regex pattern.
    colorized = colr_replace_re(
        logtext,
        "\\[\\w+\\]",
        Colr_join(
            Colr("ok", style(BRIGHT)),
            "(",
            ")"
        ),
        REG_EXTENDED
    );
    if (!colorized) return EXIT_FAILURE;
    puts(colorized);
    free(colorized);
}
