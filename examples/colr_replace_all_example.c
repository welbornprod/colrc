#include "colr.h"

int main(void) {

    // The string we are modifying.
    char* mystring = "This was foo. I mean foo.";
    char* pattern = "foo";

    /*
        Replace a string with a string.
    */
    char* replaced = colr_replace_all(
        mystring,
        pattern,
        "replacement"
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a string with a ColorText.
    */
    replaced = colr_replace_all(
        mystring,
        pattern,
        Colr("replacement", fore(RED))
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a string with a ColorResult.
    */
    replaced = colr_replace_all(
        mystring,
        pattern,
        Colr_join(
            " ",
            Colr("really", style(BRIGHT)),
            Colr("replaced", fore(BLUE))
        )
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a string with a ColorResult.
    */
    char* mytemplate = "This REDis " NC "kinda REDuseful" NC "?";
    replaced = colr_replace_all(
        mytemplate,
        "RED",
        fore(RED)
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    return EXIT_SUCCESS;
}
