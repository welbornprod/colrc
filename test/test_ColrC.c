/*! Tests for ColrC.
    Uses snow.h for testing.

    \details
    This file holds common functions for all of the tests.

    \author Christopher Welborn
    \date 06-10-2019
*/
#include "test_ColrC.h"

/*! Return the length of a ColorArgs list (`ColorArg**`), minus the `NULL` element.

    \pi lst The ColorArg list to check.
    \return The number of non-null items.
*/
size_t ColorArgs_array_len(ColorArg** lst) {
    if (!lst) return 0;
    size_t i = 0;
    while (lst[i]) i++;
    return i;
}


/*! Return the length of a string list (`char**`), minus the `NULL` element.

    \pi lst The string list to check.
    \return The number of non-null items.
*/
size_t colr_str_array_len(char** lst) {
    if (!lst) return 0;
    size_t i = 0;
    while (lst[i]) i++;
    return i;
}

/*! Creates a string representation for a list of string pointers.

    \pi lst The string list to create the representation for (`char**`).
    \return An allocated string, or `NULL` if \p lst is `NULL`, or the allocation
            fails.
*/
char* colr_str_array_repr(char** lst) {
    char* repr = NULL;
    if (!lst) {
        asprintf_or_return(NULL, &repr, "NULL");
        return repr;
    }
    if (!lst[0]) {
        asprintf_or_return(NULL, &repr, "{NULL}");
        return repr;
    }
    // Get count.
    size_t count = 0;
    for (size_t i = 0; lst[i]; i++) {
        count++;
    }
    // Get length, and string reprs.
    char* strings[count];
    size_t length = 0;
    for (size_t i = 0; lst[i]; i++) {
        char* srepr = colr_repr(lst[i]);
        length += srepr ? strlen(srepr) : 4;
        strings[i] = srepr;
    }
    // Account for last NULL item.
    length += 4;
    // Account for commas/spaces, and the NULL item.
    length += (count + 1) * 2;
    // Account for wrappers.
    length += 2;
    repr = calloc(length + 1, sizeof(char));
    char* repr_start = repr;
    repr[0] = '{';
    repr++;
    for (size_t i = 0; i < count; i++) {
        sprintf(repr, "%s", strings[i]);
        repr += strlen(strings[i]);
        sprintf(repr, ", ");
        repr += 2;
    }
    sprintf(repr, "NULL}");
    return repr_start;
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

int ioctl(int fd, unsigned long request, ...) {
    (void)fd;
    (void)request;
    return -1;
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

