/*! Tests for the BasicValue enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

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
        // BASIC_NONE is a valid value.
        BasicValue badvals[] = {
            BASIC_INVALID_RANGE,
            BASIC_INVALID,
        };
        for_each(badvals, i) {
            assert_colr_repr(
                !BasicValue_is_valid(badvals[i]),
                badvals[i]
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
        // BASIC_NONE is a valid value.
        BasicValue badvals[] = {
            BASIC_INVALID_RANGE,
            BASIC_INVALID,
        };
        for_each(badvals, i) {
            assert_colr_repr(
                BasicValue_is_invalid(badvals[i]),
                badvals[i]
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
}
subdesc(BasicValue_to_str) {
    it("converts to string") {
        for_len(basic_names_len, i) {
            BasicInfo info = basic_names[i];
            BasicValue bval = info.value;
            char* namestr = BasicValue_to_str(bval);
            assert_not_null(namestr);
            if (colr_eq(bval, RESET) && colr_str_either(namestr, "reset", "none")) {
                // Alias for "reset".
                free(namestr);
                continue;
            } else if (colr_eq(bval, WHITE) && colr_str_either(namestr, "white", "normal")) {
                // Alias for "white".
                free(namestr);
                continue;
            } else if (colr_eq(bval, LIGHTWHITE) && colr_str_either(namestr, "lightwhite", "lightnormal")) {
                // Alias for "lightwhite".
                free(namestr);
                continue;
            }
            assert_str_eq(namestr, info.name, "Names are mismatched.");
            free(namestr);
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
    }
}
} // describe(BasicValue)
