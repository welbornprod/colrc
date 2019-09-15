#include "colr.h"

int main(void) {
    /*
        colr_printf registers a new format specifier, COLR_FMT, to be used
        with printf. colr_printf acts like printf when called, except Colr
        object pointers can be passed directly, and their resources will be
        free()'d automatically.

        Notice that the Colr* macros/functions are used inside of the call,
        instead of the colr* (lowercase) macros/functions. This is because
        the Colr* versions all return an allocated ColorResult that will be
        automatically free()'d. Using the lowercase versions directly will cause
        a memory leak.
    */
    colr_printf(
        "This is a Colr: %R\n",
        Colr("This", fore(RED))
    );

    /*
        Left/right justify work as expected, and a space can be used for
        center-justified text.
        %-NR : Left-justify to a width of N.
        %NR  : Right-justify to a width of N.
        % NR : Center-justify to a width of N.
    */
    colr_printf(
        "%-10R | % 10R | %10R\n",
        Colr("Left", fore(RED)),
        Colr("Center", style(UNDERLINE)),
        Colr("Right", fore(BLUE))
    );

    /*
        The alternate-form for a Colr object is a string with no escape codes.

        %#R : Print the Colr object, but do not add escape codes.
    */
    colr_printf(
        "   With colors: %R\nWithout colors: %#R\n",
        Colr("hello", fore(RED)),
        Colr("hello", fore(RED))
    );

    /*
        Other printf-like functions are available, like `sprintf`, `snprintf`,
        and `asprintf`.
    */

    // Better have room for the codes:
    size_t possible_len = 10 + CODE_ANY_LEN;
    char mystring[possible_len];
    colr_sprintf(mystring, "%R", Colr("Again.", fore(RED), style(BRIGHT)));
    puts(mystring);

    // Ensure only a certain number of bytes are written:
    colr_snprintf(mystring, possible_len, "%R", Colr("Safe?", fore(BLUE)));
    puts(mystring);

    // Allocate the string, and then fill it:
    char* myalloced = NULL;
    if (colr_asprintf(&myalloced, "This: %R", Colr("Hah!", fore("dimgrey"))) < 1) {
        // Failed to allocate.
        return EXIT_FAILURE;
    }
    puts(myalloced);
    free(myalloced);
}
