/*! Tests for the format_* functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_colr.h"

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


int run_format_bg_tests(void) {
    return_cm_tests(
        format_bg,
        cm_test(test_format_bg),
        cm_test(test_format_bgx),
        cm_test(test_format_bg_rgb),
        cm_test(test_format_bg_RGB)
    );
}

int run_format_fg_tests(void) {
    return_cm_tests(
        format_fg,
        cm_test(test_format_fg),
    );
}
