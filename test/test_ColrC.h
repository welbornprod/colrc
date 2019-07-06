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
// snow's describe() macro triggers a -Wstrict-prototypes warning,
// "function declaration isn't a prototype."
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"
// snow redefines the `assert()` macro. It's better though.
#undef assert
// snow uses typeof(), which the clang linters hate.
#ifdef __clang__
    #define typeof(x) __typeof__(x)
#else
    #define typeof(x) __typeof(x)
#endif
#define SNOW_ENABLED
#include "snow.h"

#define array_length(array) (sizeof(array) / sizeof(array[0]))
#define in_range(x, xmin, xmax) ((bool)((x >= xmin) && (x <= xmax)))
#define assert_range(x, xmin, xmax, ...) assert(in_range(x, xmin, xmax), __VA_ARGS__)
#pragma clang diagnostic pop /* end warning -Wunused-macros */
#endif /* TEST_COLR_H */
