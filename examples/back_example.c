#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr(
        fore(BLACK),
        back(RED),
        "This is a test",
        back(BLUE),
        " and only a test."
    );
    printf("%s\n", s);
    free(s);

    // Color names:
    char* n = colr(
        back("blue"),
        fore("white"),
        "This is blue."
    );
    printf("%s\nThis is not.\n", n);
    free(n);

    // Extended (256) colors:
    char* e = colr(fore(ext(0)), back(ext(35)), "Extended colors.\n");
    printf("%s", e);
    free(e);

    // RGB (True Color) colors:
    char* r = colr(back(rgb(35, 0, 155)), "RGB\n");
    printf("%s", r);
    free(r);

    /*
        Colr() accepts a back() as one of it's arguments.
        The order does not matter.
    */
    char* colorized = colr(
        Colr("This is red.\n", back(RED)),
        Colr("This is also red.\n", back("red"), fore("white")),
        "This is not."
    );
    printf("%s\n", colorized);
    free(colorized);
}
