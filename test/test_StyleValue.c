/*! Tests for the StyleValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(StyleValue) {
subdesc(StyleValue_eq) {
    it("compares StyleValues") {
        struct {
            StyleValue a;
            StyleValue b;
            bool expected;
        } tests[] = {
            {BRIGHT, BRIGHT, true},
            {RESET_ALL, RESET_ALL, true},
            {BRIGHT, RESET_ALL, false},
            {0, 0, true},
            {1, 2, false},
        };
        for_each(tests, i) {
            bool result = StyleValue_eq(tests[i].a, tests[i].b);
            assert(result == tests[i].expected);
        }
    }
}
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
subdesc(StyleValue_is_invalid) {
    it("detects invalid StyleValues") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            // Creates valid StyleValues.
            {"bright", false},
            {"bold", false},
            {"reset_all", false},
            // Creates invalid StyleValues.
            {"bad", true},
            {"nope", true},
            {"", true},
        };
        for_each(tests, i) {
            StyleValue sval = StyleValue_from_str(tests[i].name);
            bool result = StyleValue_is_invalid(sval);
            assert(result == tests[i].expected);
        }
    }
}
subdesc(StyleValue_is_valid) {
    it("detects valid StyleValues") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            // Creates valid StyleValues.
            {"bright", true},
            {"bold", true},
            {"reset_all", true},
            // Creates invalid StyleValues.
            {"bad", false},
            {"nope", false},
            {"", false},
        };
        for_each(tests, i) {
            StyleValue sval = StyleValue_from_str(tests[i].name);
            bool result = StyleValue_is_valid(sval);
            assert(result == tests[i].expected);
        }
    }
}
subdesc(StyleValue_repr) {
    it("creates a StyleValue repr") {
        struct {
            StyleValue sval;
            char* marker;
        } tests[] = {
            {STYLE_INVALID, "STYLE_INVALID"},
            {STYLE_INVALID_RANGE, "STYLE_INVALID_RANGE"},
            {STYLE_NONE, "STYLE_NONE"},
            {RESET_ALL, "RESET_ALL"},
            {BRIGHT, "BRIGHT"},
            {DIM, "DIM"},
            {ENCIRCLE, "ENCIRCLE"},
            {FLASH, "FLASH"},
            {FRAME, "FRAME"},
            {HIGHLIGHT, "HIGHLIGHT"},
            {ITALIC, "ITALIC"},
            {NORMAL, "NORMAL"},
            {OVERLINE, "OVERLINE"},
            {STRIKETHRU, "STRIKETHRU"},
            {UNDERLINE, "UNDERLINE"},
        };
        for_each(tests, i) {
            char* s = StyleValue_repr(tests[i].sval);
            assert_not_null(s);
            assert_str_not_empty(s);
            assert_str_contains(s, tests[i].marker);
            free(s);
        }
        StyleValue invalid = 86;
        char* invalid_repr = "86";
        char* invalidstr = StyleValue_repr(invalid);
        assert_not_null(invalidstr);
        assert_str_not_empty(invalidstr);
        assert_str_contains(invalidstr, invalid_repr);
        free(invalidstr);
    }
}
subdesc(StyleValue_to_str) {
    it("creates strings from StyleValues") {
        StyleValue tests[] = {
            StyleValue_from_str("bold"),
            StyleValue_from_str("bright"),
            StyleValue_from_str("underline"),
        };
        for_each(tests, i) {
            char* s = StyleValue_to_str(tests[i]);
            assert_not_null(s);
            assert_str_not_empty(s);
            free(s);
        }
        // An unknown name.
        StyleValue invalid = 255;
        char* invalidstr = StyleValue_to_str(invalid);
        assert_not_null(invalidstr);
        assert_str_not_empty(invalidstr);
        assert_str_contains(invalidstr, "unknown");
        free(invalidstr);

    }
}
} // describe(StyleValue)
