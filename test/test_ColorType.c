/*! Tests for the ColorType enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

#define assert_ColorType_name_equal(name, ct2) \
    do { \
        ColorType a_ct_n_e = ColorType_from_str(name); \
        if (a_ct_n_e != ct2) { \
            fail("ColorType %s (%s) != ColorType %s\n", colr_repr(a_ct_n_e), name, colr_repr(ct2)); \
        } \
    } while (0)

describe(ColorType) {
subdesc(ColorType_from_str) {
    it("recognizes bad values") {
        struct TestItem {
            const char* arg;
            ColorType ret;
        } tests[] = {
            {NULL, TYPE_INVALID},
            {"", TYPE_INVALID},
            {"NOTACOLOR", TYPE_INVALID},
            {"red", TYPE_BASIC},
            {"lightblue", TYPE_BASIC},
            {"xblue", TYPE_EXTENDED},
            {"xlightblue", TYPE_EXTENDED},
            {"1", TYPE_EXTENDED},
            {"255", TYPE_EXTENDED},
            {"-1", TYPE_INVALID_EXTENDED_RANGE},
            {"--1", TYPE_INVALID},
            {"256", TYPE_INVALID_EXTENDED_RANGE},
            {"234,234,234", TYPE_RGB},
            {"355,255,255", TYPE_INVALID_RGB_RANGE},
            {"underline", TYPE_STYLE},
            {"bright", TYPE_STYLE},
        };
        for_each(tests, i) {
            const char* arg = tests[i].arg;
            ColorType ret = tests[i].ret;
            assert_ColorType_name_equal(arg, ret);
        }
    }
    it("honors the basic_names mapping") {
        // Test all basic names, in case of some weird regression.
        for_len(basic_names_len, i) {
            char* name = basic_names[i].name;
            assert_ColorType_name_equal(name, (ColorType)TYPE_BASIC);
        }
    }
    it("honors the style_names mapping") {
        // Test all style names, in case of some weird regression.
        char* known_basic[] = {"none", "normal", "reset"};
        for_len(style_names_len, i) {
            bool known_duplicate = false;
            for_each(known_basic, j) {
                if (colr_str_eq(style_names[i].name, known_basic[j])) {
                    // This is a known thing.
                    // "none"/"reset", and others are also a BasicValue names,
                    // and ColorType_from_str honors them first.
                    // TODO: This may be re-worked so that the style comes first,
                    //       but then the "honors basic_names" tests would fail too.
                    known_duplicate = true;
                    break;
                }

            }
            if (known_duplicate) continue;

            char* name = style_names[i].name;
            assert_ColorType_name_equal(name, (ColorType)TYPE_STYLE);
        }
    }
}
subdesc(ColorType_is_invalid) {
    it("detects invalid ColorTypes") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            {"NOTACOLOR", true},
            {"white", false},
            {"xwhite", false},
            {"255;255;255", false},
            {"#aabbcc", false},
            {"aliceblue", false},
        };
        for_each(tests, i) {
            ColorType type = ColorType_from_str(tests[i].name);
            if (tests[i].expected) {
                assert_is_invalid(type);
            } else {
                assert_is_valid(type);
            }
        }
    }
}
subdesc(ColorType_is_valid) {
    it("detects valid ColorTypes") {
        struct {
            char* name;
            bool expected;
        } tests[] = {
            {"NOTACOLOR", false},
            {"white", true},
            {"xwhite", true},
            {"255;255;255", true},
            {"#aabbcc", true},
            {"aliceblue", true},
        };
        for_each(tests, i) {
            ColorType type = ColorType_from_str(tests[i].name);
            if (tests[i].expected) {
                assert_is_valid(type);
            } else {
                assert_is_invalid(type);
            }
        }
    }
}
subdesc(ColorType_repr) {
    it("creates a ColorType repr") {
        struct {
            ColorType type;
            char* repr;
        } tests[] = {
            {TYPE_NONE, "TYPE_NONE"},
            {TYPE_BASIC, "TYPE_BASIC"},
            {TYPE_EXTENDED, "TYPE_EXTENDED"},
            {TYPE_RGB, "TYPE_RGB"},
            {TYPE_STYLE, "TYPE_STYLE"},
            {TYPE_INVALID, "TYPE_INVALID"},
            {TYPE_INVALID_STYLE, "TYPE_INVALID_STYLE"},
            {TYPE_INVALID_EXTENDED_RANGE, "TYPE_INVALID_EXTENDED_RANGE"},
            {TYPE_INVALID_RGB_RANGE, "TYPE_INVALID_RGB_RANGE"},
        };
        for_each(tests, i) {
            char* s = ColorType_repr(tests[i].type);
            assert_str_eq(s, tests[i].repr, "ColorType_repr() failed!");
            free(s);
        }
    }
}
} // describe(ColorType)
