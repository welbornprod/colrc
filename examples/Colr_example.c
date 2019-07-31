#include "colr.h"

int main(void) {
    /*
        Colr() is for styling one piece of text.
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
        There are three justification macros that make it easy to create
        ColorText's with center, left, or right-justified text.
    */
    char* justified = colr(
        Colr_center("This is centered.", 80, fore("lightblue")),
        "\n",
        Colr_ljust("This is on the left.", 38, fore(ext_hex("ff2525"))),
        "----",
        Colr_rjust("This is on the right.", 38, fore(ext_rgb(255, 53, 125)))
    );
    printf("%s\n", justified);
    free(justified);
}
