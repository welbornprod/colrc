#include "colr.h"

int main(void) {
    /* You can join things by a plain string or a colorized string.

       For the pieces, the order/number of arguments does not matter.
       colr() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char* colorized = colr_join(
        Colr(" and ", fore(RED)),
        "This",
        Colr("that", fore(rgb(125, 0, 155))),
        Colr("those", style(BRIGHT)),
        "the last one."
    );

    // Prints a colorized: "This and that and those and the last one."
    printf("%s\n", colorized);
    free(colorized);
}
