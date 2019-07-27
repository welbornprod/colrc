/*! Tests for the StyleValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(StyleValue) {
    subdesc(StyleValue_from_str) {
        subdesc(invalid_names) {
            it("returns COLOR_INVALID for invalid names") {
                asserteq(
                    StyleValue_from_str("NOTASTYLE"),
                    STYLE_INVALID,
                    "Invalid color name should not produce a valid StyleValue."
                );
            }
        }
        subdesc(valid_names) {
            it("returns StyleValues for known names") {
                // Test all style names, in case of some weird regression.
                for_len(style_names_len, i) {
                    char* name = style_names[i].name;
                    StyleValue bval = style_names[i].value;
                    asserteq(
                        StyleValue_from_str(name),
                        bval,
                        "Known name returned the wrong StyleValue!"
                    );
                }
            }
        }
    }
}
