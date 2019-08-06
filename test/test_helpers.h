/*! Tests for helper functions.

    \author Christopher Welborn
    \date   07-27-2019
*/


#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H
#include "test_ColrC.h"

// Example escape codes, for use in helper functions (without calling colr functions).
#define FORE_CODE_BASIC "\x1b[31m"
#define FORE_CODE_EXT "\x1b[38;5;34m"
#define FORE_CODE_RGB "\x1b[38;2;255;255;255m"
#define BACK_CODE_BASIC "\x1b[41m"
#define BACK_CODE_EXT "\x1b[48;5;34m"
#define BACK_CODE_RGB "\x1b[48;2;255;255;255m"
#define STYLE_CODE_RESET "\x1b[0m"
#define STYLE_CODE_BRIGHT "\x1b[1m"
#define STYLE_CODE_UL "\x1b[4m"

#endif // TEST_HELPERS_H
