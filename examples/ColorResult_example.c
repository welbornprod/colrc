#include "colr.h"

int main(void) {
    /*
        ColorResults mark an *allocated* string as "safe to free()" in the
        Colr macros/functions. You can wrap your own allocated strings by
        calling `ColrResult(mystring)`. Colr uses this behind the scenes to
        implement the Colr_join macro, which allows nested joins.
    */

    // Colr tries to make things easy, so you don't have to do this.
    // But if you *have to*, ColrResult will help you.
    // This example wouldn't need ColrResult if you used Colr_join instead,
    // which returns an allocated ColorResult itself.
    char* joined = colr_cat(
        ColrResult(colr_join(
            ColrResult(colr_join(
                ": ",
                Colr("debug", fore(GREEN)),
                Colr("This is a test.", fore(CYAN))
            )),
            "[",
            "]"
        )),
        "\nStack-allocated.",
        ColrResult(strdup("\nHeap-allocated for no reason."))
    );
    if (!joined) return EXIT_FAILURE;
    printf("%s\n", joined);
    // All your left with is the final allocated string result.
    free(joined);

    /*
        Without ColorResult/ColrResult, Colr will never call `free()` on your
        strings, or the strings created by Colr:
    */
    char* mine = strdup("I need this for later, don't free it.");
    if (!mine) return EXIT_FAILURE;
    char* colorized = colr(mine, fore(BLUE), back(WHITE));
    if (!colorized) return EXIT_FAILURE;
    printf("%s\n", colorized);
    // Your string is still good:
    printf("%s\n", mine);

    char* appended = colr_cat(colorized, "...still here.");
    if (!appended) return EXIT_FAILURE;
    printf("%s\n", appended);
    // The Colr-allocated string is still good:
    printf("%s\n", colorized);

    // Most colorization is a one-shot thing that doesn't need to stick
    // around, so these examples are here *just in case* you have to do this.
    // Watch these disappear when wrapped in a ColorResult and sent through
    // the colr functions/macros:
    char* final = colr_join(
        "\n",
        ColrResult(mine),
        ColrResult(colorized),
        ColrResult(appended)
    );
    if (!final) return EXIT_FAILURE;
    printf("%s\n", final);

    // All those allocations, and it's down to just the last call to colr_join().
    free(final);

    /*
        Colr_join() returns an allocated ColorResult itself, so if you were
        to use it outside of the colr macros/functions you would need to
        deal with printing/freeing it:
    */
    ColorResult* result = Colr_join(
        "\n",
        Colr("This is a line.", fore(ext_rgb(255, 128, 128))),
        ColrResult(colr_cat(
            Colr("This is another", style(UNDERLINE)),
            "."
        )),
        Colr_join("This is the final line.", "[", "]")
    );
    if (!result) return EXIT_FAILURE;
    // This actually compiles as: ColorResult_to_str(*result).
    printf("%s\n", colr_to_str(*result));

    // And, finally release the resources.
    // This actually ends up calling ColorResult_free(result) in the end:
    colr_free(result);

    /*
        Run this example through valgrind/libasan (-fsanitize=leak).
    */
}
