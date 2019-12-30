/*! Tests for the RGB struct, and it's related functions.

    \author Christopher Welborn
    \date 07-13-2019
*/
#include "test_RGB.h"

describe(RGB) {
// RGB_average
    subdesc(RGB_average) {
    it("averages an RGB value") {
        // Not much to test here.
        struct {
            RGB rgbval;
            unsigned char avg;
        } tests[] = {
            {rgb(0, 0, 0), 0},
            {rgb(1, 0, 0), 0},
            {rgb(0, 1, 0), 0},
            {rgb(0, 0, 1), 0},
            {rgb(3, 0, 0), 1},
            {rgb(3, 3, 3), 3},
            {rgb(10, 10, 10), 10},
        };
        for_each(tests, i) {
            unsigned char avg = RGB_average(tests[i].rgbval);
            assert(avg == tests[i].avg);
        }
    }
}
// RGB_from_BasicValue
subdesc(RGB_from_BasicValue) {
    it("creates an RGB from a BasicValue") {
        // This function is pretty simple, and defaults to rgb(0, 0, 0) for
        // "bad" values.
        struct {
            BasicValue bval;
            RGB expected;
        } tests[] = {
            // This is just a copy of what's in RGB_from_BasicValue.
            // Not a very good test, but the function is very simple and all
            // cases are covered.
            {BASIC_INVALID_RANGE, rgb(0, 0, 0)},
            {BASIC_INVALID, rgb(0, 0, 0)},
            {BASIC_NONE, rgb(0, 0, 0)},
            {UNUSED, rgb(0, 0, 0)},
            {RESET, rgb(0, 0, 0)},
            {BLACK, rgb(1, 1, 1)},
            {RED, rgb(255, 0, 0)},
            {GREEN, rgb(0, 255, 0)},
            {YELLOW, rgb(255, 255, 0)},
            {BLUE, rgb(0, 0, 255)},
            {MAGENTA, rgb(255, 0, 255)},
            {CYAN, rgb(0, 255, 255)},
            {WHITE, rgb(255, 255, 255)},
            {LIGHTBLACK, rgb(128, 128, 128)},
            {LIGHTRED, rgb(255, 85, 85)},
            {LIGHTGREEN, rgb(135, 255, 135)},
            {LIGHTYELLOW, rgb(255, 255, 215)},
            {LIGHTBLUE, rgb(175, 215, 215)},
            {LIGHTMAGENTA, rgb(255, 85, 255)},
            {LIGHTCYAN, rgb(215, 255, 255)},
            {LIGHTWHITE, rgb(255, 255, 255)},
        };
        for_each(tests, i) {
            RGB result = RGB_from_BasicValue(tests[i].bval);
            assert_colr_eq_repr(result, tests[i].expected, result);
        }
    }
}
// RGB_from_ExtendedValue
subdesc(RGB_from_ExtendedValue) {
    it("creates an RGB from an ExtendedValue") {
        // This is just accessing ext2rgb_map[eval], but casting to
        // ExtendedValue first.
        struct {
            ExtendedValue eval;
            RGB expected;
        } tests[] = {
            {ext(1), ext2rgb_map[1]},
            {ext(10), ext2rgb_map[10]},
            {ext(120), ext2rgb_map[120]},
            {ext(200), ext2rgb_map[200]},
            {ext(255), ext2rgb_map[255]},
            // Test overflow? Probably not guaranteed to wrap around.
            {ext(256), ext2rgb_map[0]},
            {ext(257), ext2rgb_map[1]},
            {ext(-2), ext2rgb_map[254]},
        };
        for_each(tests, i) {
            RGB result = RGB_from_ExtendedValue(tests[i].eval);
            assert_colr_eq_repr(result, tests[i].expected, tests[i].eval);
        }
    }
}
// RGB_from_esc
subdesc(RGB_from_esc) {
    it("recognizes RGB escape codes") {
        for_len(colr_name_data_len, i) {
            RGB expected = colr_name_data[i].rgb;
            char* codes = fore_str_static(expected);
            RGB rgb;
            // Test fore colors.
            assert_rgb_from(codes, RGB_from_esc, 0, &rgb);
            assert_RGB_eq(rgb, expected);
            // Test back colors also.
            codes = back_str_static(expected);
            assert_rgb_from(codes, RGB_from_esc, 0, &rgb);
            assert_RGB_eq(rgb, expected);
        }
    }
}
// RGB_from_hex
subdesc(RGB_from_hex) {
    it("recognizes hex strings") {
        for_len(hex_tests_len, i) {
            RGB rgb;
            // The call should return 0 itself.
            assert_rgb_from(hex_tests[i].input, RGB_from_hex, 0, &rgb);
            // The values returned should match the expected tests.
            assert_RGB_eq(rgb, hex_tests[i].rgb);
        }
    }
    it("recognizes bad hex values") {
        for_len(bad_hex_tests_len, i) {
            char* badstr = bad_hex_tests[i];
            // Should be COLOR_INVALID.
            RGB rgb;
            assert_rgb_from(badstr, RGB_from_hex, COLOR_INVALID, &rgb);
        }
    }
}
// RGB_from_hex_default
subdesc(RGB_from_hex_default) {
    it("recognizes hex strings") {
        for_len(hex_tests_len, i) {
            RGB rgb;
            // The call should return 0 itself.
            assert_rgb_from(hex_tests[i].input, RGB_from_hex, 0, &rgb);
            // The values returned should match the expected tests.
            assert_RGB_eq(rgb, hex_tests[i].rgb);
        }
    }
    it("defaults to user's value") {
        RGB defaults[] = {
            rgb(0, 0, 0),
            rgb(255, 255, 255),
            rgb(1, 2, 3)
        };
        for_each(defaults, i) {
            for_len(bad_hex_tests_len, j) {
                char* hexstr = bad_hex_tests[j];
                assert_colr_eq_repr(
                    RGB_from_hex_default(hexstr, defaults[i]),
                    defaults[i],
                    hexstr
                );
            }
        }
    }
}

// RGB_from_str
subdesc(RGB_from_str) {
    it("recognizes valid RGB strings") {
        for_len(str_tests_len, i) {
            RGB rgb;
            // The call should return 0 itself.
            assert_rgb_from(str_tests[i].input, RGB_from_str, 0, &rgb);
            // The values returned should match the expected tests.
            assert_RGB_eq(rgb, str_tests[i].rgb);
        }

    }
    it("recognizes bad str values") {
        for_len(bad_str_tests_len, i) {
            char* badstr = bad_str_tests[i];
            // Should be COLOR_INVALID.
            RGB rgb;
            assert_rgb_from_either(
                badstr,
                RGB_from_str,
                COLOR_INVALID,
                COLOR_INVALID_RANGE,
                &rgb
            );
        }
    }
    it("recognizes known color names") {
        for_len(colr_name_data_len, i) {
            char* name = colr_name_data[i].name;
            RGB expected = colr_name_data[i].rgb;
            RGB rgb;
            assert_rgb_from(name, RGB_from_str, 0, &rgb);
            assert_RGB_eq(rgb, expected);
        }
    }
    it("recognizes hex strings") {
        // This is the same test for RGB_from_hex:"recognizes hex strings"
        // It should produce the same results.
        for_len(hex_tests_len, i) {
            if (hex_tests[i].input[0] != '#') {
                // RGB_from_str expects to have the leading #.
                // It follows the same rules as the other from_str() functions,
                // where '111' could be confused as an ExtendedValue.
                continue;
            }
            RGB rgb;
            // The call should return 0 itself.
            assert_rgb_from(hex_tests[i].input, RGB_from_str, 0, &rgb);
            // The values returned should match the expected tests.
            assert_RGB_eq(rgb, hex_tests[i].rgb);
        }
    }
}
// RGB_grayscale
subdesc(RGB_grayscale) {
    it("creates grayscale RGBs") {
        // This is a pretty simple function, it will never return rgb(0, 0, 0).
        struct {
            RGB rgbval;
            RGB expected;
        } tests[] = {
            {rgb(0, 0, 0), rgb(1, 1, 1)},
            {rgb(10, 10, 10), rgb(10, 10, 10)},
            {rgb(10, 20, 30), rgb(20, 20, 20)},
            {rgb(6, 6, 9), rgb(7, 7, 7)},
        };
        for_each(tests, i) {
            RGB result = RGB_grayscale(tests[i].rgbval);
            assert_colr_eq(result, tests[i].expected);
        }
    }
}
// RGB_inverted
subdesc(RGB_inverted) {
    it("creates inverted RGBs") {
        // This function will never return rgb(0, 0, 0).
        struct {
            RGB rgbval;
            RGB expected;
        } tests[] = {
            {rgb(0, 0, 0), rgb(255, 255, 255)},
            {rgb(10, 10, 10), rgb(245, 245, 245)},
            {rgb(10, 20, 30), rgb(245, 235, 225)},
            {rgb(255, 255, 255), rgb(1, 1, 1)},
            {rgb(10, 10, 255), rgb(245, 245, 1)},
            {rgb(255, 10, 10), rgb(1, 245, 245)},
            {rgb(10, 255, 10), rgb(245, 1, 245)},
        };
        for_each(tests, i) {
            RGB result = RGB_inverted(tests[i].rgbval);
            assert_colr_eq(result, tests[i].expected);
        }
    }
}
// RGB_monochrome
subdesc(RGB_monochrome) {
    it("creates a monochrome RGB") {
        // This will always return rgb(255, 255, 255) or rgb(1, 1, 1).
        struct {
            RGB rgbval;
            RGB expected;
        } tests[] = {
            {rgb(0, 0, 0), rgb(1, 1, 1)},
            {rgb(10, 10, 10), rgb(1, 1, 1)},
            {rgb(10, 20, 30), rgb(1, 1, 1)},
            {rgb(255, 255, 255), rgb(255, 255, 255)},
            {rgb(100, 100, 255), rgb(255, 255, 255)},
            {rgb(129, 129, 129), rgb(255, 255, 255)},
            {rgb(128, 128, 128), rgb(1, 1, 1)},
        };
        for_each(tests, i) {
            RGB result = RGB_monochrome(tests[i].rgbval);
            assert_colr_eq(result, tests[i].expected);
        }
    }
}

// RGB_repr
subdesc(RGB_repr) {
    it("creates a repr") {
        char* repr = RGB_repr(rgb(34, 26, 47));
        assert_not_null(repr);
        assert_str_starts_with(repr, "RGB");
        free(repr);
    }
}
// RGB_to_hex
subdesc(RGB_to_hex) {
    it("converts rgb values to hex") {
        struct {
            RGB rgb;
            char* expected;
        } tests[] = {
            {rgb(0, 0, 0), "#000000"},
            {rgb(255, 255, 255), "#ffffff"},
            {rgb(1, 2, 3), "#010203"},
        };
        for_each(tests, i) {
            char* hexstr = RGB_to_hex(tests[i].rgb);
            assert_str_eq_repr(
                hexstr,
                tests[i].expected,
                tests[i].rgb
            );
            free(hexstr);
        }
    }
}
// RGB_to_str
subdesc(RGB_to_str) {
    it("converts rgb values to str") {
        struct {
            RGB rgb;
            char* expected;
        } tests[] = {
            {rgb(0, 0, 0), "000;000;000"},
            {rgb(255, 255, 255), "255;255;255"},
            {rgb(1, 2, 3), "001;002;003"},
        };
        for_each(tests, i) {
            char* rgbstr = RGB_to_str(tests[i].rgb);
            assert_str_eq_repr(
                rgbstr,
                tests[i].expected,
                tests[i].rgb
            );
            free(rgbstr);
        }
    }
}
// RGB_to_term_RGB
subdesc(RGB_to_term_RGB) {
    it("creates the nearest term-color for RGBs") {
        struct {
            RGB rgbval;
            RGB expected;
        } tests[] = {
            {rgb(150, 150, 100), rgb(135, 135, 95)},
            {rgb(255, 255, 220), rgb(255, 255, 215)},
            {rgb(170, 20, 30), rgb(175, 0, 0)},
            {rgb(125, 125, 101), rgb(135, 135, 95)},
            {rgb(10, 10, 255), rgb(0, 0, 255)},
            {rgb(255, 10, 10), rgb(255, 0, 0)},
            {rgb(10, 255, 10), rgb(0, 255, 0)},
        };
        for_each(tests, i) {
            RGB result = RGB_to_term_RGB(tests[i].rgbval);
            assert_colr_eq(result, tests[i].expected);
        }
    }
}
} // describe(RGB)
