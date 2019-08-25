/*! Tests for the ExtendedValue enum, and it's related functions.

    \author Christopher Welborn
    \date 07-01-2019
*/
#include "test_ColrC.h"
#include "test_ExtendedValue.h"

describe(ExtendedValue) {
subdesc(ExtendedValue_from_esc) {
    it("recognizes valid extended codes") {
        for (unsigned short eval = 0; eval < 256; eval++) {
            char codes[CODEX_LEN];
            format_fgx(codes, eval);
            assert_ext_eq(
                ExtendedValue_from_esc(codes),
                eval,
                "Failed to recognize fore code."
            );
            format_bgx(codes, eval);
            assert_ext_eq(
                ExtendedValue_from_esc(codes),
                eval,
                "Failed to recognize back code."
            );
        }
    }
}
subdesc(ExtendedValue_from_str) {
    subdesc(invalid_colors) {
        it("invalid color names should return COLOR_INVALID") {
            assert_ext_from_str_eq(
                "NOTACOLOR",
                COLOR_INVALID,
                "Invalid color name should be an invalid ExtendedValue"
            );
        }
    }
    subdesc(valid_colors) {
        it("recognizes extended color names") {
            // Test all basic names, in case of some weird regression.
            for_len(extended_names_len, i) {
                char* name = extended_names[i].name;
                int eval = extended_names[i].value;
                assert_ext_from_str_eq(
                   name,
                    eval,
                    "Known ExtendedValue didn't match"
                );
            }
        }
        it("recognizes known color names") {
            for_len(colr_name_data_len, i) {
                char* name = colr_name_data[i].name;
                ExtendedValue eval = colr_name_data[i].ext;
                assert_ext_from_str_eq(
                    name,
                    eval,
                    "Known name didn't match ExtendedValue."
                );
            }
        }
        it("recognizes known extended numbers") {
            char numstr[4];
            // Test all 0-255 values as strings.
            for (int i = 0; i < 256; i++) {
                sprintf(numstr, "%d", i);
                int eval = ExtendedValue_from_str(numstr);
                assert_ext_eq(i, eval, "Known extended number was considered invalid");
                assert_range(
                    eval,
                    0,
                    255,
                    "ExtendedValue within range returned invalid."
                );
                assert_ext_neq(
                    eval,
                    EXT_INVALID,
                    "Known extended number caused an invalid range"
                );
                assert_ext_neq(
                    eval,
                    EXT_INVALID_RANGE,
                    "Known extended number caused an invalid range"
                );

            }
        }
        it("returns EXT_INVALID_RANGE for bad numbers") {
            // Test bad numbers.
            char* nums[] = {"-255", "-1", "256", "355"};
            for_each(nums, i) {
                int eval = ExtendedValue_from_str(nums[i]);
                assert_ext_eq(
                    eval,
                    EXT_INVALID_RANGE,
                    "Bad number was not considered outside of the range"
                );
            }
        }
        it("returns COLOR_INVALID for really bad numbers") {
            // numstr needs to hold the largest stringified number.
            char* nums[] = {"-2555", "-1000", "2560", "2147483647"};
            for_each(nums, i) {
                assert_ext_from_str_eq(
                    nums[i],
                    EXT_INVALID_RANGE,
                    "Bad number was not considered invalid"
                );
            }
            // Something like: 18446744073709551615
            unsigned long long largest_num = ULLONG_MAX;
            char largenumstr[21];
            sprintf(largenumstr, "%llu", largest_num);
            assert_ext_from_str_eq(
                largenumstr,
                EXT_INVALID_RANGE,
                "Really long number was not considered invalid"
            );
        }
    }
}
} // describe(BasicValue)
