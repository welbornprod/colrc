/* Tests for the ColorArg struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

#pragma clang diagnostic ignored "-Wgnu-auto-type"

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
                assert_colr_eq(tests[i].a, tests[i].b);
            } else {
                assert_colr_neq(tests[i].a, tests[i].b);
            }
        }
    }
}
subdesc(ColorArg_example) {
    it("creates colorized examples") {
        bool colorized = true;
        ColorArg* cargs[] = {
            fore(RED),
            fore(XGREEN),
            back(rgb(0, 255, 0)),
            style(BRIGHT),
        };
        for_each(cargs, i) {
            ColorArg* carg = cargs[i];
            char* s = ColorArg_example(*carg, colorized);
            assert_not_null(s);
            assert(colr_str_has_codes(s));
            free(s);
            ColorArg_free(carg);
        }
    }
    it("creates non-colorized examples") {
        bool colorized = false;
        ColorArg* cargs[] = {
            fore(RED),
            fore(XGREEN),
            fore(rgb(0, 255, 0))
        };
        for_each(cargs, i) {
            ColorArg* carg = cargs[i];
            char* s = ColorArg_example(*carg, colorized);
            assert_not_null(s);
            assert(!colr_str_has_codes(s));
            free(s);
            ColorArg_free(carg);
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
subdesc(ColorArg_from_esc) {
    it("handles invalid esc-codes") {
        ColorArg carg = ColorArg_from_esc("\x1b[xm");
        assert_is_invalid(carg);
        assert_colr_eq_repr(carg.type, ARGTYPE_NONE, carg);
        assert_colr_eq_repr(carg.value.type, TYPE_INVALID, carg);
    }
    it("creates ColorArgs from basic esc-codes") {
        for_len(basic_names_len, i) {
            BasicValue bval = basic_names[i].value;
            char* codes = fore_str_static(bval);
            ColorArg carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, FORE, codes);
            assert_colr_eq_repr(carg.value.basic, bval, codes);

            codes = back_str_static(bval);
            carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, BACK, codes);
            assert_colr_eq_repr(carg.value.basic, bval, codes);
        }
    }
    it("creates ColorArgs from ext esc-codes") {
        for (unsigned short i = 0; i < 256; i++) {
            ExtendedValue eval = i;
            char* codes = fore_str_static(eval);
            ColorArg carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, FORE, codes);
            assert_colr_eq_repr(carg.value.ext, eval, codes);

            codes = back_str_static(eval);
            carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, BACK, codes);
            assert_colr_eq_repr(carg.value.ext, eval, codes);
        }
    }
    it("creates ColorArgs from style esc-codes") {
        for_len(style_names_len, i) {
            StyleValue sval = style_names[i].value;
            char codes[STYLE_LEN];
            format_style(codes, sval);
            ColorArg carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, STYLE, codes);
            assert_colr_eq_repr(carg.value.style, sval, codes);
        }
    }
    it("creates ColorArgs from RGB esc-codes") {
        for_len(colr_name_data_len, i) {
            RGB expected = colr_name_data[i].rgb;
            char* codes = fore_str_static(expected);
            ColorArg carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, FORE, codes);
            assert_colr_eq_repr(carg.value.rgb, expected, codes);

            codes = back_str_static(expected);
            carg = ColorArg_from_esc(codes);
            assert_is_valid(carg);
            assert_colr_eq_repr(carg.type, BACK, codes);
            assert_colr_eq_repr(carg.value.rgb, expected, codes);
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
            assert_colr_eq(
                ColorArg_from_str(tests[i].type, tests[i].name),
                tests[i].expected
            );
        }
    }
    it("handles mismatched names") {
        // Passing a valid color name as a style, or a style name as a color
        // should set the appropriate "TYPE_INVALID" types.
        struct {
            ArgType type;
            char* name;
            ColorType expected;
        } tests[] = {
            {FORE, "bright", TYPE_INVALID},
            {FORE, "underline", TYPE_INVALID},
            {BACK, "bright", TYPE_INVALID},
            {BACK, "underline", TYPE_INVALID},
            {STYLE, "white", TYPE_INVALID_STYLE},
            {STYLE, "255;255;255", TYPE_INVALID_STYLE},
        };
        for_each(tests, i) {
            ColorArg carg = ColorArg_from_str(tests[i].type, tests[i].name);
            assert_colr_eq(
                carg.value.type,
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
    it("handles NULL") {
        ColorArg carg = ColorArg_from_value(FORE, TYPE_STYLE, NULL);
        assert_is_invalid(carg);
    }
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
            assert_colr_eq(carg, tests[i].expected);
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
subdesc(ColorArg_to_esc) {
    it("creates escape codes") {
        struct {
            ColorArg carg;
            char* expected;
        } tests[] = {
            {ColorArg_from_str(FORE, "NOTACOLOR"), NULL},
            {fore_arg(WHITE), "\x1b[37m"},
            {fore_arg(XWHITE), "\x1b[38;5;7m"},
            {style_arg(BRIGHT), "\x1b[1m"},
            {back_arg(rgb(1, 1, 1)), "\x1b[48;2;1;1;1m"},
        };
        for_each(tests, i) {
            char* s = ColorArg_to_esc(tests[i].carg);
            if (!tests[i].expected) {
                assert_null(s);
                continue;
            } else {
                assert(colr_str_is_codes(s));
            }
            assert_str_eq(s, tests[i].expected, "Failed to fill with escape-code.");
            free(s);
        }
    }
}
subdesc(ColorArg_to_esc_s) {
    it("handles empty ColorArgs") {
        ColorArg carg = ColorArg_empty();
        char empty[CODE_ANY_LEN];
        assert(!ColorArg_to_esc_s(empty, carg));
        assert_str_empty(empty);
    }
    it("fills with escape codes") {
        struct {
            ColorArg carg;
            char* expected;
        } tests[] = {
            {ColorArg_from_str(FORE, "NOTACOLOR"), ""},
            {fore_arg(WHITE), "\x1b[37m"},
            {fore_arg(XWHITE), "\x1b[38;5;7m"},
            {style_arg(BRIGHT), "\x1b[1m"},
            {back_arg(rgb(1, 1, 1)), "\x1b[48;2;1;1;1m"},
        };
        for_each(tests, i) {
            size_t expected_len = ColorArg_length(tests[i].carg);
            // Never returns 0, but we'll make sure that hasn't changed.
            assert(expected_len);
            char dest[expected_len];
            bool success = ColorArg_to_esc_s(dest, tests[i].carg);
            if (tests[i].expected[0] == '\0') {
                assert(!success);
                assert_str_empty(dest);
            } else {
                assert(success);
                assert(colr_str_is_codes(dest));
            }
            assert_str_eq(dest, tests[i].expected, "Failed to fill with escape-code.");
        }
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
} // describe(ColorArg)

describe(ColorArgs) {
subdesc(ColorArgs_array_free) {
    it("frees ColorArg lists") {
        // The real test is when is sent through valgrind.
        ColorArg** lst = NULL;
        ColorArgs_array_fill(
            lst,
            fore(RED),
            back(WHITE),
            style(UNDERLINE)
        );
        ColorArgs_array_free(lst);
    }
}
subdesc(ColorArgs_array_repr) {
    it("handles NULL") {
        char* nullrepr = ColorArgs_array_repr(NULL);
        assert_str_eq(nullrepr, "NULL", "failed on NULL");
        free(nullrepr);
        ColorArg** emptylist = {NULL};
        char* emptyrepr = ColorArgs_array_repr(emptylist);
        assert_str_eq(emptyrepr, "NULL", "failed on empty list");
        free(emptyrepr);
    }
    it("creates a ColorArgs array repr") {
        ColorArg** lst = NULL;
        ColorArgs_array_fill(
            lst,
            fore(RED),
            back(WHITE)
        );
        char* repr = ColorArgs_array_repr(lst);
        assert_not_null(repr);
        assert_str_contains(repr, "RED");
        assert_str_contains(repr, "WHITE");
        assert_str_contains(repr, "NULL");
        free(repr);
        ColorArgs_array_free(lst);
    }
}
subdesc(ColorArgs_from_str) {
    it("handles NULL") {
        bool do_unique = false;
        assert_call_null(ColorArgs_from_str, NULL, do_unique);
        assert_call_null(ColorArgs_from_str, "", do_unique);
        assert_call_null(ColorArgs_from_str, "No codes in here.", do_unique);
    }
    it("handle escape codes") {
        char* escstr = "\x1b[4m\x1b[31m\x1b[31m\x1b[47m\x1b[47mtest\x1b[0m";
        // Do non-unique codes.
        ColorArg** cargs = ColorArgs_from_str(escstr, false);
        ColorArg expected[] = {
            fore_arg(RED),
            back_arg(WHITE),
            style_arg(UNDERLINE),
            style_arg(RESET_ALL)
        };
        for_each(expected, i) {
            assert_ColorArgs_array_contains(cargs, expected[i]);
        }
        assert_size_eq_repr(ColorArgs_array_len(cargs), 6, cargs);
        ColorArgs_array_free(cargs);

        // Do unique codes.
        cargs = ColorArgs_from_str(escstr, true);
        for_each(expected, i) {
            assert_ColorArgs_array_contains(cargs, expected[i]);
        }
        assert_size_eq_repr(ColorArgs_array_len(cargs), 4, cargs);
        ColorArgs_array_free(cargs);
    }
}
} // describe(ColorArgs)
