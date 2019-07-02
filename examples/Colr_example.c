#include "colr.h"

int main(void) {
    /*
        Colr is for styling one piece of text.
        When combined with the colr() macro it allows you to seperate colors/styles.
    */
    char* colorized = colr(
        Colr("America ", fore(RED)),
        Colr("the ", fore(WHITE)),
        Colr("beautiful", fore(BLUE)),
        ".\n"
    );

    /*
        All of the Colr, fore, back, and style resources were free'd by `colr`.
        You are responsible for the text and the resulting colorized string.
    */
    printf("%s", colorized);
    free(colorized);

    /*
        Colr is shorthand for building a ColorText struct.
        If you use them outside of the colr macros, you must free them.
        Again, you must always free the plain text if you allocated for it.
    */
    struct ColorText* ctext = Colr("I didn't allocate this.", fore(BLUE));
    /*
        If you use the *to_str functions directly,
        you must free the resulting string.
    */
    char* ctext_str = ColorText_to_str(*ctext);
    printf("But I allocated the resulting string: %s\n", ctext_str);
    // No leaks, free the ColorText's resources.
    ColorText_free(ctext);
    // And free the string you created from it.
    free(ctext_str);
}
