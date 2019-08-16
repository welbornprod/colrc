/* Tests for the ColorValue struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/
#include "test_ColorValue.h"

describe(ColorValue) {
subdesc(ColorValue_empty) {
    it("creates an empty ColorValue") {
        ColorValue empty = ColorValue_empty();
        assert(ColorValue_is_empty(empty));
    }
}
subdesc(ColorValue_from_esc) {
    it("creates ColorValues from basic esc-codes") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            char codes[CODE_LEN];
            format_fg(codes, bval);
            ColorValue cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, bval);

            format_bg(codes, bval);
            cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, bval);
        }
    }
    it("creates ColorValues from ext esc-codes") {
        for (unsigned short i = 0; i < 256; i++) {
            ExtendedValue eval = i;
            char codes[CODEX_LEN];
            format_fgx(codes, eval);
            ColorValue cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, eval);

            format_bgx(codes, eval);
            cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, eval);
        }
    }
    it("creates ColorValues from style esc-codes") {
        for_len(style_names_len, i) {
            StyleValue sval = style_names[i].value;
            char codes[STYLE_LEN];
            format_style(codes, sval);
            ColorValue cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, sval);
        }
    }
    it("creates ColorValues from RGB esc-codes") {
        for_len(colr_name_data_len, i) {
            RGB expected = colr_name_data[i].rgb;
            char codes[CODE_RGB_LEN];
            format_fg_RGB(codes, expected);
            ColorValue cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, expected);

            format_bg_RGB(codes, expected);
            cval = ColorValue_from_esc(codes);
            assert_is_valid(cval);
            assert_ColorValue_has(cval, expected);
        }
    }
}

subdesc(ColorValue_from_str) {
    it("creates ColorValues from color names") {
        ColorValue basic = ColorValue_from_str("white");
        assert(ColorValue_has_BasicValue(basic, WHITE));
        ColorValue extended = ColorValue_from_str("xwhite");
        assert(ColorValue_has_ExtendedValue(extended, XWHITE));
        ColorValue rgbval = ColorValue_from_str("255;255;255");
        assert(ColorValue_has_RGB(rgbval, rgb(255, 255, 255)));
        ColorValue style = ColorValue_from_str("underline");
        assert(ColorValue_has_StyleValue(style, UNDERLINE));
    }
    it("detects invalid types") {
        struct {
            char* s;
            ColorType type;
        } tests[] = {
            {NULL, TYPE_INVALID},
            {"", TYPE_INVALID},
            {"none", TYPE_BASIC},
            {"32", TYPE_EXTENDED},
            {"32;64;86", TYPE_RGB},
            {"3009", TYPE_INVALID_EXT_RANGE},
            {"355;355;355", TYPE_INVALID_RGB_RANGE},
        };
        for_each(tests, i) {
            assert_colorval_from_str_eq_type(tests[i].s, tests[i].type);
        }
    }
}
subdesc(ColorValue_from_value) {
    it("creates ColorValues from void pointers") {
        ColorValue cval = ColorValue_from_value(TYPE_BASIC, NULL);
        assert(ColorValue_is_invalid(cval));

        BasicValue basic = WHITE;
        cval = ColorValue_from_value(TYPE_BASIC, &basic);
        assert(ColorValue_has_BasicValue(cval, basic));

        ExtendedValue extended = XWHITE;
        cval = ColorValue_from_value(TYPE_EXTENDED, &extended);
        assert(ColorValue_has_ExtendedValue(cval, extended));

        StyleValue style = UNDERLINE;
        cval = ColorValue_from_value(TYPE_STYLE, &style);
        assert(ColorValue_has_StyleValue(cval, style));

        RGB rgbval = rgb(255, 255, 255);
        cval = ColorValue_from_value(TYPE_RGB, &rgbval);
        assert(ColorValue_has_RGB(cval, rgbval));

        // TODO: coverage for StyleValue outside of STYLE_MIN/MAX_VALUE
        // TODO: coverage for TYPE_NONE (the fallback return).
    }
}
subdesc(ColorValue_has_BasicValue) {
    it("detects BasicValue values") {
        ColorValue hasit = ColorValue_from_str("white");
        assert(ColorValue_has_BasicValue(hasit, WHITE));
        assert(!ColorValue_has_BasicValue(hasit, RED));
    }
}
subdesc(ColorValue_has_ExtendedValue) {
    it("detects ExtendedValue values") {
        ColorValue hasit = ColorValue_from_str("xwhite");
        assert(ColorValue_has_ExtendedValue(hasit, XWHITE));
        assert(!ColorValue_has_ExtendedValue(hasit, XRED));
    }
}
subdesc(ColorValue_has_StyleValue) {
    it("detects StyleValue values") {
        ColorValue hasit = ColorValue_from_str("bright");
        assert(ColorValue_has_StyleValue(hasit, BRIGHT));
        assert(!ColorValue_has_StyleValue(hasit, UNDERLINE));
    }
}
subdesc(ColorValue_has_RGB) {
    it("detects RGB values") {
        ColorValue hasit = ColorValue_from_str("255;255;255");
        assert(ColorValue_has_RGB(hasit, rgb(255, 255, 255)));
        assert(!ColorValue_has_RGB(hasit, rgb(2, 2, 2)));
    }
}
subdesc(ColorValue_is_empty) {
    it("detects empty ColorValues") {
        ColorValue empty = ColorValue_empty();
        assert(ColorValue_is_empty(empty));
        ColorValue cval = ColorValue_from_str("#ffffff");
        assert(!ColorValue_is_empty(cval));
    }
}
subdesc(ColorValue_is_invalid) {
    it("detects invalid ColorValues") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            {"NOTACOLOR", true},
            {"white", false},
            {"xwhite", false},
            {"255;255;255", false},
            {"#aabbcc", false},
        };
        for_each(tests, i) {
            ColorValue cval = ColorValue_from_str(tests[i].name);
            if (tests[i].expected) {
                assert_is_invalid(cval);
            } else {
                assert_is_valid(cval);
            }
        }
    }
}
subdesc(ColorValue_is_valid) {
    it("detects valid ColorValues") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            {"NOTACOLOR", false},
            {"white", true},
            {"xwhite", true},
            {"255;255;255", true},
            {"#aabbcc", true},
        };
        for_each(tests, i) {
            ColorValue cval = ColorValue_from_str(tests[i].name);
            if (tests[i].expected) {
                assert_is_valid(cval);
            } else {
                assert_is_invalid(cval);
            }
        }
    }
}
subdesc(ColorValue_length) {
    it("calculates the length of a ColorValue") {
        BasicValue basic = WHITE;
        ExtendedValue extended = XWHITE;
        StyleValue style = RESET_ALL;
        RGB rgbval = rgb(255, 255, 255);
        struct {
            ArgType type;
            ColorValue cval;
            size_t expected;
        } tests[] = {
            {FORE, color_val(basic), CODE_LEN},
            {FORE, color_val(extended), CODEX_LEN},
            {STYLE, color_val(style), STYLE_LEN},
            {FORE, color_val(rgbval), CODE_RGB_LEN},
        };
        for_each(tests, i) {
            size_t length = ColorValue_length(tests[i].type, tests[i].cval);
            assert_size_eq(length, tests[i].expected);
        }
    }
}
subdesc(ColorValue_repr) {
    it("creates a ColorValue repr") {
        char* s = ColorValue_repr(ColorValue_from_str("white"));
        free(s);
    }
}
subdesc(ColorValue_to_str) {
    it("creates escape codes from ColorValues") {
        BasicValue basic = WHITE;
        ExtendedValue extended = XWHITE;
        StyleValue style = BRIGHT;
        RGB rgbval = rgb(1, 1, 1);
        struct {
            ArgType type;
            ColorValue cval;
            char* expected;
        } tests[] = {
            {FORE, ColorValue_from_str("NOTACOLOR"), ""},
            {FORE, color_val(basic), "\x1b[37m"},
            {FORE, color_val(extended), "\x1b[38;5;7m"},
            {STYLE, color_val(style), "\x1b[1m"},
            {FORE, color_val(rgbval), "\x1b[38;2;1;1;1m"},
        };
        for_each(tests, i) {
            char* s = ColorValue_to_str(tests[i].type, tests[i].cval);
            assert_str_eq(s, tests[i].expected, "Failed to create escape-code.");
            free(s);
        }
    }
}
}
