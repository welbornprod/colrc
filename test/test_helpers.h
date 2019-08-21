/*! Tests for helper functions.

    \author Christopher Welborn
    \date   07-27-2019
*/


#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H
#include "test_ColrC.h"

// Example escape codes, for use in helper functions (without calling colr functions).
#define FORE_CODE_BASIC "\x1b[31m"
#define FORE_CODE_BASIC_LEN strlen(FORE_CODE_BASIC)
#define FORE_CODE_EXT "\x1b[38;5;34m"
#define FORE_CODE_EXT_LEN strlen(FORE_CODE_EXT)
#define FORE_CODE_RGB "\x1b[38;2;255;255;255m"
#define FORE_CODE_RGB_LEN strlen(FORE_CODE_RGB)
#define BACK_CODE_BASIC "\x1b[41m"
#define BACK_CODE_BASIC_LEN strlen(BACK_CODE_BASIC)
#define BACK_CODE_EXT "\x1b[48;5;34m"
#define BACK_CODE_EXT_LEN strlen(BACK_CODE_EXT)
#define BACK_CODE_RGB "\x1b[48;2;255;255;255m"
#define BACK_CODE_RGB_LEN strlen(BACK_CODE_RGB)
#define STYLE_CODE_RESET "\x1b[0m"
#define STYLE_CODE_RESET_LEN strlen(STYLE_CODE_RESET)
#define STYLE_CODE_BRIGHT "\x1b[1m"
#define STYLE_CODE_BRIGHT_LEN strlen(STYLE_CODE_BRIGHT)
#define STYLE_CODE_UL "\x1b[4m"
#define STYLE_CODE_UL_LEN strlen(STYLE_CODE_UL)
#endif // TEST_HELPERS_H
