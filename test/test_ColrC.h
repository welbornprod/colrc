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
#define assert_range(x, xmin, xmax, ...) assert(in_range(x, xmin, xmax), __VA_ARGS__)
#define in_range(x, xmin, xmax) ((bool)((x >= xmin) && (x <= xmax)))

// These two are not used right now.
#define def_test_struct(name, members) struct { members } name[]
#define def_tests(name, members, ...) def_test_struct(name, members) = {  __VA_ARGS__ }

#define for_each(array_name, x) \
    size_t _fe_ ## array_name ## _len = array_length(array_name); \
    for (size_t x = 0; x < _fe_ ## array_name ## _len; x++)

#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif /* TEST_COLR_H */
