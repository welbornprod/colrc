/*! Tests for the BasicValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_BasicValue.h"

describe(BasicValue) {
subdesc(BasicValue_eq) {
    it("compares BasicValues") {
        // This function is one line, `return a == b;`.
        // This test is here to check for regressions.
        for_len(basic_names_len, i) {
            BasicInfo info = basic_names[i];
            assert_colr_eq(info.value, info.value);
            // Test against a previous value for inequality, skipping aliases.
            size_t prev = (i < 2) ? (basic_names_len - (i + 2)) : (i - 2);
            assert_colr_neq(info.value, basic_names[prev].value);
        }
    }
}
subdesc(BasicValue_from_esc) {
    it("recognizes invalid basic codes") {
        char* invalid_args[] = {
            NULL,
            "",
            "\x1b[m",
            "not_a_code"
        };
        for_each(invalid_args, i) {
            assert_colr_eq(BasicValue_from_esc(invalid_args[i]), BASIC_INVALID);
        }
    }
    it("recognizes valid basic codes") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            char* codes = fore_str_static(bval);
            assert_colr_eq(
                BasicValue_from_esc(codes),
                bval
            );
            codes = back_str_static(bval);
            assert_colr_eq(
                BasicValue_from_esc(codes),
                bval
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
subdesc(BasicValue_is_valid) {
    it("detects valid BasicValues") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            assert_colr_repr(
                BasicValue_is_valid(bval),
                bval
            );
        }
        for_each(basic_invalid_vals, i) {
            assert_colr_repr(
                !BasicValue_is_valid(basic_invalid_vals[i]),
                basic_invalid_vals[i]
            );
        }
    }
}
subdesc(BasicValue_is_invalid) {
    it("detects invalid BasicValues") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            assert_colr_repr(
                !BasicValue_is_invalid(bval),
                bval
            );
        }
        for_each(basic_invalid_vals, i) {
            assert_colr_repr(
                BasicValue_is_invalid(basic_invalid_vals[i]),
                basic_invalid_vals[i]
            );
        }
    }
}
subdesc(BasicValue_to_ansi) {
    it("converts to ansi escape value") {
        for_len(basic_names_len, i) {
            BasicInfo info = basic_names[i];
            char codes[CODE_LEN];
            int ansival = BasicValue_to_ansi(FORE, info.value);
            sprintf(codes, "\x1b[%dm", ansival);
            ColorArg carg = ColorArg_from_esc(codes);
            assert_colr_eq(carg.type, FORE);
            assert_colr_eq(carg.value.type, TYPE_BASIC);
            assert_colr_eq(carg.value.basic, info.value);
        }
    }
    it("converts invalid values to 'reset'") {
        for_each(basic_invalid_vals, i) {
            int foreval = BasicValue_to_ansi(FORE, basic_invalid_vals[i]);
            assert_int_eq_repr(foreval, 39, foreval);
            int backval = BasicValue_to_ansi(BACK, basic_invalid_vals[i]);
            assert_int_eq_repr(backval, 49, backval);
        }
    }
}
subdesc(BasicValue_repr) {
    it("creates a repr") {
        struct {
            BasicValue bval;
            char* expected;
        } tests[] = {
            {BASIC_INVALID_RANGE, "(BasicValue) BASIC_INVALID_RANGE"},
            {BASIC_INVALID, "(BasicValue) BASIC_INVALID"},
            {BASIC_NONE, "(BasicValue) BASIC_NONE"},
            {BLACK, "(BasicValue) BLACK"},
            {RED, "(BasicValue) RED"},
            {GREEN, "(BasicValue) GREEN"},
            {YELLOW, "(BasicValue) YELLOW"},
            {BLUE, "(BasicValue) BLUE"},
            {MAGENTA, "(BasicValue) MAGENTA"},
            {CYAN, "(BasicValue) CYAN"},
            {WHITE, "(BasicValue) WHITE"},
            {UNUSED, "(BasicValue) UNUSED"},
            {RESET, "(BasicValue) RESET"},
            {LIGHTBLACK, "(BasicValue) LIGHTBLACK"},
            {LIGHTRED, "(BasicValue) LIGHTRED"},
            {LIGHTGREEN, "(BasicValue) LIGHTGREEN"},
            {LIGHTYELLOW, "(BasicValue) LIGHTYELLOW"},
            {LIGHTBLUE, "(BasicValue) LIGHTBLUE"},
            {LIGHTMAGENTA, "(BasicValue) LIGHTMAGENTA"},
            {LIGHTCYAN, "(BasicValue) LIGHTCYAN"},
            {LIGHTWHITE, "(BasicValue) LIGHTWHITE"},
        };
        for_each(tests, i) {
            char* repr = BasicValue_repr(tests[i].bval);
            assert_not_null(repr);
            assert_str_eq(repr, tests[i].expected, "Repr failed for BasicValue.");
            free(repr);
        }
        int invalid_bval = 255;
        char* invalid_bval_repr = "255";
        char* invalidrepr = BasicValue_repr(invalid_bval);
        assert_not_null(invalidrepr);
        assert_str_not_empty(invalidrepr);
        assert_str_contains(invalidrepr, invalid_bval_repr);
        free(invalidrepr);
    }
}
subdesc(BasicValue_to_str) {
    it("converts to string") {
        for_len(basic_names_len, i) {
            BasicInfo info = basic_names[i];
            BasicValue bval = info.value;
            char* namestr = BasicValue_to_str(bval);
            assert_not_null(namestr);
            if (colr_eq(bval, RESET)) {
                // Alias for "reset".
                assert_str_either(namestr, "reset", "none");
                free(namestr);
                continue;
            } else if (colr_eq(bval, WHITE)) {
                // Alias for "white".
                assert_str_either(namestr, "white", "normal");
                free(namestr);
                continue;
            } else if (colr_eq(bval, LIGHTWHITE)) {
                // Alias for "lightwhite".
                assert_str_either(namestr, "lightwhite", "lightnormal");
                free(namestr);
                continue;
            }
            assert_str_eq(namestr, info.name, "Names are mismatched.");
            free(namestr);
        }
        // An unknown name.
        BasicValue invalid = 255;
        char* invalidstr = BasicValue_to_str(invalid);
        assert_not_null(invalidstr);
        assert_str_not_empty(invalidstr);
        assert_str_contains(invalidstr, "unknown");
        free(invalidstr);
    }
}
} // describe(BasicValue)
