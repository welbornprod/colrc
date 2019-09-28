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
subdesc(RGB_repr) {
    it("creates a repr") {
        char* repr = RGB_repr(rgb(34, 26, 47));
        assert_not_null(repr);
        assert_str_starts_with(repr, "RGB");
        free(repr);
    }
}
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
}
