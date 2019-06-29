#include "test_colr.h"

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
        cm_test(test_str_startswith)
    );
}
