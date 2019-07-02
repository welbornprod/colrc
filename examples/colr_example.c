#include "colr.h"

int main(void) {
    /* You can build your strings with colr().
       Using a Colr (ColorText), or sprinkling fore(), back(), and style() calls,
       you can build multi-color strings and only worry about allocating/freeing
       the text.

       The order/number of arguments does not matter.
       colr() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char* colorized = colr(
        "This will be a plain string.\n",
        Colr("This will be red/white.\n", fore(RED), back(WHITE)),
        "Just another string",
        " and another that will be joined to it.\n",
        Colr("The Colr ", fore(rgb(255, 0, 55))),
        Colr("strings", fore("yellow"), back("black")),
        Colr(" are also joined", style("bold")),
        ".\n",
        fore(BLUE),
        "The fore, back, and styles can be thrown in anywhere ",
        fore(MAGENTA), style(BRIGHT),
        "if that's your style.\n",
        NC, "You can also reset the style by throwing a `NC` in there."
    );

    // All of the Colr, fore, back, and style resources were free'd by `colr`.
    // You are responsible for the text and the resulting colorized string.
    printf("%s", colorized);
    free(colorized);

    // Like I said before, if your text was allocated, you must free it.
    char *allocated;
    asprintf(&allocated, "\nThis is my string #%d\n", 1);

    char *colored = colr(
        Colr(allocated, fore(ext(255)), style(UNDERLINE)),
        "This one should not be free'd though.\n"
    );
    printf("%s", colored);
    free(colored);
    free(allocated);

    // Colr is shorthand for building a ColorText struct.
    // If you use them outside of the colr macros, you must free them.
    // Again, you must always free the plain text if you allocated for it.
    struct ColorText* ctext = Colr("I didn't allocate this.", fore(BLUE));
    // If you use the *to_str functions outside of the colr macros, you
    // must free the resulting string.
    char* ctext_str = ColorText_to_str(*ctext);
    printf("But I allocated the resulting string: %s\n\n", ctext_str);
    // No leaks, free the ColorText's resources.
    ColorText_free(ctext);
    // And free the string you created from it.
    free(ctext_str);
}
