/*! Tests for the BasicValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(BasicValue) {
    subdesc(BasicValue_from_str) {
        subdesc(invalid_names) {
            it("returns COLOR_INVALID for invalid names") {
                asserteq(
                    BasicValue_from_str("NOTACOLOR"),
                    COLOR_INVALID,
                    "Invalid color name should not produce a valid BasicValue."
                );
            }
        }
        subdesc(valid_names) {
            it("returns BasicValues for known names") {
                // Test all basic names, in case of some weird regression.
                for_len(basic_names_len, i) {
                    char* name = basic_names[i].name;
                    BasicValue bval = basic_names[i].value;
                    asserteq(
                        BasicValue_from_str(name),
                        bval,
                        "Known name returned the wrong BasicValue!"
                    );
                }
            }
        }
    }
}
