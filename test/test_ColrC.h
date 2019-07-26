/*! Tests for the ColrC library.

    \author Christopher Welborn
    \date 06-29-2019
*/


#ifndef TEST_COLR_H
/* Tell gcc to ignore this unused inclusion macro. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* Tell gcc to ignore clang pragmas, for linting. */
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
/* Tell clang to ignore this unused inclusion macro. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define TEST_COLR_H
#pragma clang diagnostic pop
/* Warn for any other unused macros, for gcc and clang. */
#pragma GCC diagnostic warning "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wunused-macros"

#include "../colr.h"

// snow redefines the `assert()` macro. It's better though.
#undef assert
#define SNOW_ENABLED
#include "snow.h"

#define array_length(array) (sizeof(array) / sizeof(array[0]))
#define assert_range(x, xmin, xmax, msg) \
    do { \
        char* _a_r_msg; \
        char* _a_r_x_repr = test_repr(x); \
        char* _a_r_xmin_repr = test_repr(xmin); \
        char* _a_r_xmax_repr = test_repr(xmax); \
        asprintf_or(&_a_r_msg, "%s (%s): %s-%s", msg, _a_r_x_repr, _a_r_xmin_repr, _a_r_xmax_repr) { \
            fail("Allocation failed for failure message!"); \
        } \
        free(_a_r_x_repr); \
        free(_a_r_xmin_repr); \
        free(_a_r_xmax_repr); \
        if (!in_range(x, xmin, xmax)) { \
            fail("%s", _a_r_msg); \
        } \
        free(_a_r_msg); \
    } while (0)
#define assert_str_eq(s1, s2, msg) \
    do { \
        char* _a_s_e_s1_repr = str_repr(s1); \
        char* _a_s_e_s2_repr = str_repr(s2); \
        if (strcmp(s1, s2) != 0) { \
            fail("%s: %s != %s", msg, _a_s_e_s1_repr, _a_s_e_s2_repr); \
        } \
        free(_a_s_e_s1_repr); \
        free(_a_s_e_s2_repr); \
    } while (0)
#define test_repr(x) \
    _Generic( \
        (x), \
        char: char_repr, \
        int: int_repr, \
        unsigned int: uint_repr, \
        long: long_repr, \
        long long: long_long_repr, \
        unsigned long: ulong_repr, \
        unsigned long long: ulong_long_repr \
    )(x)

#define in_range(x, xmin, xmax) ((bool)((x >= xmin) && (x <= xmax)))

/*! Construct a for-loop to iterate over an array, where `x` is the index.

    \details
    The index is declared `volatile` because a `longjmp` may occur while
    testing in `snow`.

    \pi array_name The name of the array to iterate over.
    \pi x          The index variable to use (usually `i`).
*/
#define for_each(array_name, x) \
    size_t _fe_ ## array_name ## _len = array_length(array_name); \
    for (volatile size_t x = 0; x < _fe_ ## array_name ## _len; x++)

/*! Construct a for-loop to iterate from `0` to `len`, where `x` is the index.

    \details
    The index is declared `volatile` because a `longjmp` may occur while
    testing in `snow`.

    \pi len Stopping point. The final index will be `len - 1`.
    \pi x   The index variable to use (usually `i`).
*/
#define for_len(len, x) \
    for (volatile size_t x = 0; x < len; x++)


#define _str(x) #x

char* char_repr(char x);
char* int_repr(int x);
char* long_repr(long x);
char* long_long_repr(long long x);
char* uint_repr(unsigned int x);
char* ulong_repr(unsigned long x);
char* ulong_long_repr(unsigned long long x);


#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif /* TEST_COLR_H */
