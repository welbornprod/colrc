#include "colr.h"

int main(void) {
    /*
        Styles can be given as a StyleValue, or a style name (see style_names).
    */
    char* s = colr(
        style("bright"), "This is a test ",
        style(UNDERLINE), "and only a test."
    );
    printf("%s\n", s);
    free(s);

    /*
        Colr accepts a style() as one of it's arguments.
        The order does not matter.
    */
    char* mystr = colr(
        Colr("THIS IS BOLD.\n", style(BRIGHT)),
        "This is not."
    );
    printf("%s\n", mystr);
    free(mystr);

}
