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
    // Test all basic names, in case of some weird regression.
    for (size_t i = 0; i < basic_names_len; i++) {
        char *name = basic_names[i].name;
        BasicValue bval = basic_names[i].value;
        assert_true(BasicValue_from_str(name) == bval);
    }
}

int run_BasicValue_tests(void) {
    return_cm_tests(
        BasicValue,
        cm_test(test_BasicValue_from_str),
    );
}
