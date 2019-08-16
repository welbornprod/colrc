/*! Tests for the RGB struct, and it's related functions.

    \author Christopher Welborn
    \date 07-13-2019
*/
#include "test_RGB.h"

describe(RGB) {
subdesc(RGB_from_esc) {
    it("recognizes RGB escape codes") {
        for_len(colr_name_data_len, i) {
            RGB expected = colr_name_data[i].rgb;
            char codes[CODE_RGB_LEN];
            RGB rgb;
            // Test fore colors.
            format_fg_RGB(codes, expected);
            assert_rgb_from(codes, RGB_from_esc, 0, &rgb);
            assert_RGB_eq(rgb, expected);
            // Test back colors also.
            format_bg_RGB(codes, expected);
            assert_rgb_from(codes, RGB_from_esc, 0, &rgb);
            assert_RGB_eq(rgb, expected);
        }
    }
}
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
}
