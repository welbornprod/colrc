/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_colr.h"

static void test_str_endswith(void **state) {
    /*  Tests str_endswith.
    */
    (void)state; // Unused.
    // Common uses.
    assert_true(str_endswith("lightblue", "blue"));
    assert_true(str_endswith("xred", "red"));
    assert_true(str_endswith("yellow", "low"));
    assert_true(str_endswith("!@#$^&*", "&*"));
    assert_true(str_endswith("    test    ", "    "));
    // Should not trigger a match.
    assert_false(str_endswith("test", "a"));
    assert_false(str_endswith(" test ", "test"));
    assert_false(str_endswith("t", "apple"));
    assert_false(str_endswith(NULL, "a"));
    assert_false(str_endswith("test", NULL));
    assert_false(str_endswith(NULL, NULL));
}

static void test_str_startswith(void **state) {
    /*  Tests str_startswith.
    */
    (void)state; // Unused.
    // Common uses.
    assert_true(str_startswith("lightblue", "light"));
    assert_true(str_startswith("xred", "x"));
    assert_true(str_startswith("yellow", "yel"));
    assert_true(str_startswith("!@#$^&*", "!@"));
    assert_true(str_startswith("    test", "    "));
    // Should not trigger a match.
    assert_false(str_startswith("test", "a"));
    assert_false(str_startswith(" test", "test"));
    assert_false(str_startswith("t", "apple"));
    assert_false(str_startswith(NULL, "a"));
    assert_false(str_startswith("test", NULL));
    assert_false(str_startswith(NULL, NULL));
}

int run_helper_tests(void) {
    return_cm_tests(
        helper,
        cm_test(test_str_endswith),
        cm_test(test_str_startswith),
    );
}
