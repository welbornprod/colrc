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

static void my_test(void **state) {
    (void)state; // Unused (no setup/teardown function used.)
    // No tests yet.
}

int main(int argc, char *argv[]) {
    (void)argc; // <- To silence linters when not using argc.
    (void)argv; // <- To silence linters when not using argv.
    const struct CMUnitTest colr_tests[] = {
        cmocka_unit_test(my_test),
    };
    return cmocka_run_group_tests_name("Colr Tests", colr_tests, NULL, NULL);
}
