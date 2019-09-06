#include "colr.h"

int main(void) {
    /*
        You can build your strings with colr().
        Using a Colr (ColorText), or sprinkling fore(), back(), and style() calls,
        you can build multi-color strings and only worry about allocating/freeing
        the text.

        The order/number of arguments does not matter.
        colr() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char *colorized = colr(
        "This is plain.\n",
        Colr("This is styled.\n", fore(rgb(255, 0, 155))),
        fore(RED),
        "This was styled by the previous ColorArg.\n",
        NC,
        "This is normal because of the 'reset code' that came before it.\n",
        // See the colr_join example for more about this:
        Colr_join(Colr("This was joined", fore(RED)), "[", "]")
    );

    // Prints a colorized, joined, version of all the strings above.
    printf("%s\n", colorized);

    // Free the allocated result, no leaks.
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
}
