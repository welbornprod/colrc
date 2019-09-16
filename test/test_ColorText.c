/* Tests for the ColorText struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

#define empty_colorarg() ColorArg_to_ptr(ColorArg_empty())

describe(ColorText) {
subdesc(ColorText_empty) {
    it("creates an empty ColorText") {
        ColorText empty = ColorText_empty();
        assert(ColorText_is_empty(empty));
        char* s = ColorText_to_str(empty);
        assert_null(s);
    }
}
subdesc(ColorText_free) {
    it("frees it's resources") {
        ColorText* p = Colr("XXX", fore(RED), back(XWHITE), style(UNDERLINE));
        ColorText_free(p);
    }
}
subdesc(ColorText_from_values) {
    it("creates ColorTexts from void pointers") {
        struct {
            void* arg1;
            void* arg2;
            void* arg3;
        } tests[] = {
            {fore(RED), back(XWHITE), style(BRIGHT)},
            {back(RED), fore(XWHITE), style(BRIGHT)},
            {fore(RED), style(BRIGHT), back(XWHITE)},
            {empty_colorarg(), back(XWHITE), style(BRIGHT)},
            {fore(XWHITE), empty_colorarg(), style(BRIGHT)},
            {fore(RED), back(XWHITE), empty_colorarg()},
            {style(UNDERLINE), back(rgb(0, 0, 0)), fore(RED)},
        };
        for_each(tests, i) {
            ColorText* p = Colr("XXX", tests[i].arg1, tests[i].arg2, tests[i].arg3);
            assert(ColorText_is_ptr(p));
            char* s = ColorText_to_str(*p);
            ColorText_free(p);
            assert_str_not_empty(s);
            free(s);
        }
    }
}
subdesc(ColorText_has_arg) {
    it("detects ColorArg members") {
        ColorArg forearg = fore_arg(rgb(255, 35, 45));
        ColorArg backarg = fore_arg(rgb(255, 35, 45));
        ColorArg stylearg = fore_arg(rgb(255, 35, 45));
        ColorText ctext = ColorText_empty();
        assert_ColorText_missing_arg(ctext, forearg);
        assert_ColorText_missing_arg(ctext, backarg);
        assert_ColorText_missing_arg(ctext, stylearg);
        ctext = Colra("test", &forearg, &backarg, &stylearg);
        assert_ColorText_has_arg(ctext, forearg);
        assert_ColorText_has_arg(ctext, backarg);
        assert_ColorText_has_arg(ctext, stylearg);
    }
}
subdesc(ColorText_has_args) {
    it("detects usable args") {
        ColorText* ctexts[] = {
            Colr("test", fore(RED)),
            Colr("test", back(WHITE)),
            Colr("test", style(UNDERLINE)),
        };
        for_each(ctexts, i) {
            assert(ColorText_has_args(*ctexts[i]));
            ColorText_free(ctexts[i]);
        }
        ColorText empty = ColorText_empty();
        assert(!ColorText_has_args(empty));
    }
}
subdesc(ColorText_is_empty) {
    it("detects empty ColorTexts") {
        ColorText empty = ColorText_empty();
        assert(ColorText_is_empty(empty));
        ColorText* ctext = Colr("XXX", fore(RED));
        assert(!ColorText_is_empty(*ctext));
        ColorText_free(ctext);
    }
}
subdesc(ColorText_is_ptr) {
    it("detects ColorText pointers") {
        ColorArg carg = ColorArg_empty();
        ColorText ctext = ColorText_empty();
        char* s = "test";
        struct {
            void* p;
            bool expected;
        } tests[] = {
            {&carg, false},
            {&ctext, true},
            {s, false},
        };
        for_each(tests, i) {
            asserteq(ColorText_is_ptr(tests[i].p), tests[i].expected);
        }
    }
}
subdesc(ColorText_length) {
    it("calculates the length of a ColorText") {
        ColorArg forearg = fore_arg(WHITE);
        ColorArg backarg = back_arg(RED);
        ColorArg stylearg = style_arg(RESET_ALL);
        char* teststr = "test";
        size_t slength = strlen(teststr) + 1;
        struct {
            ColorText ctext;
            size_t expected;
        } tests[] = {
            {
                Colra("test", NULL),
                slength
            },
            {
                Colra("test", &forearg),
                slength + CODE_LEN + CODE_RESET_LEN
            },
            {
                Colra("test", &backarg),
                slength + CODE_LEN + CODE_RESET_LEN
            },
            {
                // The RESET_ALL style comes before "test", so it is appended
                // again.
                Colra("test", &stylearg),
                slength + STYLE_LEN + CODE_RESET_LEN
            },
            {
                Colra("test", &forearg, &backarg),
                slength + (CODE_LEN * 2) + CODE_RESET_LEN
            },
            {
                // Again, the RESET_ALL comes before "test", so it is appended.
                Colra("test", &forearg, &backarg, &stylearg),
                slength + (CODE_LEN * 2) + STYLE_LEN + CODE_RESET_LEN
            },
        };
        for_each(tests, i) {
            size_t length = ColorText_length(tests[i].ctext);
            assert_size_eq_full(length, tests[i].expected, tests[i].ctext);
        }

        // TODO: More justification tests.
        struct {
            ColorText* ctextp;
            size_t expected;
        } just_tests[] = {
            {Colr_ljust("test", 10, NULL), 11},
            {Colr_rjust("test", 10, NULL), 11},
            {Colr_center("test", 10, NULL), 11},
        };

        for_each(just_tests, i) {
            size_t length = ColorText_length(*(just_tests[i].ctextp));
            assert_size_eq_full(length, just_tests[i].expected, *(just_tests[i].ctextp));
            ColorText_free(just_tests[i].ctextp);
        }
        ColorText* ctextp = Colr_center(teststr, 0, NULL);
        size_t length = ColorText_length(*ctextp);
        char* s = ColorText_to_str(*ctextp);
        assert(colr_str_starts_with(s, "  "));
        assert(colr_str_ends_with(s, "  "));
        assert_size_gt_full(length, 5, *ctextp);
        ColorText_free(ctextp);
        free(s);
    }
}
subdesc(ColorText_repr) {
    it("creates a ColorText repr") {
        char* s = ColorText_repr(ColorText_empty());
        free(s);
    }
}
subdesc(ColorText_set_just) {
    it("sets the justification method") {
        char* teststr = "test";
        ColorText ctext = Colra(teststr, NULL);
        struct {
            ColorJustify just;
            char* expected;
        } tests[] = {
            {ColorJustify_new(JUST_LEFT, 8, ' '), "test    "},
            {ColorJustify_new(JUST_RIGHT, 8, ' '), "    test"},
            {ColorJustify_new(JUST_CENTER, 8, ' '), "  test  "},
        };
        for_each(tests, i) {
            ColorText_set_just(&ctext, tests[i].just);
            char* resultstr = colr_to_str(ctext);
            assert_str_eq_repr(resultstr, tests[i].expected, ctext);
            free(resultstr);
        }
    }
}
subdesc(ColorText_set_values) {
    it("sets members from void pointers") {
        struct {
            void* arg1;
            void* arg2;
            void* arg3;
        } tests[] = {
            {fore(RED), back(XWHITE), style(BRIGHT)},
            {back(RED), fore(XWHITE), style(BRIGHT)},
            {fore(RED), style(BRIGHT), back(XWHITE)},
            {style(UNDERLINE), back(rgb(0, 0, 0)), fore(RED)},
            {empty_colorarg(), back(WHITE), style(UNDERLINE)},
            {fore(RED), empty_colorarg(), style(UNDERLINE)},
            {fore(RED), back(WHITE), empty_colorarg()},
            {empty_colorarg(), empty_colorarg(), empty_colorarg()}
        };
        for_each(tests, i) {
            ColorText ctext = ColorText_empty();
            ColorText_set_values(&ctext, "XXX", tests[i].arg1, tests[i].arg2, tests[i].arg3, _ColrLastArg);
            ColorArg* arg1 = tests[i].arg1;
            assert_ColorText_has_arg(ctext, *arg1);
            ColorArg* arg2 = tests[i].arg2;
            assert_ColorText_has_arg(ctext, *arg2);
            ColorArg* arg3 = tests[i].arg3;
            assert_ColorText_has_arg(ctext, *arg3);
            colr_free(tests[i].arg1);
            colr_free(tests[i].arg2);
            colr_free(tests[i].arg3);
        }

    }
}
subdesc(ColorText_to_ptr) {
    it("allocates a ColorText") {
        ColorText ctext = Colra("test", fore(RED));
        ColorText* ctextp = ColorText_to_ptr(ctext);
        assert(ColorText_is_ptr(ctextp));
        ColorText_free(ctextp);
    }
}
subdesc(ColorText_to_str) {
    it("creates colorized strings") {
        ColorText empty = ColorText_empty();
        char* emptystr = ColorText_to_str(empty);
        assert_null(emptystr);

        ColorText ctext = Colra("test", NULL);
        char* nocodes = ColorText_to_str(ctext);
        assert_str_eq(nocodes, "test", "Should be a simple string!");
        free(nocodes);

        ColorText* p = Colr("test", fore(RED));
        char* codes = ColorText_to_str(*p);
        assert(colr_str_has_codes(codes));
        free(codes);
        ColorText_free(p);
    }
    it("handles basic justification") {
        // TODO: This is a mess, but it's only here for a while for test coverage.
        // It *is* testing for correct justification, and covering all the
        // code paths, but there is a better way to do it. I'll have to come
        // back to it.
        struct {
            bool (*test_func)(const char* s, const char* pat);
            bool (*test_func2)(const char* s, const char* pat);
            char* pattern;
            ColorText* ctextp;
        } just_tests[] = {
            {colr_str_ends_with, NULL, "      ", Colr_ljust("test", 10, fore(RED))},
            {colr_str_starts_with, NULL, "      ", Colr_rjust("test", 10, fore(RED))},
            {colr_str_starts_with, colr_str_ends_with, "   ", Colr_center("test", 10, fore(RED))},
        };
        for_each(just_tests, i) {
            char* s = ColorText_to_str(*(just_tests[i].ctextp));
            // Make sure the strings starts/ends with the correct pattern/string.
            assert(just_tests[i].test_func(s, just_tests[i].pattern));
            if (just_tests[i].test_func2) {
                // Test the second str_ function against the result.
                assert(just_tests[i].test_func2(s, "  "));
            }
            free(s);
            ColorText_free(just_tests[i].ctextp);
        }
    }
}
} // describe(ColorText)
