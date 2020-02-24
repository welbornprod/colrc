/*! Tests for the rainbow-related functions.

    \author Christopher Welborn
    \date 12-29-2019
*/
#include "test_ColrC.h"

describe(rainbow) {
subdesc(_rainbow) {
    it("handles NULL strings") {
        char* s = _rainbow(format_fg_RGB, NULL, 0.0, 0, 0);
        assert_null(s);
    }
    it("calls the RGB_fmter functions correctly") {
        // This is already kinda tested, through the other functions.
        // (if you consider those tests)
        // Use default offset/frequency/spread.
        size_t offset = 0;
        double freq = 0.0;
        size_t spread = 0;
        char* s = "This is my string.\nWith newlines.";
        RGB_fmter test_funcs[] = {
            format_bg_RGB,
            format_bg_RGB_term,
            format_fg_RGB,
            format_fg_RGB_term,
        };
        for_each(test_funcs, i) {
            char* rbow = _rainbow(test_funcs[i], s, freq, offset, spread);
            assert_not_null(rbow);
            assert_str_not_empty(rbow);
            assert_str_contains(rbow, "\n");
            assert_str_not_contains(rbow, "This");
            assert(colr_str_has_codes(rbow));
            free(rbow);
        }
    }
}
// rainbow_bg
subdesc(rainbow_bg) {
    it("rainbowizes back colors") {
        // TODO: better tests
        // Use default offset/frequency/spread.
        size_t offset = 0;
        double freq = 0.0;
        size_t spread = 0;
        char* s = "This is my string.";
        char* rbow = rainbow_bg(s, freq, offset, spread);
        assert_not_null(rbow);
        assert_str_not_empty(rbow);
        assert_str_not_contains(rbow, "This");
        assert(colr_str_has_codes(rbow));
        free(rbow);
    }
}
// rainbow_bg_term
subdesc(rainbow_bg_term) {
    it("rainbowizes back colors using term-friendly colors") {
        // TODO: better tests
        // Use default offset/frequency/spread.
        size_t offset = 0;
        double freq = 0.0;
        size_t spread = 0;
        char* s = "This is my string.";
        char* rbow = rainbow_bg_term(s, freq, offset, spread);
        assert_not_null(rbow);
        assert_str_not_empty(rbow);
        assert_str_not_contains(rbow, "This");
        assert(colr_str_has_codes(rbow));
        free(rbow);
    }
}
// rainbow_fg
subdesc(rainbow_fg) {
    it("rainbowizes fore colors") {
        // TODO: better tests
        // Use default offset/frequency/spread.
        size_t offset = 0;
        double freq = 0.0;
        size_t spread = 0;
        char* s = "This is my string.";
        char* rbow = rainbow_fg(s, freq, offset, spread);
        assert_not_null(rbow);
        assert_str_not_empty(rbow);
        assert_str_not_contains(rbow, "This");
        assert(colr_str_has_codes(rbow));
        free(rbow);
    }
}
// rainbow_fg_term
subdesc(rainbow_fg_term) {
    it("rainbowizes fore colors using term-friendly colors") {
        // TODO: better tests
        // Use default offset/frequency/spread.
        size_t offset = 0;
        double freq = 0.0;
        size_t spread = 0;
        char* s = "This is my string.";
        char* rbow = rainbow_fg_term(s, freq, offset, spread);
        assert_not_null(rbow);
        assert_str_not_empty(rbow);
        assert_str_not_contains(rbow, "This");
        assert(colr_str_has_codes(rbow));
        free(rbow);
    }
}
// rainbow_step
subdesc(rainbow_step) {
    it("wraps around eventually") {
        // TODO: a better test
        // Use default frequency.
        double freq = 0.0;
        // Not really sure what to test here. It will loop back around
        // eventually when it's working correctly.
        bool rainbow_wrapped_around = false;
        RGB firstrgb = rgb(0, 0, 0);
        for (size_t offset = 1; offset < 1000; offset++) {
            RGB rgbval = rainbow_step(freq, offset);
            if (colr_eq(firstrgb, rgb(0, 0, 0))) {
                // Set first RGB value produced.
                firstrgb = rgbval;
            } else if (colr_eq(firstrgb, rgbval)) {
                // It wrapped around.
                rainbow_wrapped_around = true;
                break;
            }
        }
        assert(rainbow_wrapped_around);
    }
}
} // describe(rainbow)
