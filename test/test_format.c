/*! Tests for the format_* functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(format_functions) {
    subdesc(format_bg) {
        it("basic format_bg usage.") {
            char codeonly[CODE_LEN];
            format_bg(codeonly, RED);
            size_t bglen = strlen(codeonly);
            assert_range(bglen, CODE_LEN - 2, CODE_LEN - 1, "Invalid length for basic code.");
        }
    }

    subdesc(format_bgx) {
        it("basic format_bgx usage.") {
            char codeonly[CODEX_LEN];
            format_bgx(codeonly, LIGHTRED);
            size_t bgx_len = strlen(codeonly);
            assert_range(bgx_len, CODEX_LEN - 2, CODEX_LEN -1, "Invalid length for ext code");
        }
    }

    subdesc(format_bg_rgb) {
        it("basic format_bg_rgb usage.") {
            char codeonly[CODE_RGB_LEN];
            unsigned char r = 25;
            unsigned char g = 35;
            unsigned char b = 45;
            format_bg_rgb(codeonly, r, g, b);
            size_t bg_rgblen = strlen(codeonly);
            assert_range(
                bg_rgblen,
                CODE_RGB_LEN - 7,
                CODE_RGB_LEN - 1,
                "Invalid length for rgb code"
            );
        }
    }

    subdesc(format_bg_RGB) {
        it("basic format_bg_RGB usage.") {
            char codeonly[CODE_RGB_LEN];
            struct RGB rgb = {25, 35, 45};
            format_bg_RGB(codeonly, rgb);
            size_t bg_RGBlen = strlen(codeonly);
             assert_range(
                bg_RGBlen,
                CODE_RGB_LEN - 7,
                CODE_RGB_LEN - 1,
                "Invalid length for RGB code"
            );
        }
    }

    subdesc(format_fg) {
        it("basic format_fg usage.") {
            char codeonly[CODE_LEN];
            format_fg(codeonly, RED);
            size_t fglen = strlen(codeonly);
            assert_range(
                fglen,
                CODE_LEN - 2,
                CODE_LEN - 1,
                "Invalid length for basic code"
            );
        }
    }
}
