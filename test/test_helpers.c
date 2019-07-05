/*! Tests for small helper functions.

    \author Christopher Welborn
    \date 06-29-2019
*/

#include "test_colr.h"


static void test_char_escape_char(void** state) {
    /*! Tests char_escape_char.
    */
    (void)state; // Unused.

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
        assert_true(char_escape_char(tests[i].input) == tests[i].expected);
    }
    // 65-90 inclusive == A-Z.
    for (char C = 65; C <= 90; C++) { // Hah.
        assert_true(char_escape_char(C) == C);
    }
    // 97-122 inclusive == a-z.
    for (char c = 97; c <= 122; c++) {
        assert_true(char_escape_char(c) == c);
    }
}

static void test_char_should_escape(void** state) {
    /*! Tests char_should_escape.
    */
    (void)state; // Unused.

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
        assert_true(char_should_escape(tests[i].input));
    }
    // 65-90 inclusive == A-Z.
    for (char C = 65; C <= 90; C++) { // Hah.
        assert_false(char_should_escape(C));
    }
    // 97-122 inclusive == a-z.
    for (char c = 97; c <= 122; c++) {
        assert_false(char_should_escape(c));
    }
}

static void test_colr_empty_str(void** state) {
    /*! Basic sanity check for colr_empty_str().
    */
    (void)state;

    char* s = colr_empty_str();
    assert_string_equal(s, "");
    free(s);
}

static void test_str_endswith(void** state) {
    /*! Tests str_endswith.
    */
    (void)state; // Unused.
    // Common uses.
    assert_true(str_endswith("lightblue", "blue"));
    assert_true(str_endswith("xred", "red"));
    assert_true(str_endswith("yellow", "low"));
    assert_true(str_endswith("!@#$^&*", "&*"));
    assert_true(str_endswith("    test    ", "    "));
    // Should not trigger a match.
    assert_false(str_endswith("test", "a"));
    assert_false(str_endswith(" test ", "test"));
    assert_false(str_endswith("t", "apple"));
    assert_false(str_endswith(NULL, "a"));
    assert_false(str_endswith("test", NULL));
    assert_false(str_endswith(NULL, NULL));
}

static void test_str_lower(void** state) {
    /*! str_lower() should lowercase strings.
    */
    (void)state;
    // Should not fail.
    str_lower(NULL);

    // Should not fail.
    char* empty = "";
    str_lower(empty);
    assert_string_equal(empty, "");
    char* allocempty = colr_empty_str();
    str_lower(allocempty);
    assert_string_equal(empty, "");

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
        assert_string_equal(input, tests[i].expected);
        free(input);
    }
}

static void test_str_repr(void** state) {
    /*! Ensure str_repr() escapes properly.
    */
    (void)state; // Unused.
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
        assert_string_equal(repr, tests[i].expected);
        free(repr);
    }
}

static void test_str_startswith(void** state) {
    /*! Tests str_startswith.
    */
    (void)state; // Unused.
    // Common uses.
    assert_true(str_startswith("lightblue", "light"));
    assert_true(str_startswith("xred", "x"));
    assert_true(str_startswith("yellow", "yel"));
    assert_true(str_startswith("!@#$^&*", "!@"));
    assert_true(str_startswith("    test", "    "));
    // Should not trigger a match.
    assert_false(str_startswith("test", "a"));
    assert_false(str_startswith(" test", "test"));
    assert_false(str_startswith("t", "apple"));
    assert_false(str_startswith(NULL, "a"));
    assert_false(str_startswith("test", NULL));
    assert_false(str_startswith(NULL, NULL));
}

int run_helper_tests(void) {
    return_cm_tests(
        helper,
        cm_test(test_char_escape_char),
        cm_test(test_char_should_escape),
        cm_test(test_colr_empty_str),
        cm_test(test_str_endswith),
        cm_test(test_str_lower),
        cm_test(test_str_repr),
        cm_test(test_str_startswith),
    );
}
