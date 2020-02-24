#include "colr.h"

int main(void) {

    // The string we are modifying.
    char* mystring = "This is a foo line.";
    char* pattern = "foo";

    /*
        Replace a string with a string.
    */
    char* replaced = colr_replace(
        mystring,
        pattern,
        "replaced"
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a string with a ColorText.
    */
    replaced = colr_replace(
        mystring,
        pattern,
        Colr("replaced", fore(RED))
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a string with a ColorResult.
    */
    replaced = colr_replace(
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
    char* mytemplate = "This is REDuseful?" NC;
    replaced = colr_replace(
        mytemplate,
        "RED",
        fore(RED)
    );
    // Failed to allocate for new replaced string?
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    printf("%s\n", replaced);
    free(replaced);

    /*
        Replace a compiled regex pattern with a ColorText.
    */
    char* mypatstring = "I think this is a beautiful thing.";
    regex_t mypat;
    if (regcomp(&mypat, "th[a-z]+", REG_EXTENDED)) {
        regfree(&mypat);
        fprintf(stderr, "Failed to compile regex!\n");
        return EXIT_FAILURE;
    }
    replaced = colr_replace(mypatstring, &mypat, Colr("know", fore(BLUE)));
    // We don't need the pattern anymore, `free()` it.
    regfree(&mypat);
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    puts(replaced);
    free(replaced);

    return EXIT_SUCCESS;
}
