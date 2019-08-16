/*! Tests for the BasicValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(BasicValue) {
subdesc(BasicValue_from_esc) {
    it("recognizes valid basic codes") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            char codes[CODE_LEN];
            format_fg(codes, bval);
            asserteq(
                BasicValue_from_esc(codes),
                bval,
                "Failed to recognize basic fore code."
            );
            format_bg(codes, bval);
            asserteq(
                BasicValue_from_esc(codes),
                bval,
                "Failed to recognize basic back code."
            );
        }
    }
}
subdesc(BasicValue_from_str) {
    it("returns COLOR_INVALID for invalid names") {
        char* badnames[] = {
            NULL,
            "",
            "NOTACOLOR",
        };
        for_each(badnames, i) {
            asserteq(
                BasicValue_from_str(badnames[i]),
                COLOR_INVALID,
                "Invalid color name should not produce a valid BasicValue."
            );
        }
    }
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
} // describe(BasicValue)
