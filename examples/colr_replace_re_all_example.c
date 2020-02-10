#include "colr.h"

int main(void) {

    // The string we are modifying.
    char* mystring = "This was foo, and I mean foo.";
    char* pattern = "fo{2}";

    /*
        Replace a regex match with a string.
    */
    char* replaced = colr_replace_re_all(
        mystring,
        pattern,
        "replaced",
        0
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a regex match with a ColorText.
    */
    replaced = colr_replace_re_all(
        mystring,
        pattern,
        Colr("replaced", fore(RED)),
        REG_ICASE
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a regex match with a ColorResult.
    */
    replaced = colr_replace_re_all(
        mystring,
        pattern,
        Colr_join(
            " ",
            Colr("really", style(BRIGHT)),
            Colr("replaced", fore(BLUE))
        ),
        0
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a regex match with a ColorResult.
    */
    char* mytemplate = "This REDis " NC "kinda REDuseful?" NC;
    replaced = colr_replace_re_all(
        mytemplate,
        "RED",
        fore(RED),
        0
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    return EXIT_SUCCESS;
}
