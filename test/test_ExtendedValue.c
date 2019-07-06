/*! Tests for the ExtendedValue enum, and it's related functions.

    \author Christopher Welborn
    \date 07-01-2019
*/
#include "test_ColrC.h"

describe(ExtendedValue) {
    subdesc(ExtendedValue_from_str) {
        subdesc(invalid_colors) {
            it("invalid color names should return COLOR_INVALID") {
                asserteq(
                    ExtendedValue_from_str("NOTACOLOR"),
                    COLOR_INVALID,
                    "Invalid color name should be an invalid ExtendedValue."
                );
            }
        }
        subdesc(valid_colors) {
            it("recognizes known color names") {
                // Test all basic names, in case of some weird regression.
                for (size_t i = 0; i < extended_names_len; i++) {
                    char* name = extended_names[i].name;
                    int bval = extended_names[i].value;
                    asserteq(
                        ExtendedValue_from_str(name),
                        bval,
                        "Known ExtendedValue didn't match."
                    );
                }
            }
            it("recognizes known extended numbers") {
                char numstr[4];
                // Test all 0-255 values as strings.
                for (int i = 0; i < 256; i++) {
                    sprintf(numstr, "%d", i);
                    int eval = ExtendedValue_from_str(numstr);
                    asserteq(i, eval, "Known extended number was considered invalid.");
                    assert(
                        (eval >= 0) && (eval <= 255),
                        "ExtendedValue within range returned invalid."
                    );
                    assertneq(
                        eval,
                        EXTENDED_INVALID,
                        "Known extended number caused an invalid range."
                    );
                    assertneq(
                        eval,
                        COLOR_INVALID_RANGE,
                        "Known extended number caused an invalid range."
                    );

                }
            }
            it("returns COLOR_INVALID_RANGE for bad numbers") {
                // Test bad numbers.
                char numstr[4];
                int nums[] = {-255, -1, 256, 1337};
                size_t nums_len = array_length(nums);
                for (size_t i = 0; i < nums_len; i++) {
                    sprintf(numstr, "%d", nums[i]);
                    int eval = ExtendedValue_from_str(numstr);
                    asserteq(
                        eval,
                        COLOR_INVALID_RANGE,
                        "Bad number was not considered outside of the range."
                    );
                }
            }
        }
    }
}
