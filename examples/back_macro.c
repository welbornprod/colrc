#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr(
        back(RED),
        "This is a test",
        back(BLUE),
        " and only a test."
    );
    printf("%s\n", s);
    free(s);

    // Extended (256) colors:
    char* e = colr(back(ext(35)), "Extended colors.");
    printf("%s", e);
    free(e);

    // RGB (True Color) colors:
    char* r = colr(back(rgb(35, 0, 155)), "RGB");
    printf("%s", r);
    free(r);

    // The order and color type does not matter.
    char* all = colr(
        fore(YELLOW),
        "Basic, ",
        back(ext(234)),
        "Extended, ",
        fore(rgb(23, 0, 155)),
        "RGB, ",
        style(HIGHLIGHT),
        "Styles, ",
        fore("lightblue"),
        "color names, ",
        style("bright"),
        "and style names."
    );
    printf("%s\n", all);
    free(all);
}
