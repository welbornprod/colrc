/*! Tests for the RGB struct, and it's related functions.

    \author Christopher Welborn
    \date 07-13-2019
*/
#include "test_RGB.h"

describe(RGB) {
    subdesc(rgb_from_hex) {
        it("recognizes hex strings") {
            for (size_t i = 0; i < hex_tests_len; i++) {
                rgb_test_item item = hex_tests[i];
                unsigned char r = 0, g = 0, b = 0;
                // The call should return 0 itself.
                assert_rgb_from(item.teststr, rgb_from_hex, 0, &r, &g, &b);
                // The values returned should match the expected tests.
                assert_RGB_eq(rgb(r, g, b), rgb(item.red, item.green, item.blue));
            }
        }
        it("recognizes bad hex values") {
            for (size_t i = 0; i < bad_hex_tests_len; i++) {
                char* badstr = bad_hex_tests[i];
                // Should be COLOR_INVALID.
                unsigned char r = 0, g = 0, b = 0;
                assert_rgb_from(badstr, rgb_from_hex, COLOR_INVALID, &r, &g, &b);
            }
        }
    }
    subdesc(rgb_from_str) {
        it("recognizes valid rgb strings") {
            for (size_t i = 0; i < str_tests_len; i++) {
                rgb_test_item item = str_tests[i];
                unsigned char r = 0, g = 0, b = 0;
                // The call should return 0 itself.
                assert_rgb_from(item.teststr, rgb_from_str, 0, &r, &g, &b);
                // The values returned should match the expected tests.
                assert_RGB_eq(rgb(r, g, b), rgb(item.red, item.green, item.blue));
            }

        }
        it("recognizes bad str values") {
            for (size_t i = 0; i < bad_str_tests_len; i++) {
                char* badstr = bad_str_tests[i];
                // Should be COLOR_INVALID.
                unsigned char r = 0, g = 0, b = 0;
                assert_rgb_from_either(
                    badstr,
                    rgb_from_str,
                    COLOR_INVALID,
                    COLOR_INVALID_RANGE,
                    &r, &g, &b
                );
            }
        }
    }
    subdesc(RGB_from_hex) {
        it("recognizes hex strings") {
            for (size_t i = 0; i < hex_tests_len; i++) {
                rgb_test_item item = hex_tests[i];
                RGB rgb;
                // The call should return 0 itself.
                assert_rgb_from(item.teststr, RGB_from_hex, 0, &rgb);
                // The values returned should match the expected tests.
                assert_RGB_eq(rgb, rgb(item.red, item.green, item.blue));
            }

        }
        it("recognizes bad hex values") {
            for (size_t i = 0; i < bad_hex_tests_len; i++) {
                char* badstr = bad_hex_tests[i];
                // Should be COLOR_INVALID.
                RGB rgb;
                assert_rgb_from(badstr, RGB_from_hex, COLOR_INVALID, &rgb);
            }
        }
    }
    subdesc(RGB_from_str) {
        it("recognizes valid RGB strings") {
            for (size_t i = 0; i < str_tests_len; i++) {
                rgb_test_item item = str_tests[i];
                RGB rgb;
                // The call should return 0 itself.
                assert_rgb_from(item.teststr, RGB_from_str, 0, &rgb);
                // The values returned should match the expected tests.
                assert_RGB_eq(rgb, rgb(item.red, item.green, item.blue));
            }

        }
        it("recognizes bad str values") {
            for (size_t i = 0; i < bad_str_tests_len; i++) {
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
    }
}
