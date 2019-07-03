#include "colr.h"

int main(void) {
    /*
        You can join things by a plain string or a colorized string.

        For the pieces, the order/number of arguments does not matter.
        colr_join() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char* colorized = colr_join(
        "\n",
        "This is a plain line.",
        Colr("This one is some kind of purple.", fore(rgb(125, 0, 155))),
        Colr("This one is bright.", style(BRIGHT)),
        "Another plain one, why not?"
    );

    // Prints each colorized piece of text on it's own line:
    printf("%s\n", colorized);
    free(colorized);
}