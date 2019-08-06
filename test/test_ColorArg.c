/* Tests for the ColorArg struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

describe(ColorArg) {
subdesc(ColorArg_empty) {
    it("creates an initialized ColorArg") {
        ColorArg carg = ColorArg_empty();
        assert(ColorArg_is_ptr(&carg));
    }
}
subdesc(ColorArg_eq) {
    it("compares ColorArgs correctly") {
        struct {
            ColorArg a;
            ColorArg b;
            bool expected;
        } tests[] = {
            {fore_arg(WHITE), fore_arg(WHITE), true},
            {fore_arg(XWHITE), fore_arg(XWHITE), true},
            {fore_arg(rgb(255, 255, 255)), fore_arg(rgb(255, 255, 255)), true},
            {back_arg(WHITE), back_arg(WHITE), true},
            {back_arg(XWHITE), back_arg(XWHITE), true},
            {back_arg(rgb(255, 255, 255)), back_arg(rgb(255, 255, 255)), true},
            {style_arg(BRIGHT), style_arg(BRIGHT), true},
            {fore_arg(WHITE), fore_arg(RED), false},
            {fore_arg(XWHITE), fore_arg(XRED), false},
            {fore_arg(rgb(255, 255, 255)), fore_arg(rgb(0, 0, 0)), false},
            {back_arg(WHITE), back_arg(RED), false},
            {back_arg(XWHITE), back_arg(XRED), false},
            {back_arg(rgb(255, 255, 255)), back_arg(rgb(0, 0, 0)), false},
            {style_arg(BRIGHT), style_arg(UNDERLINE), false},
        };
        for_each(tests, i) {
            if (tests[i].expected) {
                assert_ColorArg_eq(tests[i].a, tests[i].b);
            } else {
                assert_ColorArg_neq(tests[i].a, tests[i].b);
            }
        }
    }
}
subdesc(ColorArg_free) {
    it("frees it's resources") {
        ColorArg* carg = ColorArg_to_ptr(ColorArg_empty());
        ColorArg_free(carg);
    }
}
subdesc(ColorArg_from_BasicValue) {
    it("creates BasicValue args") {
        struct {
            ArgType type;
            BasicValue bval;
        } tests[] = {
            {FORE, WHITE},
            {BACK, RED},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_BasicValue(tests[i].type, tests[i].bval);
            assert(carg.type == tests[i].type);
            assert(carg.value.basic == tests[i].bval);
        }
    }
}
subdesc(ColorArg_from_ExtendedValue) {
    it("creates ExtendedValue args") {
        struct {
            ArgType type;
            ExtendedValue eval;
        } tests[] = {
            {FORE, XWHITE},
            {BACK, XRED},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_ExtendedValue(tests[i].type, tests[i].eval);
            assert(carg.type == tests[i].type);
            assert(carg.value.ext == tests[i].eval);
        }
    }
}
subdesc(ColorArg_from_RGB) {
    it("creates RGB args") {
        struct {
            ArgType type;
            RGB val;
        } tests[] = {
            {FORE, rgb(255, 255, 255)},
            {BACK, rgb(255, 0, 0)},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_RGB(tests[i].type, tests[i].val);
            assert(carg.type == tests[i].type);
            assert(RGB_eq(carg.value.rgb, tests[i].val));
        }
    }
}
subdesc(ColorArg_from_str) {
    it("creates args from names") {
        struct {
            ArgType type;
            char* name;
            ColorArg expected;
        } tests[] = {
            {FORE, "white", fore_arg(WHITE)},
            {FORE, "xwhite", fore_arg(XWHITE)},
            {FORE, "255;255;255", fore_arg(rgb(255, 255, 255))},
            {BACK, "white", back_arg(WHITE)},
            {BACK, "xwhite", back_arg(XWHITE)},
            {BACK, "255;255;255", back_arg(rgb(255, 255, 255))},
            {STYLE, "bright", style_arg(BRIGHT)},
            {STYLE, "underline", style_arg(UNDERLINE)},
            {STYLE, "reset_all", style_arg(RESET_ALL)},
        };
        for_each(tests, i) {
            assert_ColorArg_eq(
                ColorArg_from_str(tests[i].type, tests[i].name),
                tests[i].expected
            );
        }
    }
}
subdesc(ColorArg_from_StyleValue) {
    it("creates StyleValue args") {
        struct {
            ArgType type;
            StyleValue sval;
        } tests[] = {
            {STYLE, UNDERLINE},
            {STYLE, BRIGHT},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_StyleValue(tests[i].type, tests[i].sval);
            assert(carg.type == tests[i].type);
            assert(carg.value.style == tests[i].sval);
        }
    }
}
subdesc(ColorArg_from_value) {
    it("creates args from void pointers") {
        BasicValue basic = WHITE;
        ExtendedValue extended = XWHITE;
        StyleValue style = UNDERLINE;
        RGB rgbval = rgb(255, 0, 0);

        struct {
            ArgType argtype;
            ColorType type;
            void* p;
            ColorArg expected;
        } tests[] = {
            {FORE, TYPE_BASIC, &basic, fore_arg(WHITE)},
            {FORE, TYPE_EXTENDED, &extended, fore_arg(XWHITE)},
            {FORE, TYPE_RGB, &rgbval, fore_arg(rgb(255, 0, 0))},
            {STYLE, TYPE_STYLE, &style, style_arg(UNDERLINE)},
            {BACK, TYPE_BASIC, &basic, back_arg(WHITE)},
            {BACK, TYPE_EXTENDED, &extended, back_arg(XWHITE)},
            {BACK, TYPE_RGB, &rgbval, back_arg(rgb(255, 0, 0))},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_value(
                tests[i].argtype,
                tests[i].type,
                tests[i].p
            );
            assert_ColorArg_eq(carg, tests[i].expected);
        }
    }
}
subdesc(ColorArg_is_empty) {
    it("detects empty ColorArgs") {
        assert(ColorArg_is_empty(ColorArg_empty()));
        assert(!ColorArg_is_empty(fore_arg(RESET)));
    }
}
subdesc(ColorArg_is_invalid) {
    it("detects invalid ColorArgs") {
        struct {
            ArgType type;
            char* name;
            bool expected;
        } tests[] = {
            {FORE, "NOTACOLOR", true},
            {FORE, "white", false},
            {FORE, "xwhite", false},
            {FORE, "255;255;255", false},
            {FORE, "1337", true},
            {FORE, "355;255;0", true},
            {STYLE, "underline", false},
            {STYLE, "NOTASTYLE", true},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_str(tests[i].type, tests[i].name);
             if (tests[i].expected) {
                assert_is_invalid(carg);
            } else {
                assert_is_valid(carg);
            }
        }
    }
}
subdesc(ColorArg_is_ptr) {
    it("detects ColorArg pointers") {
        ColorArg carg = ColorArg_empty();
        ColorText ctext = ColorText_empty();
        char* s = "test";
        struct {
            void* p;
            bool expected;
        } tests[] = {
            {&carg, true},
            {&ctext, false},
            {s, false},
        };
        for_each(tests, i) {
            asserteq(ColorArg_is_ptr(tests[i].p), tests[i].expected);
        }
    }
}
subdesc(ColorArg_is_valid) {
    it("detects valid ColorArgs") {
        struct {
            ArgType type;
            char* name;
            bool expected;
        } tests[] = {
            {FORE, "NOTACOLOR", false},
            {FORE, "white", true},
            {FORE, "xwhite", true},
            {FORE, "255;255;255", true},
            {FORE, "1337", false},
            {FORE, "355;255;0", false},
            {STYLE, "underline", true},
            {STYLE, "NOTASTYLE", false},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_str(tests[i].type, tests[i].name);
            if (tests[i].expected) {
                assert_is_valid(carg);
            } else {
                assert_is_invalid(carg);
            }
        }
    }
}
subdesc(ColorArg_length) {
    it("calculates the length of a ColorArg") {
        struct {
            ColorArg carg;
            size_t expected_min;
            size_t expected_max;
        } tests[] = {
            {fore_arg(WHITE), CODE_LEN_MIN, CODE_LEN},
            {fore_arg(XWHITE), CODEX_LEN_MIN, CODEX_LEN},
            {fore_arg(rgb(255, 255, 255)), CODE_RGB_LEN_MIN, CODE_RGB_LEN},
        };
        for_each(tests, i) {
            size_t length = ColorArg_length(tests[i].carg);
            assert_range(
                length,
                tests[i].expected_min,
                tests[i].expected_max,
                "length is outside of the expected range."
            );
        }
    }
}
subdesc(ColorArg_repr) {
    it("creates a ColorArg repr") {
        char* s = ColorArg_repr(fore_arg(WHITE));
        free(s);
    }
}
subdesc(ColorArg_to_ptr) {
    it("allocates a ColorArg") {
        ColorArg* carg = ColorArg_to_ptr(fore_arg(WHITE));
        assert(ColorArg_is_valid(*carg));
        assert(ColorArg_is_ptr(carg));
        free(carg);
    }
}
subdesc(ColorArg_to_str) {
    it("creates escape codes") {
        struct {
            ColorArg carg;
            bool invalid;
        } tests[] = {
            {fore_arg(WHITE), false},
            {fore_arg("NOTACOLOR"), true},
        };
        for_each(tests, i) {
            char* s = ColorArg_to_str(tests[i].carg);
            if (tests[i].invalid) {
                assert_str_empty(s);
            } else {
                assert(str_is_codes(s));
            }
            free(s);
        }
    }
}
}
