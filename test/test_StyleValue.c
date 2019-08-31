/*! Tests for the StyleValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(StyleValue) {
subdesc(StyleValue_from_esc) {
    it("recognizes valid style codes") {
        for_len(style_names_len, i) {
            StyleValue sval = style_names[i].value;
            char* codes = style_str_static(sval);
            assert_colr_eq(StyleValue_from_esc(codes), sval);
        }
    }
}
subdesc(StyleValue_from_str) {
    it("returns STYLE_INVALID for invalid names") {
        char* badnames[] = {
            NULL,
            "",
            "NOTASTYLE",
        };
        for_each(badnames, i) {
            assert_colr_eq(
                StyleValue_from_str(badnames[i]),
                STYLE_INVALID
            );
        }
    }
    it("returns StyleValues for known names") {
        // Test all style names, in case of some weird regression.
        for_len(style_names_len, i) {
            char* name = style_names[i].name;
            StyleValue bval = style_names[i].value;
            assert_colr_eq(
                StyleValue_from_str(name),
                bval
            );
        }
    }

}
}
