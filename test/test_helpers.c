/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_ColrC.h"

describe(helper_functions) {
// char_escape_char
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
// char_should_escape
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
// colr_empty_str
    subdesc(colr_empty_str) {
        it("sanity check for colr_empty_str()") {
            char* s = colr_empty_str();
            asserteq(s, "", "Empty string was not equal to \"\".");
            free(s);
        }
    }
// str_endswith
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
            struct ColorArg* args[] = {
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
// str_repr
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
// str_startswith
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
