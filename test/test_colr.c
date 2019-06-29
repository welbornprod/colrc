/* test_colr.c
    Tests for colr.h.
    Must be linked with libm and the CMocka library.
    -Christopher Welborn 06-10-2019
*/
#include "test_colr.h"

int main(int argc, char *argv[]) {
    (void)argc; // <- To silence linters when not using argc.
    (void)argv; // <- To silence linters when not using argv.
    //TODO: Run specific tests, just look at argc/argv.
    int errs = 0;
    errs += run_helper_tests();
    errs += run_format_bg_tests();
    errs += run_format_fg_tests();
    errs += run_BasicValue_tests();
    errs += run_ColorType_tests();
    return errs;
}
