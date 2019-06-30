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

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmocka.h>
#include "../colr.h"

#define return_cm_tests(name, ...) \
    const struct CMUnitTest name##_tests[] = { \
        __VA_ARGS__ \
    }; \
    return cmocka_run_group_tests_name(#name "_tests", name##_tests, NULL, NULL)


#define runner(name) name##_test_runner()

#define cm_test(x) cmocka_unit_test(x)

extern int run_helper_tests(void);
extern int run_from_str_tests(void);
extern int run_format_bg_tests(void);
extern int run_format_fg_tests(void);
extern int run_BasicValue_tests(void);
extern int run_ColorType_tests(void);
/* Warn for any other unused macros, for gcc and clang. */
#pragma GCC diagnostic warning "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wunused-macros"



#pragma clang diagnostic pop /* end warning -Wunused-macros */
#endif /* TEST_COLR_H */
