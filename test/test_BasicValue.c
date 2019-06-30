/*! Tests for the BasicValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_colr.h"

static void test_BasicValue_from_str(void **state) {
    /*  Tests BasicValue_from_str basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    assert_true(BasicValue_from_str("NOTACOLOR") == COLOR_INVALID);
}

int run_BasicValue_tests(void) {
    return_cm_tests(
        BasicValue,
        cm_test(test_BasicValue_from_str),
    );
}
