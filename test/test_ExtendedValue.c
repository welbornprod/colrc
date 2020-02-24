/*! Tests for the ExtendedValue enum, and it's related functions.

    \author Christopher Welborn
    \date 07-01-2019
*/
#include "test_ColrC.h"
#include "test_ExtendedValue.h"

describe(ExtendedValue) {
subdesc(ExtendedValue_eq) {
    it("compares ExtendedValues") {
        struct {
            ExtendedValue a;
            ExtendedValue b;
            bool expected;
        } tests[] = {
            {ext(1), ext(1), true},
            {ext(245), ext(245), true},
            {ext(0), ext(0), true},
            {ext(1), ext(2), false},
            {ext(245), ext(1), false},
            {ext(1), ext(78), false},
        };
        for_each(tests, i) {
            bool result = ExtendedValue_eq(tests[i].a, tests[i].b);
            assert(result == tests[i].expected);
        }
    }
}
subdesc(ExtendedValue_from_BasicValue) {
    it("converts BasicValues into ExtendedValues") {
        struct {
            BasicValue basic;
            ExtendedValue expected;
        } tests[] = {
            {UNUSED, ext(0)},
            {RESET, ext(0)},
            {BLACK, ext(0)},
            {RED, ext(1)},
            {GREEN, ext(2)},
            {YELLOW, ext(3)},
            {BLUE, ext(4)},
            {MAGENTA, ext(5)},
            {CYAN, ext(6)},
            {WHITE, ext(7)},
            {LIGHTBLACK, ext(8)},
            {LIGHTRED, ext(9)},
            {LIGHTGREEN, ext(10)},
            {LIGHTYELLOW, ext(11)},
            {LIGHTBLUE, ext(12)},
            {LIGHTMAGENTA, ext(13)},
            {LIGHTCYAN, ext(14)},
            {LIGHTWHITE, ext(15)},
            // Invalid BasicValue (anything > 17 right now).
            {18, ext(0)},
            {255, ext(0)},
        };
        for_each(tests, i) {
            ExtendedValue eval = ExtendedValue_from_BasicValue(tests[i].basic);
            assert_colr_eq(eval, tests[i].expected);
        }
    }
}
subdesc(ExtendedValue_from_esc) {
    it("recognizes valid extended codes") {
        for (unsigned short eval = 0; eval < 256; eval++) {
            char* codes = fore_str_static(ext(eval));
            assert_ext_eq(
                ExtendedValue_from_esc(codes),
                eval,
                "Failed to recognize fore code."
            );
            codes = back_str_static(ext(eval));
            assert_ext_eq(
                ExtendedValue_from_esc(codes),
                eval,
                "Failed to recognize back code."
            );
        }
    }
}
subdesc(ExtendedValue_from_hex) {
    struct {
        char* hexstr;
        int expected;
    } tests[] = {
        {"#ffffff", 231},
        {"#ffffd7", 230},
        {"#878787", 102},
        {"#010101", 16},
        {"#ffffff", 231},
        {"blah", COLOR_INVALID},
        {"", COLOR_INVALID},
        {NULL, COLOR_INVALID},
    };
    for_each(tests, i) {
        int eval = ExtendedValue_from_hex(tests[i].hexstr);
        if (tests[i].expected == COLOR_INVALID) {
            assert(eval == COLOR_INVALID);
        } else {
            assert_colr_eq(ext(eval), tests[i].expected);
        }
    }
}
subdesc(ExtendedValue_from_hex_default) {
    ExtendedValue eval_default = ext(222);
    struct {
        char* hexstr;
        int expected;
    } tests[] = {
        {"#ffffff", 231},
        {"#ffffd7", 230},
        {"#878787", 102},
        {"#010101", 16},
        {"#ffffff", 231},
        {"blah", eval_default},
        {"", eval_default},
        {NULL, eval_default},
    };
    for_each(tests, i) {
        int eval = ExtendedValue_from_hex_default(tests[i].hexstr, eval_default);
        assert_colr_eq(ext(eval), tests[i].expected);
    }
}
subdesc(ExtendedValue_from_RGB) {
    struct {
        RGB rgbval;
        ExtendedValue eval;
    } tests[] = {
        {{255, 255, 255}, 231},
        {{255, 255, 215}, 230},
        {{255, 215, 215}, 224},
        {{215, 175, 175}, 181},
        {{135, 135, 135}, 102},
        {{135, 255, 215}, 122},
        {{95, 215, 175}, 79},
        {{95, 135, 135}, 66},
        {{255, 255, 255}, 231},
    };
    for_each(tests, i) {
        ExtendedValue extval = ExtendedValue_from_RGB(tests[i].rgbval);
        assert_colr_eq(extval, tests[i].eval);
    }
}
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
        it("recognizes extended color names") {
            // Test all basic names, in case of some weird regression.
            for_len(extended_names_len, i) {
                char* name = extended_names[i].name;
                int eval = extended_names[i].value;
                assert_ext_from_str_eq(
                   name,
                    eval,
                    "Known ExtendedValue didn't match"
                );
            }
        }
        it("recognizes known color names") {
            for_len(colr_name_data_len, i) {
                char* name = colr_name_data[i].name;
                ExtendedValue eval = colr_name_data[i].ext;
                assert_ext_from_str_eq(
                    name,
                    eval,
                    "Known name didn't match ExtendedValue."
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
                assert_range(
                    eval,
                    0,
                    255,
                    "ExtendedValue within range returned invalid."
                );
                assert_ext_neq(
                    eval,
                    EXT_INVALID,
                    "Known extended number caused an invalid range"
                );
                assert_ext_neq(
                    eval,
                    EXT_INVALID_RANGE,
                    "Known extended number caused an invalid range"
                );

            }
        }
        it("returns EXT_INVALID_RANGE for bad numbers") {
            // Test bad numbers.
            char* nums[] = {"-255", "-1", "256", "355"};
            for_each(nums, i) {
                int eval = ExtendedValue_from_str(nums[i]);
                assert_ext_eq(
                    eval,
                    EXT_INVALID_RANGE,
                    "Bad number was not considered outside of the range"
                );
            }
        }
        it("returns COLOR_INVALID for really bad numbers") {
            // numstr needs to hold the largest stringified number.
            char* nums[] = {"-2555", "-1000", "2560", "2147483647"};
            for_each(nums, i) {
                assert_ext_from_str_eq(
                    nums[i],
                    EXT_INVALID_RANGE,
                    "Bad number was not considered invalid"
                );
            }
            // Something like: 18446744073709551615
            unsigned long long largest_num = ULLONG_MAX;
            char largenumstr[21];
            sprintf(largenumstr, "%llu", largest_num);
            assert_ext_from_str_eq(
                largenumstr,
                EXT_INVALID_RANGE,
                "Really long number was not considered invalid"
            );
        }
    }
}
subdesc(ExtendedValue_is_invalid) {
    it("detects invalid ExtendedValues") {
        struct {
            int val;
            bool expected;
        } tests[] = {
            // Valid values.
            {0, false},
            {10, false},
            {100, false},
            {200, false},
            // Invalid values.
            {-1, true},
            {256, true},
            {1337, true},
            {2600, true},
        };
        for_each(tests, i) {
            bool result = ExtendedValue_is_invalid(tests[i].val);
            assert(result == tests[i].expected);
        }
    }
}
subdesc(ExtendedValue_is_valid) {
    it("detects valid ExtendedValues") {
        struct {
            int val;
            bool expected;
        } tests[] = {
            // Valid values.
            {0, true},
            {10, true},
            {100, true},
            {200, true},
            // Invalid values.
            {-1, false},
            {256, false},
            {1337, false},
            {2600, false},
        };
        for_each(tests, i) {
            bool result = ExtendedValue_is_valid(tests[i].val);
            assert(result == tests[i].expected);
        }
    }
}
subdesc(ExtendedValue_repr) {
    it("creates a repr from ExtendedValues") {
        char* range_repr = ExtendedValue_repr(COLOR_INVALID_RANGE);
        assert_not_null(range_repr);
        assert_str_not_empty(range_repr);
        assert_str_contains(range_repr, "COLOR_INVALID_RANGE");
        free(range_repr);
        char* invalid_repr = ExtendedValue_repr(COLOR_INVALID);
        assert_not_null(invalid_repr);
        assert_str_not_empty(invalid_repr);
        assert_str_contains(invalid_repr, "COLOR_INVALID");
        free(invalid_repr);
        ExtendedValue tests[] = {
            1,
            100,
            200
        };
        for_each(tests, i) {
            char* s = ExtendedValue_repr(tests[i]);
            assert_not_null(s);
            assert_str_not_empty(s);
            assert_str_contains(s, "ExtendedValue");
            free(s);
        }
    }
}
subdesc(ExtendedValue_to_str) {
    it("creates strings from ExtendedValues") {
        // This is literally just "%d" formatting.
        struct {
            ExtendedValue eval;
            char* expected;
        } tests[] = {
            {0, "0"},
            {1, "1"},
            {100, "100"},
            {200, "200"},
            {255, "255"},
        };
        for_each(tests, i) {
            char* s = ExtendedValue_to_str(tests[i].eval);
            assert_not_null(s);
            assert_str_not_empty(s);
            assert_str_eq(s, tests[i].expected, "Failed to produce an ExtendedValue string.");
            free(s);
        }
    }
}
} // describe(BasicValue)
