#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr(
        fore(BLACK),
        back(RED), "This is a test",
        back(BLUE), " and only a test."
    );
    if (!s) return 1;
    printf("%s\n", s);
    free(s);

    // Color names:
    char* n = colr(
        back("blue"),
        fore("white"),
        "This is blue."
    );
    if (!n) return 1;
    printf("%s\nThis is not.\n", n);
    free(n);

    // Extended (256) colors:
    char* e = colr(fore(ext(0)), back(ext(35)), "Extended colors.\n");
    if (!e) return 1;
    printf("%s", e);
    free(e);

    // RGB (True Color) colors:
    char* r = colr(back(rgb(35, 0, 155)), "RGB");
    if (!r) return 1;
    printf("%s\n", r);
    free(r);

    // Hex (RGB style) colors:
    char* h = colr(
        back("#ff0000"), "Hex RGB\n",
        back(hex("fff")), fore(hex("000000")), "Hex macro RGB\n",
        back(hex_or("NOTHEX", rgb(255, 255, 255))), "Using default for bad hex str"
    );
    if (!h) return 1;
    printf("%s\n", h);
    free(h);

    // Hex (Closest ExtendedValue) colors:
    char* he = colr(
        back(ext_hex("ff0000")), "Closest ExtendedValue Hex\n",
        back(ext_hex_or("NOTAHEX", ext(255))), "Using default for bad hex str"
    );
    if (!he) return 1;
    printf("%s\n", he);
    free(he);

    /*
        Colr() accepts a back() as one of it's arguments.
        The order does not matter.
    */
    char* colorized = colr(
        Colr("This is red.\n", back(RED)),
        Colr("This is also red.\n", fore("white"), back("red")),
        "This is not."
    );
    if (!colorized) return 1;
    printf("%s\n", colorized);
    free(colorized);
}
