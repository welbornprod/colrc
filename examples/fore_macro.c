#include "colr.h"

int main(void) {
    //! Basic colors:
    char *s = colr(fore(RED), "This is a test", fore(BLUE), " and only a test.");
    printf("%s\n", s);
    free(s);

    //! Extended (256) colors:
    char *e = colr(fore(ext(35)), "Extended colors.");
    printf("%s", e);
    free(e);

    //! RGB (True Color) colors:
    char *r = colr(fore(rgb(35, 0, 155)), "RGB");
    printf("%s", r);
    free(r);

    //! The order and color type does not matter.
    char *all = colr(
        fore(YELLOW),
        "Basic, ",
        fore(ext(234)),
        "Extended, ",
        fore(rgb(23, 0, 155)),
        "and RGB."
    );
    printf("%s\n", all);
    free(all);
}
