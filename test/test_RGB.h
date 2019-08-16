/*! Tests for the RGB struct, and it's related functions.

    \author Christopher Welborn
    \date 07-13-2019
*/
#ifndef TEST_RGB_H
#define TEST_RGB_H
#include "test_ColrC.h"

#define assert_RGB_eq(a, b) \
    do { \
        if (!RGB_eq(a, b)) { \
            char* _are_a = colr_repr(a); \
            char* _are_b = colr_repr(b); \
            defer(free(_are_a)); \
            defer(free(_are_b)); \
            fail("RGB values are not equal: %s != %s\n", _are_a, _are_b); \
        } \
    } while (0)

#define assert_rgb_from(s, func, expectedret, ...) \
    do { \
        int _arf_ret = func(s, __VA_ARGS__); \
        if (_arf_ret != expectedret) { \
            char* _arf_colr_str_repr = colr_repr(s); \
            defer(free(_arf_colr_str_repr)); \
            fail( \
                "Unexpected return value from: " #func "(%s, ..) == %d (not %d)\n", \
                _arf_colr_str_repr, \
                _arf_ret, \
                expectedret \
            ); \
        } \
    } while (0)

#define assert_rgb_from_either(s, func, expecteda, expectedb, ...) \
    do { \
        int _arfr_ret = func(s, __VA_ARGS__); \
        if ((_arfr_ret != expecteda) && (_arfr_ret != expectedb)) { \
            char* _arfr_colr_str_repr = colr_repr(s); \
            fail( \
                "Unexpected return value from: " #func "(%s, ..) == %d (not one of: %d, %d)\n", \
                _arfr_colr_str_repr, \
                _arfr_ret, \
                expecteda, \
                expectedb \
            ); \
        } \
    } while (0)


typedef struct {
    char* input;
    RGB rgb;
} rgb_test_item;

rgb_test_item hex_tests[] = {
    // With
    {"#ffffff", {255, 255, 255}},
    {"#ff00ff", {255, 0, 255}},
    {"#ffff00", {255, 255, 0}},
    {"#010101", {1, 1, 1}},
    // Without
    {"ffffff", {255, 255, 255}},
    {"ff00ff", {255, 0, 255}},
    {"ffff00", {255, 255, 0}},
    {"010101", {1, 1, 1}},
};
size_t hex_tests_len = array_length(hex_tests);

char* bad_hex_tests[] = {
    NULL,
    "",
    "x",
    "01",
    "01g",
    "ghijkl",
    "##fff",
    "###fff",
};
size_t bad_hex_tests_len = array_length(bad_hex_tests);

rgb_test_item str_tests[] = {
    {"255,255,255", {255, 255, 255}},
    {"255,0,255", {255, 0, 255}},
    {"255,255,0", {255, 255, 0}},
    {"255:255:255", {255, 255, 255}},
    {"255:0:255", {255, 0, 255}},
    {"255:255:0", {255, 255, 0}},
    {"255;255;255", {255, 255, 255}},
    {"255;0;255", {255, 0, 255}},
    {"255;255;0", {255, 255, 0}},
    // Hex strings are okay, as long as they start with #.
    {"#FFFFFF", {255, 255, 255}},
    {"#FF00FF", {255, 0, 255}},
    {"#FFFF00", {255, 255, 0}},
    // Short-form is okay, if it starts with #.
    {"#FFF", {255, 255, 255}},
    {"#F0F", {255, 0, 255}},
    {"#FF0", {255, 255, 0}},
};

size_t str_tests_len = array_length(str_tests);

char* bad_str_tests[] = {
    NULL,
    "",
    "aa,bb,cc",
    "234",
    "#34",
    "#ghijkl",
    "355;1024;2048",
    "-1:-2:-3",
    "0,1:2",
    "my_bad_rgb_string"
};

size_t bad_str_tests_len = array_length(bad_str_tests);
#endif // TEST_RGB_H
