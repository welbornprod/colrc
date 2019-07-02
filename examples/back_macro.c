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

    // Extended (256) colors:
    char* e = colr(fore(ext(0)), back(ext(35)), "Extended colors.\n");
    printf("%s", e);
    free(e);

    // RGB (True Color) colors:
    char* r = colr(back(rgb(35, 0, 155)), "RGB\n");
    printf("%s", r);
    free(r);

    // The order and color type does not matter.
    char* all = colr(
        fore(WHITE),
        back(YELLOW),
        "Basic, ",
        back(ext(234)),
        "Extended, ",
        back(rgb(23, 0, 155)),
        "RGB, ",
        style(UNDERLINE),
        "Styles, ",
        back("lightblue"),
        "color names, ",
        style("bright"),
        "and style names."
    );
    printf("%s\n", all);
    free(all);
}
