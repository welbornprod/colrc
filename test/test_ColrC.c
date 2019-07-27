/*! Tests for ColrC.
    Uses snow.h for testing.

    \details
    This file holds common functions for all of the tests.

    \author Christopher Welborn
    \date 06-10-2019
*/
#include "test_ColrC.h"

/*! Creates a string representation for a char.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* char_repr(char c) {
    char* repr;
    switch (c) {
        case '\0':
            asprintf_or_return(NULL, &repr, "'\\0'");
            break;
        case '\'':
            asprintf_or_return(NULL, &repr, "'\\\'");
            break;
        case '\"':
            asprintf_or_return(NULL, &repr, "'\\\"'");
            break;
        case '\?':
            asprintf_or_return(NULL, &repr, "'\\?'");
            break;
        case '\\':
            asprintf_or_return(NULL, &repr, "'\\\\");
            break;
        case '\a':
            asprintf_or_return(NULL, &repr, "'\\a'");
            break;
        case '\b':
            asprintf_or_return(NULL, &repr, "'\\b'");
            break;
        case '\f':
            asprintf_or_return(NULL, &repr, "'\\f'");
            break;
        case '\n':
            asprintf_or_return(NULL, &repr, "'\\n'");
            break;
        case '\r':
            asprintf_or_return(NULL, &repr, "'\\r'");
            break;
        case '\t':
            asprintf_or_return(NULL, &repr, "'\\t'");
            break;
        case '\v':
            asprintf_or_return(NULL, &repr, "'\\v'");
            break;
        default:
            asprintf_or_return(NULL, &repr, "'%c'", c);
    }
    return repr;
}

/*! Creates a string representation for an int.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* int_repr(int x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%d", x);
    return repr;
}

/*! Creates a string representation for a long.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* long_repr(long x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%ld", x);
    return repr;
}

/*! Creates a string representation for a long long.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* long_long_repr(long long x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%lld", x);
    return repr;
}

/*! Creates a string representation for an unsigned int.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* uint_repr(unsigned int x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%u", x);
    return repr;
}

/*! Creates a string representation for an unsigned long.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* ulong_repr(unsigned long x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%lu", x);
    return repr;
}

/*! Creates a string representation for an unsigned long long.

    \pi x Value to create the representation for.
    \return An allocated string, or `NULL` if the allocation fails.
*/
char* ulong_long_repr(unsigned long long x) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%llu", x);
    return repr;
}

