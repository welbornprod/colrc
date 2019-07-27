/*! Tests for helper functions.

    \author Christopher Welborn
    \date   07-27-2019
*/


#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H
#include "test_ColrC.h"

// Example escape codes, for use in helper functions (without calling colr functions).
#define FORE_CODE_BASIC "\033[31m"
#define FORE_CODE_EXT "\033[38;5;34m"
#define FORE_CODE_RGB "\033[38;2;255;255;255m"
#define BACK_CODE_BASIC "\033[41m"
#define BACK_CODE_EXT "\033[48;5;34m"
#define BACK_CODE_RGB "\033[48;2;255;255;255m"
#define STYLE_CODE_RESET "\033[0m"
#define STYLE_CODE_BRIGHT "\033[1m"
#define STYLE_CODE_UL "\033[4m"

#endif // TEST_HELPERS_H
