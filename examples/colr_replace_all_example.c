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
    puts(replaced);
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
    puts(replaced);
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
    puts(replaced);
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
    puts(replaced);
    free(replaced);

    /*
        Replace a `NULL`-terminated array of regex matches with a ColorText.
    */
    char* mymatchstring = "I think this is a beautiful thing.";
    regex_t pat;
    if (regcomp(&pat, "th[a-z]+", REG_EXTENDED)) {
        regfree(&pat);
        fprintf(stderr, "Failed to compile regex!\n");
        return EXIT_FAILURE;
    }
    // `colr_re_matches` returns a `NULL`-terminated array of regex matches.
    regmatch_t** matches = colr_re_matches(mymatchstring, &pat);
    // We don't need the pattern anymore, `free()` it.
    regfree(&pat);
    if (!matches) {
        // Impossible (for this example).
        colr_free(matches);
        fprintf(stderr, "Failed to match anything!\n");
        return EXIT_FAILURE;
    }
    replaced = colr_replace_all(mymatchstring, matches, Colr("uhhh", fore(RED)));
    // We don't need the matches anymore, `free()` them.
    // You must use colr_free_re_matches() or the colr_free() macro.
    colr_free(matches);
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    puts(replaced);
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
    replaced = colr_replace_all(mypatstring, &mypat, Colr("..uh", fore(BLUE)));
    // We don't need the pattern anymore, `free()` it.
    regfree(&mypat);
    if (!replaced) return EXIT_FAILURE;
    // Print the result and `free()` it.
    puts(replaced);
    free(replaced);
    return EXIT_SUCCESS;
}
