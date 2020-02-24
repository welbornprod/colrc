#include "colr.h"

/*
    These are needed, even though the format specifier is registered with
    colr_printf_register(). The colr_printf() macros temporarily add these
    pragmas around each call, but if you use the standard printf-functions you
    will need to deal with the warnings yourself.
*/
#pragma clang diagnostic ignored "-Wformat-invalid-specifier"

#pragma GCC diagnostic ignored "-Wformat="
#pragma GCC diagnostic ignored "-Wformat-extra-args"

int main(void) {
    /*
        To use a Colr object with the standard printf-family functions,
        you must call colr_printf_register() at some point before calling
        the printf functions.

        This will register a new format specifier, COLR_FMT_CHAR, to be used
        with printf and friends.
    */
    colr_printf_register();

    /*
        Notice that the Colr* macros/functions are used inside of the call,
        instead of the colr* (lowercase) macros/functions. This is because
        the Colr* versions all return an allocated ColorResult that will be
        automatically free()'d. Using the lowercase versions directly will cause
        a memory leak.
    */
    printf(
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
    printf(
        "%-10R | % 10R | %10R\n",
        Colr("Left", fore(RED)),
        Colr("Center", style(UNDERLINE)),
        Colr("Right", fore(BLUE))
    );

    /*
        The alternate-form for a Colr object is a string with no escape codes.

        %#R : Print the Colr object, but do not add escape codes.
    */
    printf(
        "   With colors: %R\nWithout colors: %#R\n",
        Colr("hello", fore(RED)),
        Colr("hello", fore(RED))
    );

    /*
        A custom modifier was added (`/`), to allow for escaped output.

        %/R : Print the Colr object, with the output string escaped.
    */
    printf(
        "        Normal: %R\n       Escaped: %/R\n",
        Colr("okay", fore(RED)),
        Colr("okay", fore(RED))
    );

    /*
        Other printf-like functions are available, like `sprintf`, `snprintf`,
        and `asprintf`.
    */

    // Better have room for the codes:
    size_t possible_len = 10 + CODE_ANY_LEN;
    char mystring[possible_len];
    sprintf(mystring, "%R", Colr("Again.", fore(RED), style(BRIGHT)));
    puts(mystring);

    // Ensure only a certain number of bytes are written:
    snprintf(mystring, possible_len, "%R", Colr("Safe?", fore(BLUE)));
    puts(mystring);

    // Allocate the string, and then fill it:
    char* myalloced = NULL;
    if (asprintf(&myalloced, "This: %R", Colr("Hah!", fore("dimgrey"))) < 1) {
        // Failed to allocate.
        return EXIT_FAILURE;
    }
    puts(myalloced);
    free(myalloced);
}
