/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_ColrC.h"

describe(helper_functions) {
    subdesc(char_escape_char) {
        it("should recognize valid escape sequence chars") {
            struct TestItem {
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
            size_t tests_len = array_length(tests);
            for (size_t i = 0; i < tests_len; i++) {
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

    subdesc(char_should_escape) {
        it("should detect valid escape sequence chars") {
            struct TestItem {
                char input;
            } tests[] = {
                {'\''},
                {'\"'},
                {'\?'},
                {'\\'},
                {'\a'},
                {'\b'},
                {'\f'},
                {'\n'},
                {'\r'},
                {'\t'},
                {'\v'},
            };
            size_t tests_len = array_length(tests);
            for (size_t i = 0; i < tests_len; i++) {
                assert(
                    char_should_escape(tests[i].input),
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

    subdesc(colr_empty_str) {
        it("sanity check for colr_empty_str()") {
            char* s = colr_empty_str();
            asserteq(s, "", "Empty string was not equal to \"\".");
            free(s);
        }
    }

    subdesc(str_endswith) {
        it("str_endswith") {
            // Common uses.
            assert(
                str_endswith("lightblue", "blue"),
                "Known suffix was not detected."
            );
            assert(
                str_endswith("xred", "red"),
                "Known suffix was not detected."
            );
            assert(
                str_endswith("yellow", "low"),
                "Known suffix was not detected."
            );
            assert(
                str_endswith("!@#$^&*", "&*"),
                "Known suffix was not detected."
            );
            assert(
                str_endswith("    test    ", "    "),
                "Known suffix was not detected."
            );
            // Should not trigger a match.
            assert(
                !str_endswith("test", "a"),
                "Bad suffix was falsey detected."
            );
            assert(
                !str_endswith(" test ", "test"),
                "Bad suffix was falsey detected."
            );
            assert(
                !str_endswith("t", "apple"),
                "Bad suffix was falsey detected."
            );
            assert(
                !str_endswith(NULL, "a"),
                "Null argument did not return false."
            );
            assert(
                !str_endswith("test", NULL),
                "Null argument did not return false."
            );
            assert(
                !str_endswith(NULL, NULL),
                "Null arguments did not return false."
            );
        }
    }

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
            struct TestItem {
                char* input;
                char* expected;
            } tests[] = {
                {"THIS IS IT.", "this is it."},
                {"mAcRoS aRe eViL!?%%$!", "macros are evil!?%%$!"},
            };

            size_t tests_len = array_length(tests);
            for (size_t i = 0; i < tests_len; i++) {
                char* input;
                asprintf(&input, "%s", tests[i].input);
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

    subdesc(str_repr) {
        it("escapes properly") {
            struct TestItem {
                char* input;
                char* expected;
            } tests[] = {
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
            };
            size_t tests_len = array_length(tests);
            for (size_t i = 0; i < tests_len; i++) {
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

    subdesc(str_startswith) {
        it("recognizes string prefixes") {
            // Common uses.
            assert(
                str_startswith("lightblue", "light"),
                "Known prefix was not detected."
            );
            assert(
                str_startswith("xred", "x"),
                "Known prefix was not detected."
            );
            assert(
                str_startswith("yellow", "yel"),
                "Known prefix was not detected."
            );
            assert(
                str_startswith("!@#$^&*", "!@"),
                "Known prefix was not detected."
            );
            assert(
                str_startswith("    test", "    "),
                "Known prefix was not detected."
            );
            // Should not trigger a match.
            assert(
                !str_startswith("test", "a"),
                "Bad prefix was falsey detected."
            );
            assert(
                !str_startswith(" test", "test"),
                "Bad prefix was falsey detected."
            );
            assert(
                !str_startswith("t", "apple"),
                "Bad prefix was falsey detected."
            );
            assert(
                !str_startswith(NULL, "a"),
                "Null argument should not return true."
            );
            assert(
                !str_startswith("test", NULL),
                "Null argument should not return true."
            );
            assert(
                !str_startswith(NULL, NULL),
                "Null arguments should not return true."
            );
        }
    }
}
