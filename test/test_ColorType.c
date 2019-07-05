/*! Tests for the ColorType enum, and it's related functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_colr.h"

#define assert_ColorType_name_equal(name, ct2) \
    do { \
        ColorType a_ct_n_e = ColorType_from_str(name); \
        if (a_ct_n_e != ct2) { \
            fail_msg("ColorType %s (%s) != ColorType %s\n", colr_repr(a_ct_n_e), name, colr_repr(ct2)); \
        } \
    } while (0)


static void test_ColorType_from_str(void** state) {
    /*! Tests ColorType_from_str basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
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
        {"256", TYPE_INVALID_EXTENDED_RANGE},
        {"234,234,234", TYPE_RGB},
        {"355,255,255", TYPE_INVALID_RGB_RANGE},
    };
    size_t test_item_len = array_length(tests);
    for (size_t i = 0; i < test_item_len; i++) {
        const char* arg = tests[i].arg;
        ColorType ret = tests[i].ret;
        assert_ColorType_name_equal(arg, ret);
    }

    // Test all basic names, in case of some weird regression.
    for (size_t i = 0; i < basic_names_len; i++) {
        char* name = basic_names[i].name;
        assert_ColorType_name_equal(name, (ColorType)TYPE_BASIC);
    }
}

int run_ColorType_tests(void) {
    return_cm_tests(
        ColorType,
        cm_test(test_ColorType_from_str),
    );
}
