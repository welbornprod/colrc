/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_helpers.h"

describe(helpers) {
// char_escape_char
subdesc(char_escape_char) {
    it("should recognize valid escape sequence chars") {
        struct {
            char input;
            char expected;
        } tests[] = {
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
                char_escape_char(tests[i].input),
                tests[i].expected,
                "Known escape char was not escaped."
            );
        }
    }
    it("should not escape regular ascii chars") {
        // 65-90 inclusive == A-Z.
        for (char C = 65; C <= 90; C++) { // Hah.
            asserteq(
                char_escape_char(C),
                C,
                "Known non-escape char was escaped."
            );
        }
        // 97-122 inclusive == a-z.
        for (char c = 97; c <= 122; c++) {
            asserteq(
                char_escape_char(c),
                c,
                "Known non-escape char was escaped."
            );
        }
    }
}
// char_in_str
subdesc(char_in_str) {
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
                char_in_str(tests[i].c, tests[i].s),
                tests[i].expected,
                "Known char was not detected."
            );
        }
    }
}
// char_should_escape
subdesc(char_should_escape) {
    it("should detect valid escape sequence chars") {
        char tests[] = {
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
                char_should_escape(tests[i]),
                "Known escape char returned false."
            );
        }
    }
    it("should not produce false-positives") {
        // 65-90 inclusive == A-Z.
        for (char C = 65; C <= 90; C++) { // Hah.
            assert(
                !char_should_escape(C),
                "Known non-escape char returned true."
            );
        }
        // 97-122 inclusive == a-z.
        for (char c = 97; c <= 122; c++) {
            assert(
                !char_should_escape(c),
                "Known non-escape char returned true."
            );
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
// str_append_reset
subdesc(str_append_reset) {
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
            str_copy(s, tests[i].input, strlen(tests[i].input));
            str_append_reset(s);
            char* input_repr = str_repr(tests[i].input);
            char* input_msg;
            asprintf_or(&input_msg, "str_append_reset(%s) failed", input_repr) {
                fail("Allocation failed for failure message!");
            }
            free(input_repr);
            assert_str_eq(
                s,
                tests[i].expected,
                input_msg
            );
            free(input_msg);
        }
    }
}
// str_char_count
subdesc(str_char_count) {
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
            asserteq(str_char_count(tests[i].input, tests[i].c), tests[i].expected);
        }
    }
}
// str_ends_with
subdesc(str_ends_with) {
    it("detects string endings") {
        // Common uses.
        assert(
            str_ends_with("lightblue", "blue"),
            "Known suffix was not detected."
        );
        assert(
            str_ends_with("xred", "red"),
            "Known suffix was not detected."
        );
        assert(
            str_ends_with("yellow", "low"),
            "Known suffix was not detected."
        );
        assert(
            str_ends_with("!@#$^&*", "&*"),
            "Known suffix was not detected."
        );
        assert(
            str_ends_with("    test    ", "    "),
            "Known suffix was not detected."
        );
        // Should not trigger a match.
        assert(
            !str_ends_with("test", "a"),
            "Bad suffix was falsey detected."
        );
        assert(
            !str_ends_with(" test ", "test"),
            "Bad suffix was falsey detected."
        );
        assert(
            !str_ends_with("t", "apple"),
            "Bad suffix was falsey detected."
        );
        assert(
            !str_ends_with(NULL, "a"),
            "Null argument did not return false."
        );
        assert(
            !str_ends_with("test", NULL),
            "Null argument did not return false."
        );
        assert(
            !str_ends_with(NULL, NULL),
            "Null arguments did not return false."
        );
    }
}
// str_has_codes
subdesc(str_has_codes) {
    it("should detect escape codes") {
        // NULL should just return false.
        assert(str_has_codes(NULL) == false);

        // Normal strings should not trigger this.
        assert(str_has_codes("This is a string.") == false);

        // Empty strings should not trigger this.
        assert(str_has_codes("") == false);

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
            char* s = colr("This prefix.", args[i], "This suffix.");
            assert(str_has_codes(s));
            free(s);
        }
    }
}
// str_is_all
subdesc(str_is_all) {
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
            asserteq(str_is_all(tests[i].s, tests[i].c), tests[i].expected);
        }
    }
}
// str_is_digits
subdesc(str_is_digits) {
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
            asserteq(str_is_digits(tests[i].s), tests[i].expected);
        }
    }
}
// str_lower
subdesc(str_lower) {
    it("should handle empty strings") {
        // Should not fail.
        str_lower(NULL);

        // Should not fail.
        char* empty = "";
        str_lower(empty);
        asserteq(
            empty,
            "",
            "Empty string did not return empty string."
        );
        char* allocempty = colr_empty_str();
        str_lower(allocempty);
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
            char* input;
            asprintf_or(&input, "%s", tests[i].input) {
                fail("Allocation failed for test input string!");
            }
            str_lower(input);
            asserteq(
                input,
                tests[i].expected,
                "String was not lowered."
            );
            free(input);
        }
    }
}
// str_lstrip_chars
subdesc(str_lstrip_chars) {
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
            char* result = str_lstrip_chars(tests[i].s, tests[i].chars);
            if (!result) {
                if (tests[i].expected) {
                    fail(
                        "Falsely returned NULL: str_lstrip_chars(%s, %s)\n",
                        colr_repr(tests[i].s),
                        colr_repr(tests[i].chars)
                    );
                } else {
                    // expected failure.
                    continue;
                }
            }
            asserteq(result, tests[i].expected);
            free(result);
        }
    }
}
// str_repr
subdesc(str_repr) {
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
            {"This\033[0m.", "\"This\\033[0m.\""},
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
// str_starts_with
subdesc(str_starts_with) {
    it("recognizes string prefixes") {
        struct {
            char* s;
            char* prefix;
            bool expected;
        } tests[] = {
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
            {NULL, "a", false},
            {"test", NULL, false},
            {NULL, NULL, false},
        };
        for_each(tests, i) {
            asserteq(str_starts_with(tests[i].s, tests[i].prefix), tests[i].expected);
        }
    }
}
// str_strip_codes
subdesc(str_strip_codes) {
    it("strips all escape codes") {
        struct {
            char* s;
            char* expected;
        } tests[] = {
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
            char* stripped = str_strip_codes(tests[i].s);
            assert_str_eq(stripped, tests[i].expected, "Failed to strip codes");
            free(stripped);
        }
    }
}
// str_to_lower
subdesc(str_to_lower) {
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
            char* result = str_to_lower(tests[i].s);
            if (tests[i].expected) {
                assert(result != NULL, "Unexpected NULL from str_to_lower()");
            } else {
                assert(result == NULL, "Expected NULL from str_to_lower()");
                free(result);
                continue;
            }
            asserteq(result, tests[i].expected);
            free(result);
        }
    }
}
}
// TODO: Tests for str_to_wide, wide_to_str.
