/* Tests for the ColorText struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

describe(ColorText) {
subdesc(ColorText_empty) {
    it("creates an empty ColorText") {
        ColorText empty = ColorText_empty();
        assert(ColorText_is_empty(empty));
        char* s = ColorText_to_str(empty);
        assert_str_empty(s);
        free(s);
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
        assert_ColorText_nothas_arg(ctext, forearg);
        assert_ColorText_nothas_arg(ctext, backarg);
        assert_ColorText_nothas_arg(ctext, stylearg);
        ctext = ColorText_from_values("test", &forearg, &backarg, &stylearg, _ColrLastArg);
        assert_ColorText_has_arg(ctext, forearg);
        assert_ColorText_has_arg(ctext, backarg);
        assert_ColorText_has_arg(ctext, stylearg);
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
                ColorText_from_values("test", _ColrLastArg),
                slength
            },
            {
                ColorText_from_values("test", &forearg, _ColrLastArg),
                slength + CODE_LEN + CODE_RESET_LEN
            },
            {
                ColorText_from_values("test", &backarg, _ColrLastArg),
                slength + CODE_LEN + CODE_RESET_LEN
            },
            {
                // The RESET_ALL style means no extra CODE_RESET_ALL is appended.
                ColorText_from_values("test", &stylearg, _ColrLastArg),
                slength + STYLE_LEN
            },
            {
                ColorText_from_values("test", &forearg, &backarg, _ColrLastArg),
                slength + (CODE_LEN * 2) + CODE_RESET_LEN
            },
            {
                // Again, the RESET_ALL causes no extra CODE_RESET_ALL.
                ColorText_from_values("test", &forearg, &backarg, &stylearg, _ColrLastArg),
                slength + (CODE_LEN * 2) + STYLE_LEN
            },
        };
        for_each(tests, i) {
            size_t length = ColorText_length(tests[i].ctext);
            assert_size_eq_full(length, tests[i].expected, tests[i].ctext);
        }
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
            free(tests[i].arg1);
            free(tests[i].arg2);
            free(tests[i].arg3);
        }
    }
}
subdesc(ColorText_to_ptr) {
    it("allocates a ColorText") {
        ColorText* ctext = Colr("test", fore(RED));
        assert(ColorText_is_ptr(ctext));
        ColorText_free(ctext);
    }
}
subdesc(ColorText_to_str) {
    it("creates colorized strings") {
        ColorText empty = ColorText_empty();
        char* emptystr = ColorText_to_str(empty);
        assert_str_empty(emptystr);
        free(emptystr);

        ColorText ctext = ColorText_from_values("test", _ColrLastArg);
        char* nocodes = ColorText_to_str(ctext);
        assert_str_eq(nocodes, "test", "Should be a simple string!");
        free(nocodes);

        ColorText* p = Colr("test", fore(RED));
        char* codes = ColorText_to_str(*p);
        assert(str_has_codes(codes));
        free(codes);
        ColorText_free(p);
    }
}
} // describe(ColorText)
