#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr_cat(
        fore(RED),
        "This is a test ",
        back(BLUE),
        style("bright"),
        "and only a test.",
        NC,
        "\nHad to reset the colors to get plain again.\n",
        fore(rgb(255, 0, 155)),
        back(ext(255)),
        style("underline"),
        // Careful placement of the NC (CODE_RESET_ALL) before the newline:
        // You said you wanted to do this manually...
        // Colr() takes care of stuff like this fore you.
        "This is the last line.", NC, "\n"
    );
    // CODE_RESET_ALL was appended to the result (even without the NC),
    // so later calls won't be affected by previous styles.
    printf("%s\n", s);

    free(s);
}
