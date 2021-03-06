/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_helpers.h"

describe(helpers) {
// _colr_ptr_repr
subdesc(_colr_ptr_repr) {
    it("handles strings") {
        char* s = "Testing this out.";
        char* orig = colr_str_repr(s);
        char* str = _colr_ptr_repr(s);
        assert_not_null(orig);
        assert_not_null(str);
        assert_str_not_empty(orig);
        assert_str_not_empty(str);
        assert_str_eq(str, orig, "Strings are mismatched.");
        free(str);
        free(orig);
    }
    it("handles ColorArgs") {
        ColorArg* carg = fore(RED);
        char* carg_orig = ColorArg_repr(*carg);
        char* carg_str = _colr_ptr_repr(carg);
        assert_not_null(carg_orig);
        assert_not_null(carg_str);
        assert_str_not_empty(carg_orig);
        assert_str_not_empty(carg_str);
        assert_str_eq(carg_str, carg_orig, "ColorArg strings are mismatched.");
        free(carg_str);
        free(carg_orig);
        colr_free(carg);
    }
    it("handles ColorTexts") {
        ColorText* ctext = Colr("Test", fore(BLUE));
        char* ctext_orig = ColorText_repr(*ctext);
        char* ctext_str = _colr_ptr_repr(ctext);
        assert_not_null(ctext_orig);
        assert_not_null(ctext_str);
        assert_str_not_empty(ctext_orig);
        assert_str_not_empty(ctext_str);
        assert_str_eq(ctext_str, ctext_orig, "ColorText strings are mismatched.");
        free(ctext_str);
        free(ctext_orig);
        colr_free(ctext);
    }
    it("handles ColorResults") {
        ColorResult* cres = Colr_cat("This ", Colr("test", style(BRIGHT)));
        char* cres_orig = ColorResult_repr(*cres);
        char* cres_str = _colr_ptr_repr(cres);
        assert_not_null(cres_orig);
        assert_not_null(cres_str);
        assert_str_not_empty(cres_orig);
        assert_str_not_empty(cres_str);
        assert_str_eq(cres_str, cres_orig, "ColorResult strings are mismatched.");
        free(cres_str);
        free(cres_orig);
        colr_free(cres);
    }
}
// _colr_ptr_to_str
subdesc(_colr_ptr_to_str) {
    it("handles strings") {
        char* orig = "Testing this out.";
        char* str = _colr_ptr_to_str(orig);
        assert_not_null(str);
        assert_str_not_empty(str);
        assert_str_eq(str, orig, "Strings are mismatched.");
        free(str);
    }
    it("handles ColorArgs") {
        ColorArg* carg = fore(RED);
        char* carg_orig = ColorArg_to_esc(*carg);
        char* carg_str = _colr_ptr_to_str(carg);
        assert_not_null(carg_orig);
        assert_not_null(carg_str);
        assert_str_not_empty(carg_orig);
        assert_str_not_empty(carg_str);
        assert_str_eq(carg_str, carg_orig, "ColorArg strings are mismatched.");
        free(carg_str);
        free(carg_orig);
        colr_free(carg);
    }
    it("handles ColorTexts") {
        ColorText* ctext = Colr("Test", fore(BLUE));
        char* ctext_orig = ColorText_to_str(*ctext);
        char* ctext_str = _colr_ptr_to_str(ctext);
        assert_not_null(ctext_orig);
        assert_not_null(ctext_str);
        assert_str_not_empty(ctext_orig);
        assert_str_not_empty(ctext_str);
        assert_str_eq(ctext_str, ctext_orig, "ColorText strings are mismatched.");
        free(ctext_str);
        free(ctext_orig);
        colr_free(ctext);
    }
    it("handles ColorResults") {
        // ColorResult_to_str is special. These will actually be the same string.
        ColorResult* cres = Colr_cat("This ", Colr("test", style(BRIGHT)));
        char* cres_orig = ColorResult_to_str(*cres);
        char* cres_str = _colr_ptr_to_str(cres);
        assert_not_null(cres_orig);
        assert_not_null(cres_str);
        assert_str_not_empty(cres_orig);
        assert_str_not_empty(cres_str);
        assert_str_eq(cres_str, cres_orig, "ColorResult strings are mismatched.");
        colr_free(cres);
    }
}
// colr_append_reset
subdesc(colr_append_reset) {
    it("accounts for newlines") {
        struct {
            char* input;
            char* expected;
        } tests[] = {
            {"", CODE_RESET_ALL},
            {"\n", CODE_RESET_ALL "\n"},
            {"test\n", "test" CODE_RESET_ALL "\n"},
            {"test\n\n\n\n", "test" CODE_RESET_ALL "\n\n\n\n"},
            {"test\n\n\n\n\n", "test" CODE_RESET_ALL "\n\n\n\n\n"},
            {"test\n another \n\n", "test\n another " CODE_RESET_ALL "\n\n"},
        };
        for_each(tests, i) {
            size_t expected_len = strlen(tests[i].expected);
            char s[expected_len + 1];
            colr_str_copy(s, tests[i].input, strlen(tests[i].input));
            colr_append_reset(s);
            assert_str_eq(
                s,
                tests[i].expected,
                "colr_append_reset() failed"
            );
        }
    }
}
// colr_char_escape_char
subdesc(colr_char_escape_char) {
    it("should recognize valid escape sequence chars") {
        struct {
            char input;
            char expected;
        } tests[] = {
            {'\0', '0'},
            {'\'', '\''},
            {'\"', '\"'},
            {'\?', '\?'},
            {'\\', '\\'},
            {'\a', 'a'},
            {'\b', 'b'},
            {'\f', 'f'},
            {'\n', 'n'},
            {'\r', 'r'},
            {'\t', 't'},
            {'\v', 'v'},
        };
        for_each(tests, i) {
            asserteq(
                colr_char_escape_char(tests[i].input),
                tests[i].expected,
                "Known escape char was not escaped."
            );
        }
    }
    it("should not escape regular ascii chars") {
        // 65-90 inclusive == A-Z.
        for (char C = 65; C <= 90; C++) { // Hah.
            asserteq(
                colr_char_escape_char(C),
                C,
                "Known non-escape char was escaped."
            );
        }
        // 97-122 inclusive == a-z.
        for (char c = 97; c <= 122; c++) {
            asserteq(
                colr_char_escape_char(c),
                c,
                "Known non-escape char was escaped."
            );
        }
    }
}
// colr_char_in_str
subdesc(colr_char_in_str) {
    it("should recognize characters in strings") {
        struct {
            char c;
            char* s;
            bool expected;
        } tests[] = {
            {'c', "char", true},
            {'c', "anotherchar", true},
            {'z', "endingwith the z", true},
            {'X', "endingwith the z", false},
            {'X', "endingwith the z", false},
            {'X', "endingwith the z", false}
        };
        for_each(tests, i) {
            asserteq(
                colr_char_in_str(tests[i].s, tests[i].c),
                tests[i].expected,
                "Known char was not detected."
            );
        }
    }
}
// colr_char_is_code_end
subdesc(colr_char_is_code_end) {
    it("detects known code-end chars") {
        struct {
            char c;
            bool expected;
        } tests[] = {
            {'a', true},
            {'A', true},
            {'i', true},
            {'h', true},
            {'m', true},
            {'?', false},
            {'!', false},
            {'.', false},
            {' ', false},
            {';', false},
        };
        for_each(tests, i) {
            asserteq(colr_char_is_code_end(tests[i].c), tests[i].expected);
        }
    }
}
// colr_char_repr
subdesc(colr_char_repr) {
    it("creates character representations") {
        struct {
            char c;
            char* expected;
        } tests[] = {
            {'\0', "'\\0'"},
            {'\x1b', "'\\x1b'"},
            {'\'', "'\\\''"},
            {'\"', "'\\\"'"},
            {'\?', "'\\?'"},
            {'\\', "'\\\\'"},
            {'\a', "'\\a'"},
            {'\b', "'\\b'"},
            {'\f', "'\\f'"},
            {'\n', "'\\n'"},
            {'\r', "'\\r'"},
            {'\t', "'\\t'"},
            {'\v', "'\\v'"},
            {3, "'\\x3'"},
            {'a', "'a'"},
        };
        for_each(tests, i) {
            char *repr = colr_char_repr(tests[i].c);
            assert_str_eq(repr, tests[i].expected, "colr_char_repr failed!");
            free(repr);
        }
    }
}
// colr_char_should_escape
subdesc(colr_char_should_escape) {
    it("should detect valid escape sequence chars") {
        char tests[] = {
            '\0',
            '\'',
            '\"',
            '\?',
            '\\',
            '\a',
            '\b',
            '\f',
            '\n',
            '\r',
            '\t',
            '\v',
        };
        for_each(tests, i) {
            assert(
                colr_char_should_escape(tests[i]),
                "Known escape char returned false."
            );
        }
    }
    it("should not produce false-positives") {
        // 65-90 inclusive == A-Z.
        for (char C = 65; C <= 90; C++) { // Hah.
            assert(
                !colr_char_should_escape(C),
                "Known non-escape char returned true."
            );
        }
        // 97-122 inclusive == a-z.
        for (char c = 97; c <= 122; c++) {
            assert(
                !colr_char_should_escape(c),
                "Known non-escape char returned true."
            );
        }
    }
}
// colr_check_marker
subdesc(colr_check_marker) {
    it("identifies valid markers") {
        ColorJustify* cjust = malloc(sizeof(ColorJustify));
        assert_not_null(cjust);
        *cjust = ColorJustify_empty();
        struct {
            void* p;
            uint32_t marker;
        } tests[] = {
            {fore(RED), COLORARG_MARKER},
            {back(WHITE), COLORARG_MARKER},
            {Colr("test", fore(RED)), COLORTEXT_MARKER},
            {cjust, COLORJUSTIFY_MARKER},
        };
        for_each(tests, i) {
            assert(colr_check_marker(tests[i].marker, tests[i].p));
            colr_free(tests[i].p);
        }
    }
    it("identifies invalid markers") {
        struct {
            void* p;
            uint32_t marker;
            bool expected;
        } str_tests[] = {
            {NULL, COLORLASTARG_MARKER, false},
            {"", COLORLASTARG_MARKER, false},
            {"abc", COLORLASTARG_MARKER, false},
            {"abcd", COLORLASTARG_MARKER, false},
            {"abcde", COLORLASTARG_MARKER, false},
            // This edge case that I hope no one ever uses.
            // All of the markers have their own edge case like this.
            {"\xff\xff\xff\xff", COLORARG_MARKER, true},
        };
        for_each(str_tests, i) {
            assert(colr_check_marker(str_tests[i].marker, str_tests[i].p) == str_tests[i].expected);
        }
    }
}
// colr_empty_str
subdesc(colr_empty_str) {
    it("sanity check for colr_empty_str()") {
        char* s = colr_empty_str();
        asserteq(s, "", "Empty string was not equal to \"\".");
        free(s);
    }
}
// colr_free_argsv
subdesc(colr_free_argsv) {
    it("only releases ColrC objects") {
        // Run this through valgrind to check for leaks and other violations.
        char* not_colrc = asserted_asprintf("%s", "Testing");
        test_colr_free_argsv(
            NULL,
            // Should leak if you're not careful:
            not_colrc,
            // Should not leak:
            fore(RED),
            back(BLUE),
            style(BRIGHT),
            Colr("Test", fore(RED)),
            Colr_join(";", Colr("this", fore(BLUE)), Colr("thing", fore(GREEN))),
            _ColrLastArg
        );
        assert_str_eq(not_colrc, "Testing", "String was changed!");
        free(not_colrc);
    }
}
// colr_is_colr_ptr
subdesc(colr_is_colr_ptr) {
    it("recognizes ColrC objects") {
        ColorArg* carg = fore(RED);
        assert(colr_is_colr_ptr(carg));
        colr_free(carg);
        ColorText* ctext = Colr("test", fore(RED));
        assert(colr_is_colr_ptr(ctext));
        colr_free(ctext);
        ColorResult* cres = Colr_join(" ", "This", "test", "here");
        assert(colr_is_colr_ptr(cres));
        colr_free(cres);
    }
    it("does not recognize other objects") {
        assert_false(colr_is_colr_ptr("Testing"));
        int x = 2600;
        assert_false(colr_is_colr_ptr(&x));
    }
}
// colr_supports_rgb
subdesc(colr_supports_rgb) {
    it("detects rgb support") {
        char* original = getenv("COLORTERM");
        setenv("COLORTERM", "truecolor", 1);
        assert(colr_supports_rgb());
        setenv("COLORTERM", "NOTAVALIDVALUE", 1);
        assert(!colr_supports_rgb());
        // Reset the original value, just in case.
        if (original) setenv("COLORTERM", original, 1);
    }
}
// colr_supports_rgb_static
subdesc(colr_supports_rgb_static) {
    it("detects rgb support, with only 1 environment check") {
        char* original = getenv("COLORTERM");
        setenv("COLORTERM", "truecolor", 1);
        bool is_supported = colr_supports_rgb_static();
        assert(is_supported);
        // Set a bad environment var value.
        setenv("COLORTERM", "NOTAVALIDVALUE", 1);
        // This should still succeed. Even though the environment changed,
        // we stored the result of the previous call.
        assert(colr_supports_rgb_static());
        // Reset the original value, just in case.
        if (original) setenv("COLORTERM", original, 1);
    }
}
// colr_term_size
// colr_win_size
// colr_win_size_env
subdesc(TermSize) {
    it("colr_term_size: doesn't crash") {
        // Not sure how to test this, at least the scaffolding will be here
        // when I find out.
        TermSize ts = colr_term_size();
        assert(ts.rows > 0);
        assert(ts.columns > 0);
    }
    it("colr_win_size: doesn't crash") {
        // Not sure how to test this, at least the scaffolding will be here
        // when I find out.
        struct winsize ws = colr_win_size();
        assert(ws.ws_row > 0);
        assert(ws.ws_col > 0);
        // This is always true, I don't know what else to test for.
        // assert(ws.ws_xpixel >= 0);
        // assert(ws.ws_ypixel >= 0);
    }
    it("colr_win_size_env: doesn't crash") {
        // Make colr_win_size_env() use LINES/COLS.
        if (setenv("LINES", "23", 1)) perror("can't set LINES for testing");
        if (unsetenv("COLUMNS")) perror("can't unset COLUMNS for testing");
        if (setenv("COLS", "32", 1)) perror("can't set COLS for testing");
        struct winsize ws = colr_win_size_env();
        assert(ws.ws_row == 23);
        assert(ws.ws_col == 32);
        // Make colr_win_size_env() use LINES/COLUMNS.
        if (setenv("COLUMNS", "54", 1)) perror("can't set COLUMNS for testing");
        ws = colr_win_size_env();
        assert(ws.ws_col == 54);
        // Make colr_win_size_env() use the defaults.
        if (unsetenv("COLS")) perror("can't unset COLS for testing");
        if (unsetenv("COLUMNS")) perror("can't unset COLUMNS for testing");
        if (unsetenv("LINES")) perror("can't unset LINES for testing");
        ws = colr_win_size_env();
        assert(ws.ws_row == 35);
        assert(ws.ws_col == 80);

    }
    it("create a repr") {
        char* expected = "TermSize {.rows=25, .columns=35}";
        TermSize ts = {.rows=25, .columns=35};
        char* repr = TermSize_repr(ts);
        assert_not_null(repr);
        assert_str_eq(repr, expected, "Bad TermSize repr.");
        free(repr);
    }
}
// colr_mb_len
subdesc(colr_mb_len) {
    it("handles NULL") {
        size_t widths[] = {
            1,
            2,
            100
        };
        for_each(widths, i) {
            assert_size_eq(colr_mb_len(NULL, widths[i]), 0);
            assert_size_eq(colr_mb_len("", widths[i]), 0);
        }
    }
    it("detects invalid multibyte strings") {
        struct {
            char* s;
            size_t length;
            size_t expected;
        } tests[] = {
            // Some invalid utf-8 strings:
            {"\xc3\x28", 2, -1},
            {"\xe2\x28\xa1", 2, -1},
            {"\xf0\x28\x8c\x28", 2, -1},
            // This is a utf16-encoded "Test\n".
            {"\xff\xfeT\x00e\x00s\x00t\x00\n\x00", 1, -1},
        };
        for_each(tests, i) {
            assert_size_eq_repr(
                colr_mb_len(tests[i].s, tests[i].length),
                tests[i].expected,
                tests[i].s
            );
        }
    }
    it("returns a byte count for multibyte chars") {
        struct {
            char* s;
            size_t char_len;
            size_t expected;
        } tests[] = {
            {"１３３７", 1, 3},
            {"１３３７", 2, 6},
            {"１３３７", 3, 9},
            {"１３３７", 4, 12},
            // A char_len that is too large is okay. It's basically strlen().
            {"１３３７", 100, 12},
            // Calling colr_mb_len on an ascii string is like calling strlen().
            {"test", 1, 1},
            {"test", 2, 2},
            {"test", 3, 3},
            {"test", 4, 4},
            {"test", 100, 4},
        };
        for_each(tests, i) {
            size_t byte_len = colr_mb_len(tests[i].s, tests[i].char_len);
            assert_size_eq_repr(byte_len, tests[i].expected, tests[i].s);
        }

        char* s = "No multibyte characters.";
        assert_size_eq_repr(
            colr_mb_len(s, strlen(s)),
            strlen(s),
            s
        );
    }
}
// colr_str_array_contains
subdesc(colr_str_array_contains) {
    it("detects str list elements") {
        char** lst = NULL;
        str_array_fill(
            lst,
            "test",
            "this",
            "out"
        );
        asserteq(colr_str_array_contains(lst, NULL), false);
        asserteq(colr_str_array_contains(lst, ""), false);
        assert(colr_str_array_contains(lst, "test"));
        assert(colr_str_array_contains(lst, "this"));
        assert(colr_str_array_contains(lst, "out"));
        colr_str_array_free(lst);

        str_array_fill(
            lst,
            ""
        );
        assert(colr_str_array_contains(lst, ""));
        colr_str_array_free(lst);

        str_array_fill(
            lst,
            "test",
            "",
            "this"
        );
        assert(colr_str_array_contains(lst, "this"));
        assert(colr_str_array_contains(lst, ""));
        colr_str_array_free(lst);
    }
}
// colr_str_array_free
subdesc(colr_str_array_free) {
    it("frees string lists") {
        // The real test is when is sent through valgrind.
        char** lst = NULL;
        str_array_fill(
            lst,
            "test",
            "this",
            "out"
        );
        colr_str_array_free(lst);
    }
}
// colr_str_center
subdesc(colr_str_center) {
    it("handles terminal width") {
        // Terminal width test.
        TermSize ts = colr_term_size();
        char* result = colr_str_center("test", 0, ' ');
        assert_not_null(result);
        assert_str_not_empty(result);
        assert(colr_str_starts_with(result, "  "));
        assert(colr_str_ends_with(result, "  "));
        assert_str_contains(result, "test");
        assert_size_eq(strlen(result), ts.columns);
        free(result);
    }
    it("center-justifies non-escape-code strings") {
        struct {
            char* s;
            char padchar;
            int width;
            char* expected;
        } tests[] = {
            {NULL, 0, 4, NULL},
            {"", 0, 1, " "},
            {"", 0, 4, "    "},
            {"a", 0, 1, "a"},
            {"a", 0, 4, "  a "},
            {"aa", 0, 4, " aa "},
            {"aaa", ' ', 4, " aaa"},
            {"aaaa  ", ' ', 10, "  aaaa    "},
            {FORE_CODE_BASIC "a", ' ', 4, "  " FORE_CODE_BASIC "a "},
            {FORE_CODE_EXT "a", ' ', 4, "  " FORE_CODE_EXT "a "},
            {FORE_CODE_RGB "a", ' ', 4, "  " FORE_CODE_RGB "a "},
            {STYLE_CODE_UL "a", ' ', 4, "  " STYLE_CODE_UL "a "},
            {"a" FORE_CODE_BASIC, ' ', 4, "  a" FORE_CODE_BASIC " "},
            {"a " FORE_CODE_EXT "a", ' ', 4, " a " FORE_CODE_EXT "a"},
            {
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a",
                ' ',
                4,
                "  a" FORE_CODE_RGB FORE_CODE_BASIC "a"
            },
            {
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a",
                ' ',
                5,
                "  a" FORE_CODE_RGB FORE_CODE_BASIC "a "
            },
            {
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a",
                ' ',
                6,
                "   a" FORE_CODE_RGB FORE_CODE_BASIC "a "
            },
            {
                STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT,
                ' ',
                5,
                "  " STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT "  "
            },

        };
        for_each(tests, i) {
            char* result = colr_str_center(tests[i].s, tests[i].width, tests[i].padchar);
            if (!result) {
                if (!tests[i].expected) {
                    // Expected null.
                    continue;
                }
                char* input_repr = colr_repr(tests[i].s);
                char* expected_repr = colr_repr(tests[i].expected);
                fail("Unexpected NULL from colr_str_center(%s): %s", input_repr, expected_repr);
            }
            assert_str_eq(result, tests[i].expected, "colr_str_center() failed");
            free(result);
        }
    }
}
// colr_str_char_count
subdesc(colr_str_char_count) {
    it("counts characters") {
        struct {
            char* input;
            char c;
            size_t expected;
        } tests[] = {
            {NULL, 'X', 0},
            {"X", 0, 0},
            {NULL, 0, 0},
            {"", 'X', 0},
            {"X", 'X', 1},
            {"before XX", 'X', 2},
            {"XX after", 'X', 2},
            {"in the XXX middle", 'X', 3},
            {"\nspecial\nchars\n\n", '\n', 4},
        };
        for_each(tests, i) {
            assert_size_eq_repr(
                colr_str_char_count(tests[i].input, tests[i].c),
                tests[i].expected,
                tests[i].input
            );
        }
    }
}
// colr_str_char_lcount
subdesc(colr_str_char_lcount) {
    it("counts starting characters") {
        struct {
            char* input;
            char c;
            size_t expected;
        } tests[] = {
            {NULL, 'X', 0},
            {"X", 0, 0},
            {NULL, 0, 0},
            {"", 'X', 0},
            {"X", 'X', 1},
            {"before XX", 'X', 0},
            {"XX after", 'X', 2},
            {"in the XXX middle", 'X', 0},
            {"\nspecial\nchars\n\n", '\n', 1},
            {"    test", ' ', 4},
        };
        for_each(tests, i) {
            assert_size_eq_repr(
                colr_str_char_lcount(tests[i].input, tests[i].c),
                tests[i].expected,
                tests[i].input
            );
        }
    }
}
// colr_str_chars_lcount
subdesc(colr_str_chars_lcount) {
    it("counts starting characters") {
        struct {
            char* input;
            char* chars;
            size_t expected;
        } tests[] = {
            {NULL, "X", 0},
            {"X", "", 0},
            {NULL, "", 0},
            {"", "X", 0},
            {"X", "X", 1},
            {"before XX", "X", 0},
            {"XX after", "XX", 2},
            {"in the XXX middle", "X", 0},
            {"\nspecial\nchars\n\n", "\n", 1},
            {"    test", " ", 4},
            {"cba test", "abc", 3},
            {" \n\t\vtest", "\n\t\v ", 4},
            {"  \n\n\t\t\v\vtest", "\n\t\v ", 8},
        };
        for_each(tests, i) {
            assert_size_eq_repr(
                colr_str_chars_lcount(tests[i].input, tests[i].chars),
                tests[i].expected,
                tests[i].input
            );
        }
    }
}
// colr_str_code_count
subdesc(colr_str_code_count) {
    it("counts escape codes") {
        ColorArg forearg = fore_arg(WHITE);
        ColorArg backarg = back_arg(RED);
        ColorArg stylearg = style_arg(RESET_ALL);

        struct {
            ColorText ctext;
            size_t expected;
        } tests[] = {
            {ColorText_from_values("Test", _ColrLastArg), 0},
            {ColorText_from_values("Test", &forearg, _ColrLastArg), 2},
            {ColorText_from_values("Test", &forearg, &backarg, _ColrLastArg), 3},
            {ColorText_from_values("Test", &forearg, &backarg, &stylearg, _ColrLastArg), 4},
        };
        for_each(tests, i) {
            char* s = ColorText_to_str(tests[i].ctext);
            assert_size_eq_repr(colr_str_code_count(s), tests[i].expected, s);
            free(s);
        }
        assert_size_eq(colr_str_code_count(NULL), 0);
        assert_size_eq(colr_str_code_count(""), 0);
        // Overflow the current_code buffer.
        // 1 extra char.
        assert_size_eq(colr_str_code_count("\x1b[38;2;255;255;2550m"), 0);
        // Many extra chars.
        char* waytoolong = "\x1b[38;2;255;255;2550101010101010101010101010101m";
        assert_size_eq(colr_str_code_count(waytoolong), 0);
    }
}
// colr_str_code_len
subdesc(colr_str_code_len) {
    it("counts escape code chars") {
        struct {
            char* s;
            size_t expected;
        } tests[] = {
            {NULL, 0},
            {"", 0},
            {"apple", 0},
            {"\x1b[0m", 4},
            {"test\x1b[0m", 4},
            {"\x1b[0mtest", 4},
            {"test\x1b[0mtest", 4},
            {FORE_CODE_BASIC, FORE_CODE_BASIC_LEN},
            {FORE_CODE_EXT, FORE_CODE_EXT_LEN},
            {FORE_CODE_RGB, FORE_CODE_RGB_LEN},
            {STYLE_CODE_UL, STYLE_CODE_UL_LEN},
            {"test" FORE_CODE_BASIC, FORE_CODE_BASIC_LEN},
            {"test" FORE_CODE_EXT, FORE_CODE_EXT_LEN},
            {"test" FORE_CODE_RGB, FORE_CODE_RGB_LEN},
            {"test" STYLE_CODE_UL, STYLE_CODE_UL_LEN},
            {FORE_CODE_BASIC "test", FORE_CODE_BASIC_LEN},
            {FORE_CODE_EXT "test", FORE_CODE_EXT_LEN},
            {FORE_CODE_RGB "test", FORE_CODE_RGB_LEN},
            {STYLE_CODE_UL "test", STYLE_CODE_UL_LEN},
            {"test" FORE_CODE_BASIC "test", FORE_CODE_BASIC_LEN},
            {"test" FORE_CODE_EXT "test", FORE_CODE_EXT_LEN},
            {"test" FORE_CODE_RGB "test", FORE_CODE_RGB_LEN},
            {"test" STYLE_CODE_UL "test", STYLE_CODE_UL_LEN},
        };
        for_each(tests, i) {
            assert_size_eq_repr(
                colr_str_code_len(tests[i].s),
                tests[i].expected,
                tests[i].s
            );
        }
        // Overflow the current_code buffer.
        // 1 extra char.
        assert_size_eq(colr_str_code_len("\x1b[38;2;255;255;2550m"), 0);
        // Many extra chars.
        char* waytoolong = "\x1b[38;2;255;255;2550101010101010101010101010101m";
        assert_size_eq(colr_str_code_len(waytoolong), 0);

    }
}
// colr_str_copy
subdesc(colr_str_copy) {
    it("copies strings") {
        char* destp = NULL;
        char* sp = NULL;
        assert(colr_str_copy(destp, sp, 1) == NULL);
        char s[] = "testing";
        size_t length = strlen(s);
        assert(colr_str_copy(destp, s, 4) == NULL);
        char* dest = calloc(length + 1, sizeof(char));
        assert(colr_str_copy(dest, NULL, 4) == NULL);
        colr_str_copy(dest, "", 1);
        assert_str_empty(dest);
        colr_str_copy(dest, s, 4);
        assert_str_eq(dest, "test", "Failed to copy 4 bytes from string.");
        colr_str_copy(dest, s, length);
        assert_str_eq(dest, s, "Failed to copy the entire string.");
        free(dest);
    }
}
// colr_str_ends_with
subdesc(colr_str_ends_with) {
    it("detects string endings") {
        struct {
            char* s;
            char* suffix;
            bool expected;
        } tests[] = {
            // Common uses.
            {"lightblue", "blue", true},
            {"xred", "red", true},
            {"yellow", "low", true},
            {"!@#$^&*", "&*", true},
            {"    test    ", "    ", true},
            {"test\x1b[0m", "\x1b[0m", true},
            // Should not trigger a match.
            {NULL, "a", false},
            {"test", NULL, false},
            {NULL, NULL, false},
            {"test", "a", false},
            {" test ", "test", false},
            {"t", "apple", false},
            {"\x1b[0mtest", "\x1b[0m", false},
        };
        for_each(tests, i) {
            asserteq(
                colr_str_ends_with(
                    tests[i].s,
                    tests[i].suffix
                ),
                tests[i].expected
            );
        }



    }
}
// colr_str_get_codes
subdesc(colr_str_get_codes) {
    it("builds escape-code lists") {
        assert_null(colr_str_get_codes(NULL, false));
        assert_null(colr_str_get_codes(NULL, true));
        assert_null(colr_str_get_codes("", false));
        assert_null(colr_str_get_codes("", true));
        assert_null(colr_str_get_codes("No codes in here.", false));
        assert_null(colr_str_get_codes("No codes in here.", true));
        // Cause an overflow that will be skipped.
        // These don't even touch the busy path. colr_str_code_count() causes
        // an early return because it doesn't accept overflow either.
        assert_null(colr_str_get_codes("\x1b[38;2;255;255;2550m", false));
        assert_null(colr_str_get_codes("\x1b[38;2;255;255;2550m", true));
        // Need at least one good code to trigger the overflow handler.
        char* waytoolong = "\x1b[0m\x1b[38;2;255;255;2550101010101010101010101010101m";
        char** code_array = colr_str_get_codes(waytoolong, false);
        assert_not_null(code_array);
        assert_str_array_size_eq_repr(
            colr_str_array_len(code_array),
            (size_t)1,
            code_array
        );
        colr_str_array_free(code_array);

        char** code_array_unique = colr_str_get_codes(waytoolong, true);
        assert_not_null(code_array_unique);
        assert_str_array_size_eq_repr(
            colr_str_array_len(code_array_unique),
            (size_t)1,
            code_array_unique
        );
        colr_str_array_free(code_array_unique);

        char* s = colr_cat(
            fore(RED),
            back(WHITE),
            style(BRIGHT),
            fore(ext(255)),
            fore(RED),
            back(WHITE),
            style(BRIGHT),
            fore(ext(255))
        );
        code_array = colr_str_get_codes(s, false);
        code_array_unique = colr_str_get_codes(s, true);;
        free(s);
        assert_not_null(code_array);
        // A reset code is appended when calling colr_cat() with ColorArgs.
        // So it's +1 for whatever items you see.
        assert_str_array_size_eq_repr(
            colr_str_array_len(code_array),
            (size_t)9,
            code_array
        );
        assert_str_array_contains(code_array, "\x1b[31m");
        assert_str_array_contains(code_array, "\x1b[47m");
        assert_str_array_contains(code_array, "\x1b[1m");
        assert_str_array_contains(code_array, "\x1b[38;5;255m");
        colr_str_array_free(code_array);

        assert_not_null(code_array_unique);
        // A reset code is appended when calling colr_cat() with ColorArgs.
        // So it's +1 for whatever *unique* items you see.
       assert_str_array_size_eq_repr(
            colr_str_array_len(code_array_unique),
            (size_t)5,
            code_array_unique
        );
        assert_str_array_contains(code_array_unique, "\x1b[31m");
        assert_str_array_contains(code_array_unique, "\x1b[47m");
        assert_str_array_contains(code_array_unique, "\x1b[1m");
        assert_str_array_contains(code_array_unique, "\x1b[38;5;255m");
        colr_str_array_free(code_array_unique);
    }
}
// colr_str_has_ColorArg
subdesc(colr_str_has_ColorArg) {
    it("handles NULL") {
        ColorArg* arg = fore(RED);
        assert_false(colr_str_has_ColorArg(NULL, arg));
        assert_false(colr_str_has_ColorArg("test", NULL));
        // Empty string.
        assert_false(colr_str_has_ColorArg("", arg));
        colr_free(arg);
        // Empty ColorArg.
        arg = ColorArg_to_ptr(ColorArg_empty());
        assert_false(colr_str_has_ColorArg("test", arg));
        colr_free(arg);
    }
    it("detects ColorArgs") {
        struct {
            char* s;
            ColorArg* carg;
            bool expected;
        } tests[] = {
            {colr("test", fore(RED)), fore(RED), true},
            {colr("test", fore(RED), back(BLUE)), back(BLUE), true},
            {colr("test", fore(RED), style(BRIGHT)), style(BRIGHT), true},
            {colr("test", fore(BLUE), back(RED)), fore(RED), false},
            {colr("test", fore(RED), back(RED), style(BRIGHT)), fore(BLUE), false},
        };
        for_each(tests, i) {
            bool result = colr_str_has_ColorArg(tests[i].s, tests[i].carg);
            asserteq(result, tests[i].expected);
            free(tests[i].s);
            colr_free(tests[i].carg);
        }
    }
}
// colr_str_has_codes
subdesc(colr_str_has_codes) {
    it("should detect escape codes") {
        // NULL should just return false.
        assert(colr_str_has_codes(NULL) == false);

        // Normal strings should not trigger this.
        assert(colr_str_has_codes("This is a string.") == false);

        // Empty strings should not trigger this.
        assert(colr_str_has_codes("") == false);

        // Colors should though.
        ColorArg* args[] = {
            fore(RED),
            back(LIGHTBLUE),
            style(UNDERLINE),
            fore(ext(32)),
            back(ext(254)),
            fore(rgb(12, 34, 56)),
            back(rgb(78, 89, 90)),
        };
        size_t args_len = array_length(args);
        for (size_t i = 0; i < args_len; i++) {
            char* s = colr_cat("This prefix.", args[i], "This suffix.");
            assert(colr_str_has_codes(s));
            free(s);
        }
    }
}
// colr_str_hash
subdesc(colr_str_hash) {
    it("computes simple string hashes") {
        ColrHash zero = 0;
        ColrHash empty = 5381;
        assert_hash_eq(colr_str_hash(NULL), zero);
        assert_hash_eq(colr_str_hash(""), empty);
        assert(colr_str_hash("test"));
    }
    it("does not collide for basic color names") {
        for_len(basic_names_len, i) {
            char* namea = basic_names[i].name;
            for_len(basic_names_len, j) {
                char* nameb = basic_names[j].name;
                if (colr_str_eq(namea, nameb)) continue;
                // Names are different, they should not be equal.
                assert_str_hash_neq(namea, nameb);
            }
        }
    }
    it("does not collide for style names") {
        for_len(style_names_len, i) {
            char* namea = style_names[i].name;
            for_len(style_names_len, j) {
                char* nameb = style_names[j].name;
                if (colr_str_eq(namea, nameb)) continue;
                // Names are different, they should not be equal.
                assert_str_hash_neq(namea, nameb);

            }
        }
    }
    it("does not collide for known color names") {
        for_len(colr_name_data_len, i) {
            char* namea = colr_name_data[i].name;
            for_len(colr_name_data_len, j) {
                char* nameb = colr_name_data[j].name;
                if (colr_str_eq(namea, nameb)) continue;
                // Names are different, they should not be equal.
                assert_str_hash_neq(namea, nameb);
            }
        }
    }
}
// colr_str_is_all
subdesc(colr_str_is_all) {
    it("should detect single-char strings") {
        struct {
            char* s;
            char c;
            bool expected;
        } tests[] = {
            {NULL, 0, false},
            {"test", 0, false},
            {NULL, 'a', false},
            {"aaa", 'a', true},
            {"aaaa", 'a', true},
            {"aaa", 'b', false},
            {"apple", 'a', false},
            {"xaaa", 'a', false},
        };
        for_each(tests, i) {
            asserteq(colr_str_is_all(tests[i].s, tests[i].c), tests[i].expected);
        }
    }
}
// colr_str_is_codes
subdesc(colr_str_is_codes) {
    it("should detect escape-code-only strings") {
        struct {
            char* s;
            bool expected;
        } tests[] = {
            {NULL, false},
            {"", false},
            {"\x1b[4m", true},
            {"\x1b[31m", true},
            {"\x1b[48;5;7m", true},
            {"\x1b[38;2;1;1;1m", true},
            {"\x1b[4m\x1b[31m\x1b[48;5;7m\x1b[0m", true},
            {"\x1b[1m\x1b[38;2;1;1;1m\x1b[48;5;1m\x1b[0m", true},
            {"X\x1b[4m\x1b[31m\x1b[48;5;7m\x1b[0m", false},
            {"\x1b[4mX\x1b[31m\x1b[48;5;7m\x1b[0m", false},
            {"\x1b[4m\x1b[31mX\x1b[48;5;7m\x1b[0m", false},
            {"\x1b[4m\x1b[31m\x1b[48;5;7mX\x1b[0m", false},
            {"\x1b[4m\x1b[31m\x1b[48;5;7m\x1b[0mX", false},
        };
        for_each(tests, i) {
            asserteq(
                colr_str_is_codes(tests[i].s),
                tests[i].expected,
            );
        }
    }
}
// colr_str_is_digits
subdesc(colr_str_is_digits) {
    it("should detect digit-only strings") {
        struct {
            char* s;
            bool expected;
        } tests[] = {
            {NULL, false},
            {"", false},
            {"0", true},
            {"1", true},
            {"1234567890", true},
            {"-1234", false},
            {"111a", false},
            {"a1111", false},
        };
        for_each(tests, i) {
            asserteq(colr_str_is_digits(tests[i].s), tests[i].expected);
        }
    }
}
// colr_str_ljust
subdesc(colr_str_ljust) {
    it("handles terminal width") {
        // Terminal width test.
        TermSize ts = colr_term_size();
        char* result = colr_str_ljust("test", 0, ' ');
        assert_not_null(result);
        assert_str_not_empty(result);
        assert(colr_str_ends_with(result, "  "));
        assert_str_contains(result, "test");
        assert_size_eq(strlen(result), ts.columns);
        free(result);
    }
    it("left-justifies non-escape-code strings") {
        struct {
            char* s;
            char padchar;
            int width;
            char* expected;
        } tests[] = {
            {NULL, 0, 4, NULL},
            {"", 0, 1, " "},
            {"", 0, 4, "    "},
            {"a", 0, 1, "a"},
            {"a", 0, 4, "a   "},
            {"aa", 0, 4, "aa  "},
            {"aaa", ' ', 4, "aaa "},
            {"aaaa  ", ' ', 10, "aaaa      "},
            {FORE_CODE_BASIC "a", ' ', 4, FORE_CODE_BASIC "a   "},
            {FORE_CODE_EXT "a", ' ', 4, FORE_CODE_EXT "a   "},
            {FORE_CODE_RGB "a", ' ', 4, FORE_CODE_RGB "a   "},
            {STYLE_CODE_UL "a", ' ', 4, STYLE_CODE_UL "a   "},
            {"a" FORE_CODE_BASIC, ' ', 4, "a" FORE_CODE_BASIC "   "},
            {"a " FORE_CODE_EXT "a", ' ', 4, "a " FORE_CODE_EXT "a "},
            {
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a",
                ' ',
                4,
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a "
            },
            {
                STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT,
                ' ',
                5,
                STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT "    "
            },

        };
        for_each(tests, i) {
            char* result = colr_str_ljust(tests[i].s, tests[i].width, tests[i].padchar);
            if (!result) {
                if (!tests[i].expected) {
                    // Expected null.
                    continue;
                }
                char* input_repr = colr_repr(tests[i].s);
                char* expected_repr = colr_repr(tests[i].expected);
                fail("Unexpected NULL from colr_str_ljust(%s): %s", input_repr, expected_repr);
            }
            assert_str_eq(result, tests[i].expected, "colr_str_ljust failed to justify.");
            free(result);
        }
    }
}
// colr_str_lower
subdesc(colr_str_lower) {
    it("should handle empty strings") {
        // Should not fail.
        colr_str_lower(NULL);

        // Should not fail.
        char* empty = "";
        colr_str_lower(empty);
        asserteq(
            empty,
            "",
            "Empty string did not return empty string."
        );
        char* allocempty = colr_empty_str();
        colr_str_lower(allocempty);
        asserteq(
            empty,
            "",
            "Empty string did not return empty string."
        );
        free(allocempty);
    }
    it("should lowercase strings") {
        struct {
            char* input;
            char* expected;
        } tests[] = {
            {"THIS IS IT.", "this is it."},
            {"mAcRoS aRe eViL!?%%$!", "macros are evil!?%%$!"},
        };

        for_each(tests, i) {
            char* input = NULL;
            if_not_asprintf(&input, "%s", tests[i].input) {
                fail("Allocation failed for test input string!");
            }
            colr_str_lower(input);
            asserteq(
                input,
                tests[i].expected,
                "String was not lowered."
            );
            free(input);
        }
    }
}
// colr_str_lstrip
// colr_str_lstrip_char
subdesc(colr_str_lstrip_char) {
    // This function calls colr_str_lstrip directly.
    it("should lstrip a char") {
        struct {
            char* s;
            char c;
            char* expected;
        } tests[] = {
            {"", 'c', NULL},
            {NULL, 'c', NULL},
            {NULL, 0, NULL},
            {"test", 0, "test"},
            {"test", 'c', "test"},
            {"aatest", 'a', "test"},
            {"btest", 'b', "test"},
            {"test", 't', "est"},
            {"tttttesttttt", 't', "esttttt"},
            {"\t\t\n test", '\t', "\n test"},
            {"\t\t\n test", 0, "test"},
        };
        for_each(tests, i) {
            char* result = colr_str_lstrip_char(tests[i].s, tests[i].c);
            if (!result) {
                if (tests[i].expected) {
                    fail(
                        "Falsely returned NULL: colr_str_lstrip_char(%s, %s)\n",
                        colr_repr(tests[i].s),
                        colr_repr(tests[i].c)
                    );
                } else {
                    // expected failure.
                    continue;
                }
            }
            assert_str_eq(result, tests[i].expected, "Failed to remove char.");
            free(result);
        }
    }
}
// colr_str_lstrip_chars
subdesc(colr_str_lstrip_chars) {
    it("should lstrip chars") {
        struct {
            char* s;
            char* chars;
            char* expected;
        } tests[] = {
            {"", "cba", NULL},
            {"test", "", NULL},
            {NULL, "cba", NULL},
            {"test", NULL, NULL},
            {NULL, NULL, NULL},
            {"test", "cba", "test"},
            {"aabbcctest", "cba", "test"},
            {"aabbcctcabest", "cba", "tcabest"},
            {" \t \t\n test", " \n\t", "test"},
            {"aabbcctest", "cba", "test"},
        };
        for_each(tests, i) {
            char* result = colr_str_lstrip_chars(tests[i].s, tests[i].chars);
            if (!result) {
                if (tests[i].expected) {
                    fail(
                        "Falsely returned NULL: colr_str_lstrip_chars(%s, %s)\n",
                        colr_repr(tests[i].s),
                        colr_repr(tests[i].chars)
                    );
                } else {
                    // expected failure.
                    continue;
                }
            }
            assert_str_eq(result, tests[i].expected, "Failed to remove chars.");
            free(result);
        }
    }
}
// colr_str_mb_len
subdesc(colr_str_mb_len) {
    it("counts single an multibyte chars") {
        struct {
            char* s;
            size_t expected;
        } tests[] = {
            {NULL, 0},
            {"", 0},
            {" ", 1},
            {"\n\n", 2},
            {"⮰⮱⮲⮳⮴⮵⮶⮷", 8},
            {"⮰test", 5},
            {"⮵test", 5},
            {"⮰test⮵", 6},
            {"⇇ ⇈ ⇉ ⇊ ⇶ ⬱ ⮄ ⮅ ⮆ ⮇ ⮔", 21},
        };
        setlocale(LC_ALL, "");
        for_each(tests, i) {
            size_t length = colr_str_mb_len(tests[i].s);
            assert_size_eq_repr(length, tests[i].expected, tests[i].s);
        }
    }
    it("detects invalid multibyte strings") {
        char* invalid_strs[] = {
            // This is a utf16-encoded "Test\n".
            "\xff\xfeT\x00e\x00s\x00t\x00\n\x00",
        };
        for_each(invalid_strs, i) {
            assert_size_eq_repr(colr_str_mb_len(invalid_strs[i]), 0, invalid_strs[i]);
        }
    }
}
// colr_str_noncode_len
subdesc(colr_str_noncode_len) {
    it("counts non-escape-code chars") {
        struct {
            char* s;
            size_t expected;
        } tests[] = {
            {NULL, 0},
            {"", 0},
            {FORE_CODE_BASIC "test", 4},
            {"test" FORE_CODE_BASIC, 4},
            {"test" FORE_CODE_BASIC "test", 8},
            {FORE_CODE_EXT "test", 4},
            {"test" FORE_CODE_EXT, 4},
            {"test" FORE_CODE_EXT "test", 8},
            {FORE_CODE_RGB "test", 4},
            {"test" FORE_CODE_RGB, 4},
            {"test" FORE_CODE_RGB "test", 8},
            {BACK_CODE_BASIC "test", 4},
            {"test" BACK_CODE_BASIC, 4},
            {"test" BACK_CODE_BASIC "test", 8},
            {BACK_CODE_EXT "test", 4},
            {"test" BACK_CODE_EXT, 4},
            {"test" BACK_CODE_EXT "test", 8},
            {BACK_CODE_RGB "test", 4},
            {"test" BACK_CODE_RGB, 4},
            {"test" BACK_CODE_RGB "test", 8},
            {STYLE_CODE_BRIGHT "test", 4},
            {"test" STYLE_CODE_UL, 4},
            {"test" STYLE_CODE_UL "test", 8},
            {
                FORE_CODE_RGB BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC "\ntest",
                5
            },
            {
                "\ntest" FORE_CODE_RGB BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC,
                5
            },
            {
                FORE_CODE_RGB " test " BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC "\ntest",
                11
            },
        };
        for_each(tests, i) {
            size_t length = colr_str_noncode_len(tests[i].s);
            asserteq(length, tests[i].expected, "Failed to count non-code-chars");
        }
    }
}
// colr_str_repr
subdesc(colr_str_repr) {
    it("escapes properly") {
        struct {
            char* input;
            char* expected;
        } tests[] = {
            {NULL, "NULL"},
            {"This\'", "\"This\\'\""},
            {"This\"", "\"This\\\"\""},
            {"This\?", "\"This\\?\""},
            {"This\\", "\"This\\\\\""},
            {"This\a", "\"This\\a\""},
            {"This\b", "\"This\\b\""},
            {"This\f", "\"This\\f\""},
            {"This\n", "\"This\\n\""},
            {"This\r", "\"This\\r\""},
            {"This\t", "\"This\\t\""},
            {"This\v", "\"This\\v\""},
            {
                "All\'together\"now\?\\\a\b\f\n\r\t\vokay.",
                "\"All\\'together\\\"now\\?\\\\\\a\\b\\f\\n\\r\\t\\vokay.\""
            },
            {"This\x1b[0m.", "\"This\\x1b[0m.\""},
        };
        for_each(tests, i) {
            char* repr = colr_repr(tests[i].input);
            asserteq(
                repr,
                tests[i].expected,
                "String was not escaped properly."
            );
            free(repr);
        }

    }
}
// colr_str_rjust.
subdesc(colr_str_rjust) {
    it("handles terminal width") {
        // Terminal width test.
        TermSize ts = colr_term_size();
        char* result = colr_str_rjust("test", 0, ' ');
        assert_not_null(result);
        assert_str_not_empty(result);
        assert(colr_str_starts_with(result, "  "));
        assert_str_contains(result, "test");
        assert_size_eq(strlen(result), ts.columns);
        free(result);
    }
    it("right-justifies non-escape-code strings") {
        struct {
            char* s;
            char padchar;
            int width;
            char* expected;
        } tests[] = {
            {NULL, 0, 4, NULL},
            {"", 0, 1, " "},
            {"", 0, 4, "    "},
            {"a", 0, 1, "a"},
            {"a", 0, 4, "   a"},
            {"aa", 0, 4, "  aa"},
            {"aaa", ' ', 4, " aaa"},
            {"aaaa  ", ' ', 10, "    aaaa  "},
            {FORE_CODE_BASIC "a", ' ', 4, "   " FORE_CODE_BASIC "a"},
            {FORE_CODE_EXT "a", ' ', 4, "   " FORE_CODE_EXT "a"},
            {FORE_CODE_RGB "a", ' ', 4, "   " FORE_CODE_RGB "a"},
            {STYLE_CODE_UL "a", ' ', 4, "   " STYLE_CODE_UL "a"},
            {"a" FORE_CODE_BASIC, ' ', 4, "   a" FORE_CODE_BASIC},
            {"a " FORE_CODE_EXT "a", ' ', 4, " a " FORE_CODE_EXT "a"},
            {
                " a" FORE_CODE_RGB FORE_CODE_BASIC "a",
                ' ',
                4,
                "  a" FORE_CODE_RGB FORE_CODE_BASIC "a"
            },
            {
                STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT,
                ' ',
                5,
                "    " STYLE_CODE_UL "a" FORE_CODE_BASIC FORE_CODE_EXT
            },

        };
        for_each(tests, i) {
            char* result = colr_str_rjust(tests[i].s, tests[i].width, tests[i].padchar);
            if (!result) {
                if (!tests[i].expected) {
                    // Expected null.
                    continue;
                }
                char* input_repr = colr_repr(tests[i].s);
                char* expected_repr = colr_repr(tests[i].expected);
                fail("Unexpected NULL from colr_str_rjust(%s): %s", input_repr, expected_repr);
            }
            assert_str_eq(result, tests[i].expected, "colr_str_rjust failed to justify.");
            free(result);
        }
    }
}
// colr_str_starts_with
subdesc(colr_str_starts_with) {
    it("recognizes string prefixes") {
        struct {
            char* s;
            char* prefix;
            bool expected;
        } tests[] = {
            // Null strings.
            {NULL, "a", false},
            {"test", NULL, false},
            {NULL, NULL, false},
            // Empty strings.
            {"", "", false},
            {"", "x", false},
            {"x", "", false},
            // Prefix too long.
            {"x", "xxx", false},
            {"abc", "abcdefg", false},
            // Common uses.
            {"lightblue", "light", true},
            {"xred", "x", true},
            {"yellow", "yel", true},
            {"!@#$^&*", "!@", true},
            {"    test", "    ", true},
            // Should not trigger a match.
            {"test", "a", false},
            {" test", "test", false},
            {"t", "apple", false},
        };
        for_each(tests, i) {
            asserteq(colr_str_starts_with(tests[i].s, tests[i].prefix), tests[i].expected);
        }
    }
}
// colr_str_strip_codes
subdesc(colr_str_strip_codes) {
    it("strips all escape codes") {
        struct {
            char* s;
            char* expected;
        } tests[] = {
            {NULL, NULL},
            {"", ""},
            {FORE_CODE_BASIC "test", "test"},
            {"test" FORE_CODE_BASIC, "test"},
            {"test" FORE_CODE_BASIC "test", "testtest"},
            {FORE_CODE_EXT "test", "test"},
            {"test" FORE_CODE_EXT, "test"},
            {"test" FORE_CODE_EXT "test", "testtest"},
            {FORE_CODE_RGB "test", "test"},
            {"test" FORE_CODE_RGB, "test"},
            {"test" FORE_CODE_RGB "test", "testtest"},
            {BACK_CODE_BASIC "test", "test"},
            {"test" BACK_CODE_BASIC, "test"},
            {"test" BACK_CODE_BASIC "test", "testtest"},
            {BACK_CODE_EXT "test", "test"},
            {"test" BACK_CODE_EXT, "test"},
            {"test" BACK_CODE_EXT "test", "testtest"},
            {BACK_CODE_RGB "test", "test"},
            {"test" BACK_CODE_RGB, "test"},
            {"test" BACK_CODE_RGB "test", "testtest"},
            {STYLE_CODE_BRIGHT "test", "test"},
            {"test" STYLE_CODE_UL, "test"},
            {"test" STYLE_CODE_UL "test", "testtest"},
            {
                FORE_CODE_RGB BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC "\ntest",
                "\ntest"
            },
            {
                "\ntest" FORE_CODE_RGB BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC,
                "\ntest"
            },
            {
                FORE_CODE_RGB " test " BACK_CODE_RGB STYLE_CODE_UL FORE_CODE_BASIC "\ntest",
                " test \ntest"
            },
        };
        for_each(tests, i) {
            char* stripped = colr_str_strip_codes(tests[i].s);
            if (!stripped) {
                // Expected failure.
                if (!tests[i].expected) continue;
                fail("Falsey returned NULL for a valid string!");
            }
            assert_str_eq(stripped, tests[i].expected, "Failed to strip codes");
            free(stripped);
        }
    }
}
// colr_str_to_lower
subdesc(colr_str_to_lower) {
    it("lowercases strings") {
        struct {
            char* s;
            char* expected;
        } tests[] = {
            {NULL, NULL},
            {"", ""},
            {"A", "a"},
            {"ABCDEFGHIJKLMNOP", "abcdefghijklmnop"},
            {"  TeSt  ", "  test  "}
        };
        for_each(tests, i) {
            char* result = colr_str_to_lower(tests[i].s);
            if (tests[i].expected) {
                assert(result != NULL, "Unexpected NULL from colr_str_to_lower()");
            } else {
                assert(result == NULL, "Expected NULL from colr_str_to_lower()");
                free(result);
                continue;
            }
            asserteq(result, tests[i].expected);
            free(result);
        }
    }
}
}
