#include "test_colr.h"


static void test_ColorType_from_str(void **state) {
    /*  Tests ColorType_from_str basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    struct TestItems {
        const char* arg;
        ColorType ret;
    } test_items[] = {
        {"NOTACOLOR", TYPE_INVALID},
        {"red", TYPE_BASIC},
        {"lightblue", TYPE_BASIC},
        {"234,234,234", TYPE_RGB},
        {"355,255,255", TYPE_INVALID_RGB_RANGE},
    };
    size_t test_item_len = sizeof(test_items) / sizeof(struct TestItems);
    for (size_t i = 0; i < test_item_len; i++) {
        const char *arg = test_items[i].arg;
        ColorType ret = test_items[i].ret;
        assert_true(ColorType_from_str(arg) == ret);
    }

    for (size_t i = 0; i < color_names_len; i++) {
        char *name = color_names[i].name;
        if (str_startswith(name, "x")) {
            assert_true(ColorType_from_str(name) == TYPE_EXTENDED);
        } else {
            assert_true(ColorType_from_str(name) == TYPE_BASIC);
        }
    }
}

int run_ColorType_tests(void) {
    return_cm_tests(
        ColorType,
        cm_test(test_ColorType_from_str),
    );
}
