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
        fore("blue"),
        "This is blue."
    );
    printf("%s\n", n);
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
        Colr() accepts a fore() as one of it's arguments.
        The order does not matter.
    */
    char* mystr = colr(
        Colr("This is red.\n", fore(RED)),
        Colr("This is also red.\n", back("white"), fore("red")),
        "This is not."
    );
    printf("%s\n", mystr);
    free(mystr);
}
