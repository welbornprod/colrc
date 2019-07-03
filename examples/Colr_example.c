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
}
