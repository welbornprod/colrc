#include "colr.h"

int main(void) {
    /*
        You can join things by a plain string or a colorized string.

        For the pieces, the order/number of arguments does not matter.
        colr_join() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char* colorized = colr_join(
        "\n",
        "This is a plain line.",
        Colr("This one is some kind of purple.", fore(rgb(125, 0, 155))),
        Colr("This one is bright.", style(BRIGHT)),
        "Another plain one, why not?"
    );
    if (!colorized) return 1;
    // Prints each colorized piece of text on it's own line:
    printf("%s\n", colorized);
    free(colorized);

    /*
        The joiner can be a ColorText, string, or ColorArg (though ColorArgs
        would be kinda useless).
    */
    char* final = colr_join(
        Colr(" <--> ", fore(ext_hex("#353535")), style(UNDERLINE)),
        "This",
        Colr(" that ", fore(RED)),
        "the other."
    );
    if (!final) return 1;
    // Prints each piece, joined by a colorized " <--> ".
    printf("%s\n", final);
    free(final);

    /*
        Arrays of ColorText, ColorArgs, or strings can be used with
        colr_join_array().
    */
    char* joiner = " [and] ";
    ColorText* words[] = {
        Colr("this", fore(RED)),
        Colr("that", fore(hex("ff3599"))),
        Colr("the other", fore(BLUE), style(UNDERLINE)),
        // The last member must be NULL.
        NULL
    };
    char* s = colr_join_array(joiner, words);
    if (!s) {
        // Couldn't allocate memory for the final string.
        for (size_t i = 0; words[i]; i++) ColorText_free(words[i]);
        return 1;
    }
    printf("%s\n", s);
    free(s);

    // Don't forget to free your ColorTexts/ColorArgs.
    for (size_t i = 0; words[i]; i++) ColorText_free(words[i]);
}
