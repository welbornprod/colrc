/*! Tests for the format_* functions.

    \author Christopher Welborn
    \date 06-29-2019
*/
#include "test_ColrC.h"

describe(formatters) {
subdesc(format_bg) {
    it("basic format_bg usage.") {
        char codeonly[CODE_LEN];
        format_bg(codeonly, RED);
        size_t bglen = strlen(codeonly);
        assert_range(bglen, CODE_LEN_MIN - 1, CODE_LEN - 1, "Invalid length for basic code");
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_bgx) {
    it("basic format_bgx usage.") {
        char codeonly[CODEX_LEN];
        format_bgx(codeonly, LIGHTRED);
        size_t bgx_len = strlen(codeonly);
        assert_range(bgx_len, CODEX_LEN_MIN - 1, CODEX_LEN -1, "Invalid length for ext code");
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_bg_RGB) {
    it("basic format_bg_RGB usage.") {
        char codeonly[CODE_RGB_LEN];
        RGB rgb = {25, 35, 45};
        format_bg_RGB(codeonly, rgb);
        size_t bg_RGBlen = strlen(codeonly);
         assert_range(
            bg_RGBlen,
            CODE_RGB_LEN_MIN - 1,
            CODE_RGB_LEN - 1,
            "Invalid length for RGB code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_bg_RGB_term) {
    it("basic format_bg_RGB_term usage.") {
        char codeonly[CODE_RGB_LEN];
        RGB rgb = {25, 35, 45};
        format_bg_RGB_term(codeonly, rgb);
        size_t bg_RGBlen = strlen(codeonly);
         assert_range(
            bg_RGBlen,
            CODEX_LEN_MIN - 1,
            CODEX_LEN - 1,
            "Invalid length for RGB term code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_fg) {
    it("basic format_fg usage.") {
        char codeonly[CODE_LEN];
        format_fg(codeonly, RED);
        size_t fglen = strlen(codeonly);
        assert_range(
            fglen,
            CODE_LEN_MIN - 1,
            CODE_LEN - 1,
            "Invalid length for basic code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_fgx) {
    it("basic format_fgx usage.") {
        char codeonly[CODEX_LEN];
        format_fgx(codeonly, LIGHTRED);
        size_t fgx_len = strlen(codeonly);
        assert_range(
            fgx_len,
            CODEX_LEN_MIN - 1,
            CODEX_LEN -1,
            "Invalid length for ext code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_fg_RGB) {
    it("basic format_fg_RGB usage.") {
        char codeonly[CODE_RGB_LEN];
        RGB rgb = {25, 35, 45};
        format_fg_RGB(codeonly, rgb);
        size_t fg_RGBlen = strlen(codeonly);
         assert_range(
            fg_RGBlen,
            CODE_RGB_LEN_MIN - 1,
            CODE_RGB_LEN - 1,
            "Invalid length for RGB code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

subdesc(format_fg_RGB_term) {
    it("basic format_fg_RGB_term usage.") {
        char codeonly[CODE_RGB_LEN];
        RGB rgb = {25, 35, 45};
        format_fg_RGB_term(codeonly, rgb);
        size_t fg_RGBlen = strlen(codeonly);
         assert_range(
            fg_RGBlen,
            CODEX_LEN_MIN - 1,
            CODEX_LEN - 1,
            "Invalid length for RGB term code"
        );
        // Codes get cut off if there is not enough space for them.
        assert(colr_str_ends_with(codeonly, "m"));
    }
}

    // TODO: Rainbow formatters.
}
