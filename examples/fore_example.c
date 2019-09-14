#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr_cat(
        fore(RED),
        "This is a test",
        fore(BLUE),
        " and only a test."
    );
    printf("%s\n", s);
    free(s);

    // Color names:
    char* n = colr_cat(
        fore("red"),
        "This is red."
    );
    printf("%s\n", n);
    free(n);

    // Extended (256) colors:
    char* e = colr_cat(fore(ext(35)), "Extended colors.");
    printf("%s\n", e);
    free(e);

    // RGB (True Color) colors:
    char* r = colr_cat(fore(rgb(35, 0, 155)), "RGB");
    printf("%s\n", r);
    free(r);

    /*
        Colr() accepts a fore() as one of it's arguments.
        The order does not matter.
    */
    char* mystr = colr_cat(
        Colr("This is red.", fore(RED)),
        Colr("This is also red.", back("white"), fore("red")),
        "This is not.\n"
    );
    printf("%s\n", mystr);
    free(mystr);
}
