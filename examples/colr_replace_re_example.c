#include "colr.h"

int main(void) {
    /*
        If you already have a `NULL`-terminated array of `regmatch_t` (`regmatch_t**`),
        a single `regex_t`, or a compiled regex pattern (`regex_t`),
        you can use colr_replace() or colr_replace_all().
        This macro (colr_replace_re_all) is for string patterns.
    */


    // The string we are modifying.
    char* mystring = "This is a foo line.";
    char* pattern = "fo{2}";

    /*
        Replace a regex match with a string.
    */
    char* replaced = colr_replace_re(
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
    replaced = colr_replace_re(
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
    replaced = colr_replace_re(
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
    char* mytemplate = "This is REDuseful" NC "?";
    replaced = colr_replace_re(
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
