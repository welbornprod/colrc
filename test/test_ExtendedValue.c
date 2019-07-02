/*! Tests for the ExtendedValue enum, and it's related functions.

    \author Christopher Welborn
    \date 07-01-2019
*/
#include "test_colr.h"

static void test_ExtendedValue_from_str(void **state) {
    /*  Tests ExtendedValue_from_str basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    assert_true(ExtendedValue_from_str("NOTACOLOR") == COLOR_INVALID);
    // Test all basic names, in case of some weird regression.
    for (size_t i = 0; i < extended_names_len; i++) {
        char *name = extended_names[i].name;
        ExtendedValue bval = extended_names[i].value;
        assert_true(ExtendedValue_from_str(name) == bval);
    }
    char numstr[4];
    // Test all 0-255 values as strings.
    for (int i = 0; i < 256; i++) {
        sprintf(numstr, "%d", i);
        int eval = ExtendedValue_from_str(numstr);
        assert_true(i == eval);
        assert_true((i >= 0) && (i <= 255));
        assert_false(eval == EXTENDED_INVALID);
        assert_false(eval == COLOR_INVALID_RANGE);

    }
    // Test bad numbers.
    int nums[] = {-255, -1, 256, 1337};
    size_t nums_len = sizeof(nums) / sizeof(nums[0]);
    for (size_t i = 0; i < nums_len; i++) {
        sprintf(numstr, "%d", nums[i]);
        int eval = ExtendedValue_from_str(numstr);
        assert_true(eval == COLOR_INVALID_RANGE);
    }
}

int run_ExtendedValue_tests(void) {
    return_cm_tests(
        ExtendedValue,
        cm_test(test_ExtendedValue_from_str),
    );
}
