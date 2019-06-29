#include "colr.h"

int main(void) {
    //! Basic colors:
    char *s = colr(style(BRIGHT), "This is a test", style(UNDERLINE), " and only a test.");
    printf("%s\n", s);
    free(s);

    //! The order and color type does not matter.
    char *all = colr(
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
