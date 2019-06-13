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
    struct RGB rgb = {25, 35, 45};
    format_bg_RGB(codeonly, &rgb);
    assert_true(strlen(codeonly) > 30);
}

static void test_format_fore(void **state) {
    /*  Tests basic format_fore usage.
    */
    (void)state; // Unused (no setup/teardown function used.)
    char codeonly[CODE_LEN];
    format_fore(codeonly, RED);
    assert_true(strlen(codeonly) > 3);
}

int run_colorname_tests(void) {
    const struct CMUnitTest colorname_tests[] = {
        cmocka_unit_test(test_colorname_to_color),
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

int run_format_fore_tests(void) {
    const struct CMUnitTest format_fore_tests[] = {
        cmocka_unit_test(test_format_fore),
    };
    return cmocka_run_group_tests_name("format_fore_tests", format_fore_tests, NULL, NULL);
}

int main(int argc, char *argv[]) {
    (void)argc; // <- To silence linters when not using argc.
    (void)argv; // <- To silence linters when not using argv.
    int errs = 0;
    errs += run_colorname_tests();
    errs += run_format_bg_tests();
    errs += run_format_fore_tests();
    return errs;
}
