#include "colr.h"

int main(void) {
    /*
        Colr is shorthand for building a ColorText struct.
        If you use them outside of the colr macros, you must free them using
        ColorText_free().
        Again, you must always free the plain text if you allocated for it.
    */
    ColorText* ctext = Colr("I didn't allocate this.", fore(BLUE));
    /*
        If you use the *to_str functions directly,
        you must free the resulting string.
    */
    char* ctext_str = ColorText_to_str(*ctext);
    printf("But I allocated the resulting string: %s\n", ctext_str);
    // No leaks, free the ColorText's resources.
    ColorText_free(ctext);
    // And free the string you created from it.
    free(ctext_str);

    // Another way to do that would be:
    ColorText* piece = Colr("Just another string", back(ext(255)), fore(ext(0)));
    char *text_str = colr_cat(piece);
    printf("%s\n", text_str);
    // The ColorText `piece` is gone. `colr_cat()` called `free()` on it.
    // Still, we need to free the result.
    free(text_str);
}
