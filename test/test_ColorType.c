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
    }
}
