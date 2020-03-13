#include "colr.h"

int main(int argc, char** argv) {
    // Print-related macros, using Colr() to build colorized text:
    puts("\nColrC supports ");
    colr_puts(Colr_join(
        ", ",
        Colr("basic", fore(WHITE)),
        Colr("extended (256)", fore(ext(155))),
        Colr("rgb", fore(rgb(155, 25, 195))),
        Colr("hex", fore(hex("#ff00bb"))),
        Colr("extended hex", fore(ext_hex("#ff00bb"))),
        Colr("color names", fore("dodgerblue"), back("aliceblue")),
        Colr("and styles.", style(BRIGHT))
    ));

    colr_puts(
        "Strings and ",
        Colr("colors", fore(LIGHTBLUE)),
        " can be mixed in any order."
    );

    // Create a string, using colr(), instead of colr_puts() or colr_print().
    char* mystr = colr("Don't want to print this.", style(UNDERLINE));
    printf("\nNow I do: %s\n", mystr);
    free(mystr);

    // Concatenate existing strings with ColrC objects.
    // Remember that the colr macros will free ColrC objects, not strings.
    // So I'm going to use the Colr* macros inside of this call (not colr*).
    char* catted = colr_cat(
        "Exhibit: ",
        Colr("b", fore(BLUE)),
        "\nThe ColorText/Colr was released."
    );
    puts(catted);
    free(catted);

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

    // Create a joined string (a "[warning]" label).
    char* warning_label = colr_join(Colr("warning", fore(YELLOW)), "[", "]");
    // Simulate multiple uses of the string.
    for (int i = 1; i < 4; i++) printf("%s This is #%d\n", warning_label, i);
    // Okay, now we're done with the colorized string.
    free(warning_label);

    // Colorize an existing string by replacing a word.
    char* logtext = "[warning] This is an awesome warning.";
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

    // Or maybe you want to replace ALL of the occurrences?
    char* logtext2 = "[warning] This is an awesome warning.";
    // There is also a colr_replace_re_all() if you'd rather use a regex pattern.
    char* colorizedall = colr_replace_all(
        logtext2,
        "warning",
        Colr("WARNING", fore(YELLOW))
    );
    // Failed to allocate for new string?
    if (!colorizedall) return EXIT_FAILURE;
    puts(colorizedall);
    // You have to free the resulting string.
    free(colorizedall);

}
