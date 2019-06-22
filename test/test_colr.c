/* test_colr.c
    Tests for colr.h.
    Must be linked with the CMocka library.
    -Christopher Welborn 06-10-2019
*/
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmocka.h>
#include "../colr.h"

static void test_colorname_to_color(void **state) {
    /*  Tests colorname_to_color basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    assert_true(colorname_to_color("NOTACOLOR") == COLOR_INVALID);
}

static void test_colorname_type(void **state) {
    /*  Tests colorname_to_color basic usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    struct TestItems {
        const char* arg;
        ColorNameType ret;
    } test_items[] = {
        {"NOTACOLOR", COLORNAME_INVALID},
        {"red", COLORNAME_BASIC},
        {"lightblue", COLORNAME_EXTENDED},
        {"234,234,234", COLORNAME_RGB},
        {"355,255,255", COLORNAME_INVALID_RGB_RANGE},
    };
    size_t test_item_len = sizeof(test_items) / sizeof(struct TestItems);
    for (size_t i = 0; i < test_item_len; i++) {
        const char *arg = test_items[i].arg;
        ColorNameType ret = test_items[i].ret;
        assert_true(colorname_type(arg) == ret);
    }

    for (size_t i = 0; i < color_names_len; i++) {
        char *name = color_names[i].name;
        if (str_startswith(name, "light") || str_startswith(name, "x")) {
            assert_true(colorname_type(name) == COLORNAME_EXTENDED);
        } else {
            assert_true(colorname_type(name) == COLORNAME_BASIC);
        }
    }
}

static void test_format_bg(void **state) {
    /*  Tests basic format_bg usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODE_LEN];
    format_bg(codeonly, RED);
    assert_true(strlen(codeonly) > 3);
}

static void test_format_bgx(void **state) {
    /*  Tests basic format_bgx usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODEX_LEN];
    format_bgx(codeonly, LIGHTRED);
    assert_true(strlen(codeonly) > 3);
}

static void test_format_bg_rgb(void **state) {
    /*  Tests basic format_bg_rgb usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODE_RGB_LEN];
    unsigned char r = 25;
    unsigned char g = 35;
    unsigned char b = 45;
    format_bg_rgb(codeonly, r, g, b);
    assert_true(strlen(codeonly) < 30);
}

static void test_format_bg_RGB(void **state) {
    /*  Tests basic format_bg_RGB usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODE_RGB_LEN];
    RGB rgb = {25, 35, 45};
    format_bg_RGB(codeonly, rgb);
    assert_true(strlen(codeonly) < 30);
}

static void test_format_fg(void **state) {
    /*  Tests basic format_fg usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODE_LEN];
    format_fg(codeonly, RED);
    assert_true(strlen(codeonly) > 3);
}

static void test_str_startswith(void **state) {
    /*  Tests str_startswith.
    */
    (void)state; // Unused.
    // Common uses.
    assert_true(str_startswith("lightblue", "light"));
    assert_true(str_startswith("xred", "x"));
    assert_true(str_startswith("yellow", "yel"));
    assert_true(str_startswith("!@#$^&*", "!@"));
    assert_true(str_startswith("    test", "    "));
    // Should not trigger a match.
    assert_false(str_startswith("test", "a"));
    assert_false(str_startswith(" test", "test"));
    assert_false(str_startswith("t", "apple"));
    assert_false(str_startswith(NULL, "a"));
    assert_false(str_startswith("test", NULL));
    assert_false(str_startswith(NULL, NULL));
}

int run_colorname_tests(void) {
    const struct CMUnitTest colorname_tests[] = {
        cmocka_unit_test(test_colorname_to_color),
        cmocka_unit_test(test_colorname_type),
    };
    return cmocka_run_group_tests_name("colorname_tests", colorname_tests, NULL, NULL);
}

int run_format_bg_tests(void) {
    const struct CMUnitTest format_bg_tests[] = {
        cmocka_unit_test(test_format_bg),
        cmocka_unit_test(test_format_bgx),
        cmocka_unit_test(test_format_bg_rgb),
        cmocka_unit_test(test_format_bg_RGB),
    };
    return cmocka_run_group_tests_name("format_bg_tests", format_bg_tests, NULL, NULL);
}

int run_format_fg_tests(void) {
    const struct CMUnitTest format_fg_tests[] = {
        cmocka_unit_test(test_format_fg),
    };
    return cmocka_run_group_tests_name("format_fg_tests", format_fg_tests, NULL, NULL);
}

int run_helper_tests(void) {
    const struct CMUnitTest helper_tests[] = {
        cmocka_unit_test(test_str_startswith),
    };
    return cmocka_run_group_tests_name("helper_tests", helper_tests, NULL, NULL);
}

int main(int argc, char *argv[]) {
    (void)argc; // <- To silence linters when not using argc.
    (void)argv; // <- To silence linters when not using argv.
    int errs = 0;
    errs += run_helper_tests();
    errs += run_colorname_tests();
    errs += run_format_bg_tests();
    errs += run_format_fg_tests();
    return errs;
}
