/*! Tests for the ExtendedValue enum, and it's related functions.

    \author Christopher Welborn
    \date 07-01-2019
*/
#include "test_ColrC.h"
#include "test_ExtendedValue.h"

describe(ExtendedValue) {
    subdesc(ExtendedValue_from_str) {
        subdesc(invalid_colors) {
            it("invalid color names should return COLOR_INVALID") {
                assert_ext_from_str_eq(
                    "NOTACOLOR",
                    COLOR_INVALID,
                    "Invalid color name should be an invalid ExtendedValue"
                );
            }
        }
        subdesc(valid_colors) {
            it("recognizes known color names") {
                // Test all basic names, in case of some weird regression.
                for (size_t i = 0; i < extended_names_len; i++) {
                    char* name = extended_names[i].name;
                    int bval = extended_names[i].value;
                    assert_ext_from_str_eq(
                       name,
                        bval,
                        "Known ExtendedValue didn't match"
                    );
                }
            }
            it("recognizes known extended numbers") {
                char numstr[4];
                // Test all 0-255 values as strings.
                for (int i = 0; i < 256; i++) {
                    sprintf(numstr, "%d", i);
                    int eval = ExtendedValue_from_str(numstr);
                    assert_ext_eq(i, eval, "Known extended number was considered invalid");
                    assert(
                        (eval >= 0) && (eval <= 255),
                        "ExtendedValue within range returned invalid."
                    );
                    assert_ext_neq(
                        eval,
                        EXTENDED_INVALID,
                        "Known extended number caused an invalid range"
                    );
                    assert_ext_neq(
                        eval,
                        COLOR_INVALID_RANGE,
                        "Known extended number caused an invalid range"
                    );

                }
            }
            it("returns COLOR_INVALID_RANGE for bad numbers") {
                // Test bad numbers.
                char numstr[4];
                int nums[] = {-255, -1, 256, 355};
                size_t nums_len = array_length(nums);
                for (size_t i = 0; i < nums_len; i++) {
                    sprintf(numstr, "%d", nums[i]);
                    int eval = ExtendedValue_from_str(numstr);
                    assert_ext_eq(
                        eval,
                        COLOR_INVALID_RANGE,
                        "Bad number was not considered outside of the range"
                    );
                }
            }
            it("returns COLOR_INVALID for really bad numbers") {
                // numstr needs to hold the largest stringified number.
                char numstr[11];
                int nums[] = {-2555, -1000, 2560, 2147483647};
                size_t nums_len = array_length(nums);
                for (size_t i = 0; i < nums_len; i++) {
                    sprintf(numstr, "%d", nums[i]);
                    assert_ext_from_str_eq(
                        numstr,
                        COLOR_INVALID_RANGE,
                        "Bad number was not considered invalid"
                    );
                }
                // Something like: 18446744073709551615
                unsigned long long largest_num = ULLONG_MAX;
                char largenumstr[21];
                sprintf(largenumstr, "%llu", largest_num);
                assert_ext_from_str_eq(
                    largenumstr,
                    COLOR_INVALID_RANGE,
                    "Really long number was not considered invalid"
                );
            }
        }
    }
}
