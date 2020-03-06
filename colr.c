/*! \file colr.c
    Implements everything in the colr.h header.

    \internal
    \author Christopher Welborn
    \date 06-22-2019
    \endinternal
*/
#include "colr.h"

/*! Integer to test for the presence of the "escaped output modifier" in
    colr_printf_handler. It is used to trigger "escaped output mode" when
    printing ColrC objects, where the color codes are escaped so you can see
    what they look like (instead of affecting the terminal).

    \details
    The character used as the "escaped output modifier" is COLR_FMT_MOD_ESC,
    from colr.h.

    \warning
    This is for ColrC only. You should have no reason to use or modify
    this variable.

    \details
    This is set in colr_printf_register when the modifier is registered.
    On a successful call to register_printf_modifier, it will be a positive
    number representing the bit set in the USER field in 'struct printf_info'.
    So later on, in colr_printf_handler():
    \code
        using_escape_modifier = (info->user & colr_printf_esc_mod);
    \endcode
*/
int colr_printf_esc_mod = 0;

//! An array of BasicInfo items, used with BasicValue_from_str().
const BasicInfo basic_names[] = {
    {"reset", RESET},
    {"none", RESET},
    {"black", BLACK},
    {"blue", BLUE},
    {"cyan", CYAN},
    {"green", GREEN},
    {"magenta", MAGENTA},
    {"red", RED},
    {"white", WHITE},
    {"normal", WHITE},
    {"yellow", YELLOW},
    {"lightblack", LIGHTBLACK},
    {"lightblue", LIGHTBLUE},
    {"lightcyan", LIGHTCYAN},
    {"lightgreen", LIGHTGREEN},
    {"lightmagenta", LIGHTMAGENTA},
    {"lightred", LIGHTRED},
    {"lightwhite", LIGHTWHITE},
    {"lightnormal", LIGHTWHITE},
    {"lightyellow", LIGHTYELLOW},
    {NULL, RESET},
};

//! Length of usable values basic_names.
const size_t basic_names_len = (sizeof(basic_names) / sizeof(basic_names[0])) - 1;

//! An array of ExtendedInfo, used with ExtendedValue_from_str().
const ExtendedInfo extended_names[] = {
    {"xred", XRED},
    {"xgreen", XGREEN},
    {"xyellow", XYELLOW},
    {"xblue", XBLUE},
    {"xmagenta", XMAGENTA},
    {"xcyan", XCYAN},
    {"xwhite", XWHITE},
    {"xnormal", XWHITE},
    {"xlightred", XLIGHTRED},
    {"xlightgreen", XLIGHTGREEN},
    {"xlightyellow", XLIGHTYELLOW},
    {"xlightblack", XLIGHTBLACK},
    {"xlightblue", XLIGHTBLUE},
    {"xlightmagenta", XLIGHTMAGENTA},
    {"xlightwhite", XLIGHTWHITE},
    {"xlightnormal", XLIGHTWHITE},
    {"xlightcyan", XLIGHTCYAN},
    {NULL, RESET},
};

//! Length of usable values in extended_names.
const size_t extended_names_len = (sizeof(extended_names) / sizeof(extended_names[0])) - 1;

//! An array of StyleInfo items, used with StyleName_from_str().
const StyleInfo style_names[] = {
    // Using "reset" as the default style name for all RESET_ALL aliases.
    // Whichever name is first in the list is the "default" for that style.
    {"reset", RESET_ALL},
    {"none", RESET_ALL},
    {"resetall", RESET_ALL},
    {"reset-all", RESET_ALL},
    {"reset_all", RESET_ALL},
    {"bold", BRIGHT},
    {"bright", BRIGHT},
    {"dim", DIM},
    {"italic", ITALIC},
    {"underline", UNDERLINE},
    {"flash", FLASH},
    {"highlight", HIGHLIGHT},
    {"normal", NORMAL},
    {"strikethru", STRIKETHRU},
    {"strike", STRIKETHRU},
    {"strikethrough", STRIKETHRU},
    {"frame", FRAME},
    {"encircle", ENCIRCLE},
    {"circle", ENCIRCLE},
    {"overline", OVERLINE},
    {NULL, RESET_ALL},
};

//! Length of usable values in style_names.
const size_t style_names_len = (sizeof(style_names) / sizeof(style_names[0])) - 1;

/*! A map from ExtendedValue (256-color) to RGB value, where the index is the
    is the ExtendedValue, and the value is the RGB.

    \details
    This is used in several RGB/ExtendedValue functions.

    \sa ExtendedValue_from_RGB
    \sa RGB_to_term_RGB

    \examplecodefor{ext2rgb_map,.c}
    // Fast map an ExtendedValue to an RGB value.
    ExtendedValue eval = 9; // 9 happens to be XRED (255;85;85 in RGB).
    RGB rgbval = ext2rgb_map[eval];

    // The result from ExtendedValue_from_RGB should always match the map.
    assert(ExtendedValue_from_RGB(rgbval) == eval);

    \endexamplecode
*/
const RGB ext2rgb_map[] = {
    // Primary 3-bit colors (8 colors, 0-7)
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {255, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {192, 192, 192},
    // "Bright" versions of the original 8 colors (8-15).
    {128, 128, 128},
    {255, 85, 85},
    {135, 255, 135},
    {255, 255, 215},
    {175, 215, 215},
    {255, 85, 255},
    {215, 255, 255},
    {255, 255, 255},
    // Strictly ascending.
    {0, 0, 0},
    {0, 0, 95},
    {0, 0, 135},
    {0, 0, 175},
    {0, 0, 215},
    {0, 0, 255},
    {0, 95, 0},
    {0, 95, 95},
    {0, 95, 135},
    {0, 95, 175},
    {0, 95, 215},
    {0, 95, 255},
    {0, 135, 0},
    {0, 135, 95},
    {0, 135, 135},
    {0, 135, 175},
    {0, 135, 215},
    {0, 135, 255},
    {0, 175, 0},
    {0, 175, 95},
    {0, 175, 135},
    {0, 175, 175},
    {0, 175, 215},
    {0, 175, 255},
    {0, 215, 0},
    {0, 215, 95},
    {0, 215, 135},
    {0, 215, 175},
    {0, 215, 215},
    {0, 215, 255},
    {0, 255, 0},
    {0, 255, 95},
    {0, 255, 135},
    {0, 255, 175},
    {0, 255, 215},
    {0, 255, 255},
    {95, 0, 0},
    {95, 0, 95},
    {95, 0, 135},
    {95, 0, 175},
    {95, 0, 215},
    {95, 0, 255},
    {95, 95, 0},
    {95, 95, 95},
    {95, 95, 135},
    {95, 95, 175},
    {95, 95, 215},
    {95, 95, 255},
    {95, 135, 0},
    {95, 135, 95},
    {95, 135, 135},
    {95, 135, 175},
    {95, 135, 215},
    {95, 135, 255},
    {95, 175, 0},
    {95, 175, 95},
    {95, 175, 135},
    {95, 175, 175},
    {95, 175, 215},
    {95, 175, 255},
    {95, 215, 0},
    {95, 215, 95},
    {95, 215, 135},
    {95, 215, 175},
    {95, 215, 215},
    {95, 215, 255},
    {95, 255, 0},
    {95, 255, 95},
    {95, 255, 135},
    {95, 255, 175},
    {95, 255, 215},
    {95, 255, 255},
    {135, 0, 0},
    {135, 0, 95},
    {135, 0, 135},
    {135, 0, 175},
    {135, 0, 215},
    {135, 0, 255},
    {135, 95, 0},
    {135, 95, 95},
    {135, 95, 135},
    {135, 95, 175},
    {135, 95, 215},
    {135, 95, 255},
    {135, 135, 0},
    {135, 135, 95},
    {135, 135, 135},
    {135, 135, 175},
    {135, 135, 215},
    {135, 135, 255},
    {135, 175, 0},
    {135, 175, 95},
    {135, 175, 135},
    {135, 175, 175},
    {135, 175, 215},
    {135, 175, 255},
    {135, 215, 0},
    {135, 215, 95},
    {135, 215, 135},
    {135, 215, 175},
    {135, 215, 215},
    {135, 215, 255},
    {135, 255, 0},
    {135, 255, 95},
    {135, 255, 135},
    {135, 255, 175},
    {135, 255, 215},
    {135, 255, 255},
    {175, 0, 0},
    {175, 0, 95},
    {175, 0, 135},
    {175, 0, 175},
    {175, 0, 215},
    {175, 0, 255},
    {175, 95, 0},
    {175, 95, 95},
    {175, 95, 135},
    {175, 95, 175},
    {175, 95, 215},
    {175, 95, 255},
    {175, 135, 0},
    {175, 135, 95},
    {175, 135, 135},
    {175, 135, 175},
    {175, 135, 215},
    {175, 135, 255},
    {175, 175, 0},
    {175, 175, 95},
    {175, 175, 135},
    {175, 175, 175},
    {175, 175, 215},
    {175, 175, 255},
    {175, 215, 0},
    {175, 215, 95},
    {175, 215, 135},
    {175, 215, 175},
    {175, 215, 215},
    {175, 215, 255},
    {175, 255, 0},
    {175, 255, 95},
    {175, 255, 135},
    {175, 255, 175},
    {175, 255, 215},
    {175, 255, 255},
    {215, 0, 0},
    {215, 0, 95},
    {215, 0, 135},
    {215, 0, 175},
    {215, 0, 215},
    {215, 0, 255},
    {215, 95, 0},
    {215, 95, 95},
    {215, 95, 135},
    {215, 95, 175},
    {215, 95, 215},
    {215, 95, 255},
    {215, 135, 0},
    {215, 135, 95},
    {215, 135, 135},
    {215, 135, 175},
    {215, 135, 215},
    {215, 135, 255},
    {215, 175, 0},
    {215, 175, 95},
    {215, 175, 135},
    {215, 175, 175},
    {215, 175, 215},
    {215, 175, 255},
    {215, 215, 0},
    {215, 215, 95},
    {215, 215, 135},
    {215, 215, 175},
    {215, 215, 215},
    {215, 215, 255},
    {215, 255, 0},
    {215, 255, 95},
    {215, 255, 135},
    {215, 255, 175},
    {215, 255, 215},
    {215, 255, 255},
    {255, 0, 0},
    {255, 0, 95},
    {255, 0, 135},
    {255, 0, 175},
    {255, 0, 215},
    {255, 0, 255},
    {255, 95, 0},
    {255, 95, 95},
    {255, 95, 135},
    {255, 95, 175},
    {255, 95, 215},
    {255, 95, 255},
    {255, 135, 0},
    {255, 135, 95},
    {255, 135, 135},
    {255, 135, 175},
    {255, 135, 215},
    {255, 135, 255},
    {255, 175, 0},
    {255, 175, 95},
    {255, 175, 135},
    {255, 175, 175},
    {255, 175, 215},
    {255, 175, 255},
    {255, 215, 0},
    {255, 215, 95},
    {255, 215, 135},
    {255, 215, 175},
    {255, 215, 215},
    {255, 215, 255},
    {255, 255, 0},
    {255, 255, 95},
    {255, 255, 135},
    {255, 255, 175},
    {255, 255, 215},
    {255, 255, 255},
    {8, 8, 8},
    {18, 18, 18},
    {28, 28, 28},
    {38, 38, 38},
    {48, 48, 48},
    {58, 58, 58},
    {68, 68, 68},
    {78, 78, 78},
    {88, 88, 88},
    {98, 98, 98},
    {108, 108, 108},
    {118, 118, 118},
    {128, 128, 128},
    {138, 138, 138},
    {148, 148, 148},
    {158, 158, 158},
    {168, 168, 168},
    {178, 178, 178},
    {188, 188, 188},
    {198, 198, 198},
    {208, 208, 208},
    {218, 218, 218},
    {228, 228, 228},
    {238, 238, 238},
};

//! Length of ext2rgb_map  (should always be 256).
static_assert(
    (sizeof(ext2rgb_map) / sizeof(ext2rgb_map[0])) == 256,
    "ExtendedValue->RGB map is not 1:1, should only contain indexes 0-255!"
);
const size_t ext2rgb_map_len = sizeof(ext2rgb_map) / sizeof(ext2rgb_map[0]);

//! An array that holds a known color name, it's ExtendedValue, and it's RGB value.
const ColorNameData colr_name_data[] = {
    {"aliceblue", 231, {255, 255, 255}},
    {"antiquewhite", 230, {255, 255, 215}},
    {"antiquewhite2", 224, {255, 215, 215}},
    {"antiquewhite3", 181, {215, 175, 175}},
    {"antiquewhite4", 102, {135, 135, 135}},
    {"aquamarine", 122, {135, 255, 215}},
    {"aquamarine2", 79, {95, 215, 175}},
    {"aquamarine3", 66, {95, 135, 135}},
    {"azure", 231, {255, 255, 255}},
    {"azure2", 195, {215, 255, 255}},
    {"azure3", 152, {175, 215, 215}},
    {"azure4", 102, {135, 135, 135}},
    {"beige", 230, {255, 255, 215}},
    {"bisque", 224, {255, 215, 215}},
    {"bisque2", 223, {255, 215, 175}},
    {"bisque3", 181, {215, 175, 175}},
    {"bisque4", 101, {135, 135, 95}},
    {"black", 16, {1, 1, 1}},
    {"blanchedalmond", 230, {255, 255, 215}},
    {"blue", 4, {0, 0, 255}},
    {"blue2", 20, {0, 0, 215}},
    {"blue3", 18, {0, 0, 135}},
    {"blueviolet", 92, {135, 0, 215}},
    {"brown", 124, {175, 0, 0}},
    {"brown2", 203, {255, 95, 95}},
    {"brown3", 167, {215, 95, 95}},
    {"brown4", 88, {135, 0, 0}},
    {"burlywood", 180, {215, 175, 135}},
    {"burlywood2", 223, {255, 215, 175}},
    {"burlywood3", 222, {255, 215, 135}},
    {"burlywood4", 180, {215, 175, 135}},
    {"burlywood5", 101, {135, 135, 95}},
    {"cadetblue", 73, {95, 175, 175}},
    {"cadetblue2", 123, {135, 255, 255}},
    {"cadetblue3", 117, {135, 215, 255}},
    {"cadetblue4", 116, {135, 215, 215}},
    {"cadetblue5", 66, {95, 135, 135}},
    {"chartreuse", 118, {135, 255, 0}},
    {"chartreuse2", 76, {95, 215, 0}},
    {"chartreuse3", 64, {95, 135, 0}},
    {"chocolate", 166, {215, 95, 0}},
    {"chocolate2", 208, {255, 135, 0}},
    {"chocolate3", 166, {215, 95, 0}},
    {"chocolate4", 94, {135, 95, 0}},
    {"coral", 203, {255, 95, 95}},
    {"coral2", 209, {255, 135, 95}},
    {"coral3", 167, {215, 95, 95}},
    {"coral4", 94, {135, 95, 0}},
    {"cornflowerblue", 69, {95, 135, 255}},
    {"cornsilk", 230, {255, 255, 215}},
    {"cornsilk2", 224, {255, 215, 215}},
    {"cornsilk3", 187, {215, 215, 175}},
    {"cornsilk4", 102, {135, 135, 135}},
    {"cyan", 6, {0, 255, 255}},
    {"cyan2", 44, {0, 215, 215}},
    {"cyan3", 30, {0, 135, 135}},
    {"darkblue", 18, {0, 0, 135}},
    {"darkcyan", 30, {0, 135, 135}},
    {"darkgoldenrod", 136, {175, 135, 0}},
    {"darkgoldenrod2", 214, {255, 175, 0}},
    {"darkgoldenrod3", 172, {215, 135, 0}},
    {"darkgoldenrod4", 94, {135, 95, 0}},
    {"darkgray", 145, {175, 175, 175}},
    {"darkgreen", 22, {0, 95, 0}},
    {"darkgrey", 145, {175, 175, 175}},
    {"darkkhaki", 143, {175, 175, 95}},
    {"darkmagenta", 90, {135, 0, 135}},
    {"darkolivegreen", 58, {95, 95, 0}},
    {"darkolivegreen2", 191, {215, 255, 95}},
    {"darkolivegreen3", 155, {175, 255, 95}},
    {"darkolivegreen4", 149, {175, 215, 95}},
    {"darkolivegreen5", 65, {95, 135, 95}},
    {"darkorange", 208, {255, 135, 0}},
    {"darkorange2", 166, {215, 95, 0}},
    {"darkorange3", 94, {135, 95, 0}},
    {"darkorchid", 98, {135, 95, 215}},
    {"darkorchid2", 135, {175, 95, 255}},
    {"darkorchid3", 98, {135, 95, 215}},
    {"darkorchid4", 54, {95, 0, 135}},
    {"darkred", 88, {135, 0, 0}},
    {"darksalmon", 174, {215, 135, 135}},
    {"darkseagreen", 108, {135, 175, 135}},
    {"darkseagreen2", 157, {175, 255, 175}},
    {"darkseagreen3", 151, {175, 215, 175}},
    {"darkseagreen4", 65, {95, 135, 95}},
    {"darkslateblue", 60, {95, 95, 135}},
    {"darkslategray", 23, {0, 95, 95}},
    {"darkslategray2", 123, {135, 255, 255}},
    {"darkslategray3", 116, {135, 215, 215}},
    {"darkslategray4", 66, {95, 135, 135}},
    {"darkslategrey", 23, {0, 95, 95}},
    {"darkturquoise", 44, {0, 215, 215}},
    {"darkviolet", 92, {135, 0, 215}},
    {"debianred", 161, {215, 0, 95}},
    {"deeppink", 198, {255, 0, 135}},
    {"deeppink2", 162, {215, 0, 135}},
    {"deeppink3", 89, {135, 0, 95}},
    {"deepskyblue", 39, {0, 175, 255}},
    {"deepskyblue2", 32, {0, 135, 215}},
    {"deepskyblue3", 24, {0, 95, 135}},
    {"dimgrey", 59, {95, 95, 95}},
    {"dodgerblue", 33, {0, 135, 255}},
    {"dodgerblue2", 32, {0, 135, 215}},
    {"dodgerblue3", 24, {0, 95, 135}},
    {"firebrick", 124, {175, 0, 0}},
    {"firebrick2", 203, {255, 95, 95}},
    {"firebrick3", 160, {215, 0, 0}},
    {"firebrick4", 88, {135, 0, 0}},
    {"floralwhite", 231, {255, 255, 255}},
    {"forestgreen", 28, {0, 135, 0}},
    {"gainsboro", 188, {215, 215, 215}},
    {"ghostwhite", 231, {255, 255, 255}},
    {"gold", 220, {255, 215, 0}},
    {"gold2", 178, {215, 175, 0}},
    {"gold3", 100, {135, 135, 0}},
    {"goldenrod", 178, {215, 175, 0}},
    {"goldenrod2", 214, {255, 175, 0}},
    {"goldenrod3", 178, {215, 175, 0}},
    {"goldenrod4", 94, {135, 95, 0}},
    {"gray", 145, {175, 175, 175}},
    {"gray100", 231, {255, 255, 255}},
    {"gray37", 59, {95, 95, 95}},
    {"gray50", 102, {135, 135, 135}},
    {"gray59", 102, {135, 135, 135}},
    {"green", 2, {0, 255, 0}},
    {"green2", 40, {0, 215, 0}},
    {"green3", 28, {0, 135, 0}},
    {"greenyellow", 154, {175, 255, 0}},
    {"grey", 145, {175, 175, 175}},
    {"grey100", 231, {255, 255, 255}},
    {"grey37", 59, {95, 95, 95}},
    {"grey50", 102, {135, 135, 135}},
    {"grey59", 102, {135, 135, 135}},
    {"honeydew2", 194, {215, 255, 215}},
    {"honeydew3", 151, {175, 215, 175}},
    {"honeydew4", 102, {135, 135, 135}},
    {"hotpink", 205, {255, 95, 175}},
    {"hotpink2", 168, {215, 95, 135}},
    {"hotpink3", 95, {135, 95, 95}},
    {"indianred", 167, {215, 95, 95}},
    {"indianred2", 203, {255, 95, 95}},
    {"indianred3", 167, {215, 95, 95}},
    {"indianred4", 95, {135, 95, 95}},
    {"ivory", 231, {255, 255, 255}},
    {"ivory2", 230, {255, 255, 215}},
    {"ivory3", 187, {215, 215, 175}},
    {"ivory4", 102, {135, 135, 135}},
    {"khaki", 222, {255, 215, 135}},
    {"khaki2", 228, {255, 255, 135}},
    {"khaki3", 186, {215, 215, 135}},
    {"khaki4", 101, {135, 135, 95}},
    {"lavender", 189, {215, 215, 255}},
    {"lavenderblush", 231, {255, 255, 255}},
    {"lavenderblush2", 224, {255, 215, 215}},
    {"lavenderblush3", 182, {215, 175, 215}},
    {"lavenderblush4", 102, {135, 135, 135}},
    {"lawngreen", 118, {135, 255, 0}},
    {"lemonchiffon", 230, {255, 255, 215}},
    {"lemonchiffon2", 223, {255, 215, 175}},
    {"lemonchiffon3", 187, {215, 215, 175}},
    {"lemonchiffon4", 101, {135, 135, 95}},
    {"lightblack", 243, {128, 128, 128}},
    {"lightblue", 12, {175, 215, 215}},
    {"lightblue2", 159, {175, 255, 255}},
    {"lightblue3", 153, {175, 215, 255}},
    {"lightblue4", 110, {135, 175, 215}},
    {"lightblue5", 66, {95, 135, 135}},
    {"lightcoral", 210, {255, 135, 135}},
    {"lightcyan", 14, {215, 255, 255}},
    {"lightcyan3", 152, {175, 215, 215}},
    {"lightcyan4", 102, {135, 135, 135}},
    {"lightgoldenrod", 222, {255, 215, 135}},
    {"lightgoldenrod2", 228, {255, 255, 135}},
    {"lightgoldenrod3", 179, {215, 175, 95}},
    {"lightgoldenrod4", 101, {135, 135, 95}},
    {"lightgoldenrodyellow", 230, {255, 255, 215}},
    {"lightgray", 188, {215, 215, 215}},
    {"lightgreen", 10, {135, 255, 135}},
    {"lightgrey", 188, {215, 215, 215}},
    {"lightmagenta", 13, {255, 85, 255}},
    {"lightpink", 217, {255, 175, 175}},
    {"lightpink2", 174, {215, 135, 135}},
    {"lightpink3", 95, {135, 95, 95}},
    {"lightred", 9, {255, 85, 85}},
    {"lightsalmon", 216, {255, 175, 135}},
    {"lightsalmon2", 209, {255, 135, 95}},
    {"lightsalmon3", 173, {215, 135, 95}},
    {"lightsalmon4", 95, {135, 95, 95}},
    {"lightseagreen", 37, {0, 175, 175}},
    {"lightskyblue", 117, {135, 215, 255}},
    {"lightskyblue2", 153, {175, 215, 255}},
    {"lightskyblue3", 110, {135, 175, 215}},
    {"lightskyblue4", 66, {95, 135, 135}},
    {"lightslateblue", 99, {135, 95, 255}},
    {"lightslategray", 102, {135, 135, 135}},
    {"lightsteelblue", 152, {175, 215, 215}},
    {"lightsteelblue2", 189, {215, 215, 255}},
    {"lightsteelblue3", 153, {175, 215, 255}},
    {"lightsteelblue4", 146, {175, 175, 215}},
    {"lightsteelblue5", 66, {95, 135, 135}},
    {"lightyellow", 11, {255, 255, 215}},
    {"lightyellow2", 230, {255, 255, 215}},
    {"lightyellow3", 187, {215, 215, 175}},
    {"lightyellow4", 102, {135, 135, 135}},
    {"lightwhite", 15, {255, 255, 255}},
    {"limegreen", 77, {95, 215, 95}},
    {"linen", 230, {255, 255, 215}},
    {"magenta", 5, {255, 0, 255}},
    {"magenta2", 164, {215, 0, 215}},
    {"magenta3", 90, {135, 0, 135}},
    {"maroon", 131, {175, 95, 95}},
    {"maroon2", 205, {255, 95, 175}},
    {"maroon3", 162, {215, 0, 135}},
    {"maroon4", 89, {135, 0, 95}},
    {"mediumaquamarine", 79, {95, 215, 175}},
    {"mediumblue", 20, {0, 0, 215}},
    {"mediumorchid", 134, {175, 95, 215}},
    {"mediumorchid2", 171, {215, 95, 255}},
    {"mediumorchid3", 134, {175, 95, 215}},
    {"mediumorchid4", 96, {135, 95, 135}},
    {"mediumpurple", 98, {135, 95, 215}},
    {"mediumpurple3", 141, {175, 135, 255}},
    {"mediumpurple4", 98, {135, 95, 215}},
    {"mediumpurple5", 60, {95, 95, 135}},
    {"mediumseagreen", 71, {95, 175, 95}},
    {"mediumslateblue", 99, {135, 95, 255}},
    {"mediumspringgreen", 48, {0, 255, 135}},
    {"mediumturquoise", 80, {95, 215, 215}},
    {"mediumvioletred", 162, {215, 0, 135}},
    {"midnightblue", 17, {0, 0, 95}},
    {"mintcream", 231, {255, 255, 255}},
    {"mistyrose", 224, {255, 215, 215}},
    {"mistyrose2", 181, {215, 175, 175}},
    {"mistyrose3", 102, {135, 135, 135}},
    {"moccasin", 223, {255, 215, 175}},
    {"navajowhite", 223, {255, 215, 175}},
    {"navajowhite2", 180, {215, 175, 135}},
    {"navajowhite3", 101, {135, 135, 95}},
    {"navy", 18, {0, 0, 135}},
    {"navyblue", 18, {0, 0, 135}},
    {"oldlace", 230, {255, 255, 215}},
    {"olivedrab", 64, {95, 135, 0}},
    {"olivedrab2", 155, {175, 255, 95}},
    {"olivedrab3", 113, {135, 215, 95}},
    {"olivedrab4", 64, {95, 135, 0}},
    {"orange", 214, {255, 175, 0}},
    {"orange2", 214, {255, 175, 0}},
    {"orange3", 208, {255, 135, 0}},
    {"orange4", 172, {215, 135, 0}},
    {"orange5", 94, {135, 95, 0}},
    {"orangered", 202, {255, 95, 0}},
    {"orangered2", 166, {215, 95, 0}},
    {"orangered3", 88, {135, 0, 0}},
    {"orchid", 170, {215, 95, 215}},
    {"orchid2", 213, {255, 135, 255}},
    {"orchid3", 212, {255, 135, 215}},
    {"orchid4", 170, {215, 95, 215}},
    {"orchid5", 96, {135, 95, 135}},
    {"palegoldenrod", 223, {255, 215, 175}},
    {"palegreen", 120, {135, 255, 135}},
    {"palegreen2", 114, {135, 215, 135}},
    {"palegreen3", 65, {95, 135, 95}},
    {"paleturquoise", 159, {175, 255, 255}},
    {"paleturquoise2", 116, {135, 215, 215}},
    {"paleturquoise3", 66, {95, 135, 135}},
    {"palevioletred", 168, {215, 95, 135}},
    {"palevioletred2", 211, {255, 135, 175}},
    {"palevioletred3", 168, {215, 95, 135}},
    {"palevioletred4", 95, {135, 95, 95}},
    {"papayawhip", 230, {255, 255, 215}},
    {"peachpuff", 223, {255, 215, 175}},
    {"peachpuff2", 223, {255, 215, 175}},
    {"peachpuff3", 180, {215, 175, 135}},
    {"peachpuff4", 101, {135, 135, 95}},
    {"peru", 173, {215, 135, 95}},
    {"pink", 218, {255, 175, 215}},
    {"pink2", 217, {255, 175, 175}},
    {"pink3", 175, {215, 135, 175}},
    {"pink4", 95, {135, 95, 95}},
    {"plum", 182, {215, 175, 215}},
    {"plum2", 219, {255, 175, 255}},
    {"plum3", 176, {215, 135, 215}},
    {"plum4", 96, {135, 95, 135}},
    {"powderblue", 152, {175, 215, 215}},
    {"purple", 129, {175, 0, 255}},
    {"purple2", 135, {175, 95, 255}},
    {"purple3", 93, {135, 0, 255}},
    {"purple4", 92, {135, 0, 215}},
    {"purple5", 54, {95, 0, 135}},
    {"red", 1, {255, 0, 0}},
    {"red2", 160, {215, 0, 0}},
    {"red3", 88, {135, 0, 0}},
    {"rosybrown", 138, {175, 135, 135}},
    {"rosybrown2", 217, {255, 175, 175}},
    {"rosybrown3", 181, {215, 175, 175}},
    {"rosybrown4", 95, {135, 95, 95}},
    {"royalblue", 62, {95, 95, 215}},
    {"royalblue2", 69, {95, 135, 255}},
    {"royalblue3", 63, {95, 95, 255}},
    {"royalblue4", 62, {95, 95, 215}},
    {"royalblue5", 24, {0, 95, 135}},
    {"saddlebrown", 94, {135, 95, 0}},
    {"salmon", 209, {255, 135, 95}},
    {"salmon2", 209, {255, 135, 95}},
    {"salmon3", 167, {215, 95, 95}},
    {"salmon4", 95, {135, 95, 95}},
    {"sandybrown", 215, {255, 175, 95}},
    {"seagreen", 29, {0, 135, 95}},
    {"seagreen2", 85, {95, 255, 175}},
    {"seagreen3", 84, {95, 255, 135}},
    {"seagreen4", 78, {95, 215, 135}},
    {"seagreen5", 29, {0, 135, 95}},
    {"seashell", 231, {255, 255, 255}},
    {"seashell2", 224, {255, 215, 215}},
    {"seashell3", 187, {215, 215, 175}},
    {"seashell4", 102, {135, 135, 135}},
    {"sienna", 130, {175, 95, 0}},
    {"sienna2", 209, {255, 135, 95}},
    {"sienna3", 167, {215, 95, 95}},
    {"sienna4", 94, {135, 95, 0}},
    {"skyblue", 117, {135, 215, 255}},
    {"skyblue2", 111, {135, 175, 255}},
    {"skyblue3", 74, {95, 175, 215}},
    {"skyblue4", 60, {95, 95, 135}},
    {"slateblue", 62, {95, 95, 215}},
    {"slateblue2", 99, {135, 95, 255}},
    {"slateblue3", 62, {95, 95, 215}},
    {"slateblue4", 60, {95, 95, 135}},
    {"slategray", 66, {95, 135, 135}},
    {"slategray2", 189, {215, 215, 255}},
    {"slategray3", 153, {175, 215, 255}},
    {"slategray4", 146, {175, 175, 215}},
    {"slategray5", 66, {95, 135, 135}},
    {"slategrey", 66, {95, 135, 135}},
    {"snow", 231, {255, 255, 255}},
    {"snow2", 224, {255, 215, 215}},
    {"snow3", 188, {215, 215, 215}},
    {"snow4", 102, {135, 135, 135}},
    {"springgreen", 48, {0, 255, 135}},
    {"springgreen2", 48, {0, 255, 135}},
    {"springgreen3", 41, {0, 215, 95}},
    {"springgreen4", 29, {0, 135, 95}},
    {"steelblue", 67, {95, 135, 175}},
    {"steelblue2", 75, {95, 175, 255}},
    {"steelblue3", 68, {95, 135, 215}},
    {"steelblue4", 60, {95, 95, 135}},
    {"tan", 180, {215, 175, 135}},
    {"tan2", 215, {255, 175, 95}},
    {"tan3", 209, {255, 135, 95}},
    {"tan4", 173, {215, 135, 95}},
    {"tan5", 94, {135, 95, 0}},
    {"thistle", 182, {215, 175, 215}},
    {"thistle2", 225, {255, 215, 255}},
    {"thistle3", 225, {255, 215, 255}},
    {"thistle4", 182, {215, 175, 215}},
    {"thistle5", 102, {135, 135, 135}},
    {"tomato", 203, {255, 95, 95}},
    {"tomato2", 167, {215, 95, 95}},
    {"tomato3", 94, {135, 95, 0}},
    {"turquoise", 80, {95, 215, 215}},
    {"turquoise2", 51, {0, 255, 255}},
    {"turquoise3", 45, {0, 215, 255}},
    {"turquoise4", 44, {0, 215, 215}},
    {"turquoise5", 30, {0, 135, 135}},
    {"violet", 213, {255, 135, 255}},
    {"violetred", 162, {215, 0, 135}},
    {"violetred2", 204, {255, 95, 135}},
    {"violetred3", 168, {215, 95, 135}},
    {"violetred4", 89, {135, 0, 95}},
    {"wheat", 223, {255, 215, 175}},
    {"wheat2", 223, {255, 215, 175}},
    {"wheat3", 180, {215, 175, 135}},
    {"wheat4", 101, {135, 135, 95}},
    {"white", 7, {255, 255, 255}},
    {"whitesmoke", 231, {224, 255, 255}},
    {"yellow", 3, {255, 255, 0}},
    {"yellow2", 184, {215, 215, 0}},
    {"yellow3", 100, {135, 135, 0}},
    {"yellowgreen", 113, {135, 215, 95}}
};

//! Length of colr_name_data.
const size_t colr_name_data_len = sizeof(colr_name_data) / sizeof(colr_name_data[0]);

/*! Allocate and format a string like `asprintf`, but wrap it in an allocated
    ColorResult.

    \pi fmt  Format string for `asprintf`.
    \pi ...  Other arguments for `asprintf`.
    \return  \parblock
                An allocated ColorResult.
                \maybenull
                \colrmightfree
             \endparblock
*/
ColorResult* Colr_fmt_str(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list argcopy;
    va_copy(argcopy, args);
    char* s = NULL;
    if (vasprintf(&s, fmt, argcopy) < 0) {
        va_end(argcopy);
        va_end(args);
        return NULL;
    }
    va_end(argcopy);
    va_end(args);
    return ColorResult_to_ptr(ColorResult_new(s));
}

/*! Allocates space for a regmatch_t, initializes it, and returns a pointer
    to it.
    \pi match A `regmatch_t` to allocate for and copy.
    \return   An allocated copy of the `regmatch_t`.
*/
regmatch_t* colr_alloc_regmatch(regmatch_t match) {
    regmatch_t* matchcopy = calloc(sizeof(regmatch_t), 1);
    *matchcopy = match;
    return matchcopy;
}

/*! Appends CODE_RESET_ALL to a \string, but makes sure to do it before any
    newlines.

    \pi s \parblock
            The string to append to.
            <em>Must have extra room for CODE_RESET_ALL</em>.
            \mustnull
          \endparblock
*/
void colr_append_reset(char* s) {
    if (!s) return;
    if (s[0] == '\0') {
        // Special case, an empty string, with room for CODE_RESET_ALL.
        snprintf(s, CODE_RESET_LEN, "%s", CODE_RESET_ALL);
        return;
    }
    if (colr_str_ends_with(s, CODE_RESET_ALL)) {
        // Already has one.
        return;
    }
    size_t length = strlen(s);
    size_t lastindex = length - 1;
    size_t newlines = 0;
    // Cut newlines off if needed. I'll add them after the reset code.
    while ((lastindex > 0) && (s[lastindex] == '\n')) {
        s[lastindex] = '\0';
        newlines++;
        lastindex--;
    }
    if ((lastindex == 0) && s[lastindex] == '\n') {
        // String starts with a newline.
        s[lastindex] = '\0';
        newlines++;
    } else {
        lastindex++;
    }
    char* p = s + lastindex;
    snprintf(p, CODE_RESET_LEN, "%s", CODE_RESET_ALL);
    p += CODE_RESET_LEN - 1;
    while (newlines--) {
        *(p++) = '\n';
    }
    *p = '\0';
}

/*! Returns the char needed to represent an escape sequence in C.

    \details
    The following characters are supported:
        Escape Sequence  |  Description Representation
        ---------------: | :--------------------------
                    @\ ' | single quote
                   @\ @" | double quote
                    @\ ? | question mark
                   @\ @\ | backslash
                    @\ a | audible bell
                    @\ b | backspace
                    @\ f | form feed - new page
                    @\ n | line feed - new line
                    @\ r | carriage return
                    @\ t | horizontal tab
                    @\ v | vertical tab

    \pi c   The character to check.
    \return The letter, without a backslash, needed to create an escape sequence.
            If the char doesn't need an escape sequence, it is simply returned.

    \examplecodefor{colr_char_escape_char,.c}
    char constantchar = colr_char_escape_char('\n');
    assert(constantchar == 'n');

    char constantquote = colr_char_escape_char('"');
    assert(constantquote == '"');

    // The actual escape sequence would need the backslash added to it:
    char* escaped;
    asprintf(&escaped, "\\%c", colr_char_escape_char('\t'));
    free(escaped);
    \endexamplecode
*/
char colr_char_escape_char(const char c) {
    switch (c) {
        // 0 is a special case for colr_char_repr().
        case '\0': return '0';
        case '\'': return '\'';
        case '\"': return '"';
        case '\?': return '?';
        case '\\': return '\\';
        case '\a': return 'a';
        case '\b': return 'b';
        case '\f': return 'f';
        case '\n': return 'n';
        case '\r': return 'r';
        case '\t': return 't';
        case '\v': return 'v';
        default:
            return c;
    }
}

/*! Determines if a character exists in the given \string.
    \pi c Character to search for.
    \pi s \parblock
            String to check.
            \mustnullin
          \endparblock

    \return `true` if \p c is found in \p s, otherwise `false`.
*/
bool colr_char_in_str(const char* s, const char c) {
    for (size_t i = 0; s[i]; i++) {
        if (s[i] == c) return true;
    }
    return false;
}

/*! Determines if a character is suitable for an escape code ending.

    \details
    `m` is used as the last character in color codes, but other characters
    can be used for escape sequences (such as "\x1b[2A", cursor up). Actual
    escape code endings can be in the range (`char`) 64-126 (inclusive).

    \details
    Since ColrC only deals with color codes and maybe some cursor/erase codes,
    this function tests if the character is either `A-Z` or `a-z`.

    \details
    For more information, see: https://en.wikipedia.org/wiki/ANSI_escape_code

    \pi c   Character to test.
    \return `true` if the character is a possible escape code ending, otherwise `false`.
*/
bool colr_char_is_code_end(const char c) {
    /*  The actual end chars can be: 64-126 (inclusive) ( ASCII: @A–Z[\]^_`a–z{ )
        I'm just testing for alpha chars. A: 65, Z: 90, a: 97, z: 122
        This is not a macro because it may be expanded in the future to detect
        the full range of "end" chars.
    */
    // Lowercase `m` is the most common case for ColrC.
    return ((c > 64) && (c < 91)) || ((c > 96) && (c < 123));
}

/*! Creates a \string representation for a char.

    \pi c   Value to create the representation for.
    \return \parblock
                An allocated string.
                \mustfree
                \maybenullalloc
            \endparblock
*/
char* colr_char_repr(char c) {
    char* repr;
    switch (c) {
        case '\0':
            asprintf_or_return(NULL, &repr, "'\\0'");
            break;
        case '\x1b':
            asprintf_or_return(NULL, &repr, "'\\x1b'");
            break;
        case '\'':
            asprintf_or_return(NULL, &repr, "'\\\''");
            break;
        case '\"':
            asprintf_or_return(NULL, &repr, "'\\\"'");
            break;
        case '\?':
            asprintf_or_return(NULL, &repr, "'\\?'");
            break;
        case '\\':
            asprintf_or_return(NULL, &repr, "'\\\\'");
            break;
        case '\a':
            asprintf_or_return(NULL, &repr, "'\\a'");
            break;
        case '\b':
            asprintf_or_return(NULL, &repr, "'\\b'");
            break;
        case '\f':
            asprintf_or_return(NULL, &repr, "'\\f'");
            break;
        case '\n':
            asprintf_or_return(NULL, &repr, "'\\n'");
            break;
        case '\r':
            asprintf_or_return(NULL, &repr, "'\\r'");
            break;
        case '\t':
            asprintf_or_return(NULL, &repr, "'\\t'");
            break;
        case '\v':
            asprintf_or_return(NULL, &repr, "'\\v'");
            break;
        default:
            if (iscntrl(c)) {
                // Handle all other non-printables in hex-form.
                asprintf_or_return(NULL, &repr, "'\\x%x'", c);
            } else {
                asprintf_or_return(NULL, &repr, "'%c'", c);
            }
    }
    return repr;
}

/*! Determines if an ascii character has an escape sequence in C.

    \details
    The following characters are supported:
        Escape Sequence  |  Description Representation
        ---------------: | :--------------------------
                    @\ ' | single quote
                   @\ @" | double quote
                    @\ ? | question mark
                   @\ @\ | backslash
                    @\ a | audible bell
                    @\ b | backspace
                    @\ f | form feed - new page
                    @\ n | line feed - new line
                    @\ r | carriage return
                    @\ t | horizontal tab
                    @\ v | vertical tab

    \pi c   The character to check.
    \return `true` if the character needs an escape sequence, otherwise `false`.
*/
bool colr_char_should_escape(const char c) {
    switch (c) {
        // 0 is a special case for colr_char_repr().
        case '\0': return true;
        case '\'': return true;
        case '\"': return true;
        case '\?': return true;
        case '\\': return true;
        case '\a': return true;
        case '\b': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default:
            return false;
    }
}


/*! Checks an unsigned int against the individual bytes behind a pointer's
    value. This helps to guard against overflows, because only a single
    byte is checked at a time. If any byte doesn't match the marker, `false`
    is immediately returned, instead of continuing past the pointer's bounds.

    \pi marker A colr marker, like COLORARG_MARKER, COLORTEXT_MARKER, etc.
    \pi p      A pointer to check, to see if it starts with the marker.
    \return    `true` if all bytes match the marker, otherwise `false`.

    \sa ColorArg_is_ptr
    \sa ColorText_is_ptr

    \examplecodefor{colr_check_marker,.c}
    // This is actually used with void pointer, where the type is not known.
    // It's only known that the pointer will be 1 of 3 types.
    ColorArg* cargp = fore(GREEN);
    assert(colr_check_marker(COLORARG_MARKER, cargp));
    colr_free(cargp);

    char* s = "Not a ColorText at all.";
    assert(!colr_check_marker(COLORTEXT_MARKER, s));

    ColorText* ctextp = Colr("But this is a ColorText.", fore(GREEN));
    assert(colr_check_marker(COLORTEXT_MARKER, ctextp));
    colr_free(ctextp);

    printf("colr_check_marker() assertions held up.\n");
    \endexamplecode
*/
bool colr_check_marker(uint32_t marker, void* p) {
    // Don't overflow the bounds. Check a byte at a time.
    if (!p) return false;
    ColorStructMarker byte_checker = {marker};
    uint8_t* singlebyte = p;
    if (*singlebyte != byte_checker.bytes.b1) return false;
    else if (*(singlebyte + 1) != byte_checker.bytes.b2) return false;
    else if (*(singlebyte + 2) != byte_checker.bytes.b3) return false;
    else if (*(singlebyte + 3) != byte_checker.bytes.b4) return false;
    return true;
}

/*! Allocates an empty \string.
    \details
    This is for keeping the interface simple, so the return values from
    color functions with invalid values can be consistent.

    \return \parblock
                Pointer to an allocated string consisting of '\0'.
                \mustfree
                \maybenullalloc
            \endparblock
*/
char* colr_empty_str(void) {
    char* s = malloc(sizeof(char));
    if (!s) return NULL;
    s[0] = '\0';
    return s;
}

/*! Free an array of allocated `regmatch_t`, like the return from
    colr_re_matches().

    \po matches A pointer to an array of `regmatch_t` pointers.
*/
void colr_free_re_matches(regmatch_t** matches) {
    if (!matches) return;
    size_t cnt = 0;
    while (matches[cnt]) {
        free(matches[cnt]);
        matches[cnt++] = NULL;
    }
    free(matches);
}

/*! Like `mbrlen`, except it will return the length of the next N (`length`)
    multibyte characters in bytes.

    /details
    Unlike colr_str_mb_len(), which returns the number of multibyte characters,
    this function will return the number of bytes that make up the next
    number (`length`) of multibyte characters.

    \pi s              The string to check.
    \pi length         Number of multibyte characters to get the length for.
    \return            The number of bytes parsed in \p s to get at least \p
                       length multibyte characters.
    \retval 0          if \p s is `NULL`/empty, or `length` is `0`.
    \retval (size_t)-1 if an invalid multibyte sequence is found at the start
                       of \p s.

    \sa colr_str_mb_len
    \sa colr_is_valid_mblen

    \examplecodefor{colr_mb_len,.c}
    #include "colr.h"
    int main(void) {
        // There are 4 multibyte characters in the string.
        // There are multiple bytes making up each character, probably 3 each,
        // which makes the byte-length 12.
        char* s = "１３３７";

        // Calling colr_mb_len(s, 1) is like calling mbrlen().
        size_t first_len = colr_mb_len(s, 1);
        mbstate_t st;
        memset(&st, 0, sizeof(mbstate_t));
        assert(first_len == (size_t)mbrlen(s, MB_LEN_MAX, &st));

        size_t first_two_len = colr_mb_len(s, 2);
        // This assertion doesn't hold for ALL characters, but it should hold
        // for these, because they are all FULLWIDTH DIGIT chars.
        assert(first_two_len == (first_len * 2));

        // Skip about 6 bytes, to get past the first 2
        char* skip2 = s + first_two_len;
        assert(strcmp(skip2, "３７") == 0);

        // There are only about 7 bytes (6 + 1 for the null).
        // Calling colr_mb_len with a `length` that is too large is okay.
        size_t length = colr_mb_len(skip2, 100);
        assert(length == strlen(skip2));
        size_t charlength = colr_str_mb_len(skip2);
        printf("\"%s\" contains %lu bytes for %lu chars.\n", skip2, length, charlength);

        // This should always hold true, even though it is useless:
        assert(colr_mb_len(s, strlen(s)) == strlen(s));

    }

    \endexamplecode
*/
size_t colr_mb_len(const char* s, size_t length) {
    if ((!s) || (s[0] == '\0')) return 0;
    if (length < 1) return 0;
    colr_set_locale();
    mbstate_t st;
    memset(&st, 0, sizeof(mbstate_t));
    if (length == 1) return mbrlen(s, MB_LEN_MAX, &st);
    size_t total = 0;
    while (s[total] && length--) {
        size_t char_len = mbrlen(s, MB_LEN_MAX, &st);
        if (!colr_is_valid_mblen(char_len)) {
            // Invalid multibyte (-1), or incomplete multibyte (-2).
            return char_len;
        }
        total += char_len;
    }
    return total;
}

#ifdef COLR_GNU
/*! Handles printing with printf for Colr objects.

    \details
    This function matches the required `typedef` in `printf.h` (`printf_function`),
    for handling a custom printf format char with `register_printf_specifier`.

    \gnuonly

    \pi fp   FILE pointer for output.
    \pi info Info from printf about how to format the argument.
    \pi args Argument list (with only 1 argument), containing a ColorArg,
             ColorResult, ColorText, or \string to format.
    \return  The number of characters written.
*/
int colr_printf_handler(FILE *fp, const struct printf_info *info, const void *const *args) {
    (void)info; // Unused.
    // Wide character output is not supported right now.
    assert(info->wide == 0);

    void* p = *(void**)args[0];
    char* s = NULL;
    ColorArg* cargp = NULL;
    ColorResult* cresp = NULL;
    ColorText* ctextp = NULL;
    if (ColorArg_is_ptr(p)) {
        cargp = p;
        s = ColorArg_to_esc(*cargp);
        ColorArg_free(cargp);
    } else if (ColorText_is_ptr(p)) {
        ctextp = p;
        s = ColorText_to_str(*ctextp);
        ColorText_free(ctextp);
    } else if (ColorResult_is_ptr(p)) {
        cresp = p;
        s = ColorResult_to_str(*cresp);
    } else {
        // Better be a string.
        s = (char*)p;
    }
    if (!s) return 1;
    if (info->alt) {
        // Alternate-form (#), no codes.
        char* stripped = colr_str_strip_codes(s);
        // If I couldn't allocate for the stripped version, use the original.
        // Worse things than that are about to happen anyway.
        if (stripped) {
            free(s);
            s = stripped;
        }

    }
    char* justified = NULL;
    if (info->width) {
        // TODO: Use wchar_t in justification funcs to forward `info->pad` to it.
        //       I'm not sure how this is used from printf though.
        int pad = info->pad ? wctob(info->pad) : ' ';
        if (pad == EOF) pad = ' ';
        if (info->left) {
            justified = colr_str_ljust(s, info->width, pad);
        } else if (info->space) {
            justified = colr_str_center(s, info->width, pad);
        } else {
            justified = colr_str_rjust(s, info->width, pad);
        }
        if (justified) {
            if (cargp || ctextp) free(s);
            if (cresp) ColorResult_free(cresp);
            s = justified;
        } else {
            // Can't allocate for justified text.
            return 1; // LCOV_EXCL_LINE
        }
    }
    // Escape output if the '/' modifier was used.
    if (info->user & colr_printf_esc_mod) {
        char* escaped = colr_repr(s);
        if (escaped) {
            // Escape was successful, we don't need the original `s` anymore.
            free(s);
            s = escaped;
        }
    }
    fprintf(fp, "%s", s);
    size_t length = strlen(s);
    if (cargp || ctextp || justified) free(s);
    if (cresp) ColorResult_free(cresp);

    return length;
}

/*! Handles the arg count/size for the Colr printf handler.

    \details
    This function matches the required `typedef` in `printf.h` (`printf_arginfo_size_function`)
    for handling a custom printf format char with `register_printf_specifier`.

    \gnuonly

    \pi info     Info from printf about how to format the argument.
    \pi n        Number of arguments for the format char.
    \po argtypes Type of arguments being handled, from an `enum` defined in `printf`.
                 Colr uses/sets one argument, a `PA_POINTER` type.
    \po sz       Size of the arguments. Not used in Colr.
    \return      The number of argument types set in \p argtypes.
*/
int colr_printf_info(const struct printf_info *info, size_t n, int *argtypes, int *sz) {
    (void)info; // Unused.
    (void)sz; // Unused.
     if (n > 0)
     {
          // Only setting one argument for Colr.
          argtypes[0] = PA_POINTER;
     }
     // Only using one argument for Colr.
     return 1;
}

/*! Registers COLR_FMT_CHAR to handle Colr objects in the printf-family
    functions.

    \details
    This function only needs to be called once and `register_printf_specifier`
    is only called the first time this function is called.

    \gnuonly

    \examplecodefor{colr_printf_register,.c}
    #include "colr.h"

    // These are needed if you have certain warnings enabled.
    #pragma clang diagnostic ignored "-Winvalid-format-specifier"
    #pragma GCC diagnostic ignored "-Wformat="
    #pragma GCC diagnostic ignored "-Wformat-extra-args"

    int main(void) {
        colr_printf_register();
        printf("A colr object: %R\n", Colr("Hello", fore(RED)));

        char* colorized = NULL;
        asprintf(
            &colorized,
            "I made this: %R",
            Colr("No, I made this.", style(UNDERLINE))
        );
        puts(colorized);
        free(colorized);
    }
    \endexamplecode
*/
void colr_printf_register(void) {
    static bool is_registered = false;
    if (is_registered) return;
    // Do the actual registering, if not done already.
    register_printf_specifier(COLR_FMT_CHAR, colr_printf_handler, colr_printf_info);
    // Register the '/' modifier for escaped output.
    colr_printf_esc_mod = register_printf_modifier(L"/");
    is_registered = true;
}
#endif // COLR_GNU

/*! Sets the locale to `(LC_ALL, "")` if it hasn't already been set.
    \details
    This is used for functions dealing with multibyte strings.

    \return `true` if the locale had to be set, `false` if it was already set.
*/
bool colr_set_locale(void) {
    static bool checked = false;
    // Calling this function more than once is not going to do anything.
    if (checked) return false;
    checked = true;
    locale_t current = uselocale(0);
    if (current == LC_GLOBAL_LOCALE) {
        // Not set yet, set it.
        setlocale(LC_ALL, "");
        return checked;
    }
    // Locale already set.
    // No tests for this line yet.
    return false; // LCOV_EXCL_LINE
}

/*! Determine if a \string is in an array of strings (`char**`, where the last
    element is `NULL`).

    \pi lst  The string array to look in.
    \pi s    The string to look for.
    \return  `true` if the string is found, otherwise `false`.
    \retval  `false` if \p lst is `NULL` or \p s is `NULL`.

    \examplecodefor{colr_str_array_contains,.c}
    char* strarray[] = {
        "this",
        "that",
        NULL
    };
    if (colr_str_array_contains(strarray, "that")) {
        printf("List contained \"that\".\n");
    }
    if (!colr_str_array_contains(strarray, "NONEXISTENT")) {
        printf("List did not contain \"NONEXISTENT\".\n");
    }
    \endexamplecode
*/
bool colr_str_array_contains(char** lst, const char* s) {
    if (!(lst && s)) return false;
    ColrHash strhash = colr_str_hash(s);
    size_t i = 0;
    while (lst[i]) {
        ColrHash hash = colr_str_hash(lst[i]);
        i++;
        if (hash == strhash) return true;
    }
    return false;
}

/*! Free an allocated array of strings, including the array itself.

    \details
    Each individual string will be released, and finally the allocated memory
    for the array of pointers will be released.

    \pi ps A pointer to an array of strings.

    \examplecodefor{colr_str_array_free,.c}
    char* s = colr("Test", fore(RED), back(WHITE), style(BRIGHT));
    if (!s) exit(1);
    // Call something that creates an array of strings on the heap.
    char** code_array = colr_str_get_codes(s, false);
    free(s);
    if (!code_array) exit(1);
    // ... do something with the array of strings.

    // And then free it:
    colr_str_array_free(code_array);
    \endexamplecode
*/
void colr_str_array_free(char** ps) {
    if (!ps) return;
    // Free the individual items, until NULL is hit.
    for (size_t i = 0; ps[i]; i++) free(ps[i]);
    // Free the pointer array.
    free(ps);
}

/*! Center-justifies a \string, ignoring escape codes when measuring the width.

    \pi s       \parblock
                    The string to justify.
                    \mustnullin
                \endparblock
    \pi width   The overall width for the resulting string.
                If set to '0', the terminal width will be used from colr_term_size().
    \pi padchar The character to pad with. If '0', then `' '` is used.

    \return     \parblock
                    An allocated string with the result.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa colr_str_ljust
    \sa colr_str_rjust
    \sa colr_term_size

    \examplecodefor{colr_str_center,.c}
    char* colorized = colr("This.", fore(RED), back(WHITE));
    char* justified = colr_str_center(colorized, 9, ' ');
    free(colorized);
    // The string still has codes, but only 4 spaces were added.
    assert(colr_str_starts_with(justified, "  "));
    assert(colr_str_ends_with(justified, "  "));
    // It was "justified" to 9 characters long, but it is well over that.
    assert(strlen(justified) > 9);
    printf("'%s'\n", justified);
    free(justified);
    \endexamplecode
*/
char* colr_str_center(const char* s, int width, const char padchar) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = colr_str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    size_t pos = 0;
    size_t leftdiff = diff / 2;
    if (diff % 2 == 1) leftdiff++;
    // Handle left-just.
    while (pos < leftdiff) {
        result[pos++] = pad;
    }
    // Handle string.
    size_t i = 0;
    while (s[i]) {
        result[pos++] = s[i++];
    }
    // Handle remaining right-just.
    final_len--;
    while (pos < final_len) {
        result[pos++] = pad;
    }
    return result;
}

/*! Counts the number of characters (`c`) that are found in a \string (`s`).

    \details
    Returns `0` if `s` is `NULL`, or `c` is `'\0'`.

    \pi s \parblock
            The string to examine.
            \mustnull
          \endparblock
    \pi c \parblock
            The character to count.
            \mustnotzero
          \endparblock

    \return The number of times \p c occurs in \p s.

    \examplecodefor{colr_str_char_count,.c}
    size_t ps = colr_str_char_count("apple", 'p');
    assert(ps == 2);
    printf("Found %lu 'p' characters.\n", ps)
    \endexamplecode
*/
size_t colr_str_char_count(const char* s, const char c) {
    if (!(s && c)) return 0;
    if (s[0] == '\0') return 0;

    size_t i = 0;
    size_t total = 0;
    while (s[i]) {
        if (s[i++] == c) total++;
    }
    return total;
}

/*! Counts the number of characters (`c`) that are found at the beginning of a
    \string (`s`).

    \details
    Returns `0` if \p s is `NULL`, \p c is `'\0'`, or the string doesn't start
    with \p c.

    \pi s \parblock
            The string to examine.
            \mustnull
          \endparblock
    \pi c \parblock
            The character to count.
            \mustnotzero
          \endparblock

    \return The number of times \p c occurs at the start of \p s.

    \examplecodefor{colr_str_char_lcount,.c}
    size_t xs = colr_str_char_lcount("xxApple", 'x');
    assert(xs == 2);
    printf("Found %lu 'x' characters.\n", xs)
    \endexamplecode

*/
size_t colr_str_char_lcount(const char* s, const char c) {
    if (!(s && c)) return 0;
    if (s[0] == '\0') return 0;

    size_t i = 0;
    size_t total = 0;
    while (s[i++] == c) total++;
    return total;
}

/*! Counts the number of characters that are found at the beginning of a
    \string (`s`), where the character can be any of \p chars.

    \details
    Returns `0` if \p s is `NULL`/empty, \p chars is `NULL`/empty, or the
    string doesn't start with any of the characters in \p chars.

    \pi s     \parblock
                The string to examine.
                \mustnull
              \endparblock
    \pi chars \parblock
                The characters to count, in any order.
                \mustnotzero
              \endparblock

    \return   The number of times a character in \p chars occurs at the start
              of \p s.

    \examplecodefor{colr_str_chars_lcount,.c}
    size_t chars = colr_str_chars_lcount("cbaabcbbccApple", "abc");
    assert(chars == 10);
    printf("Found %lu 'a', 'b', or 'c' characters.\n", chars)
    \endexamplecode

*/
size_t colr_str_chars_lcount(const char* restrict s, const char* restrict chars) {
    if (!(s && chars)) return 0;
    if ((s[0] == '\0') || (chars[0] == '\0')) return 0;

    size_t i = 0;
    size_t total = 0;
    while (colr_char_in_str(chars, s[i++])) total++;
    return total;
}

/*! Return the number of escape-codes in a \string.

    \pi s   \parblock
                A string to count the escape-codes for.
                \mustnull
            \endparblock
    \return The number of escape-codes, or `0` if \p s is `NULL`, or
            doesn't contain any escape-codes.

    \examplecodefor{colr_str_code_count,.c}
    char* s = colr("Testing this out.", fore(RED), back(WHITE));
    if (!s) exit(1);
    size_t code_cnt = colr_str_code_count(s);
    assert(code_cnt == 3); // The reset code is also appended.
    printf("Found codes: %zu\n", code_cnt);
    free(s);
    \endexamplecode
*/
size_t colr_str_code_count(const char* s) {
    if (!s) return 0;
    if (s[0] == '\0') return 0;
    // Length of code, minus the 'm' and '\0'.
    size_t code_max = CODE_RGB_LEN - 2;
    size_t total = 0;
    size_t i = 0;
    while (s[i]) {
        // Skip past non code stuff, if any.
        while (s[i] && s[i] != '\x1b') i++;
        if (s[i] == '\0') break;
        // Have code.
        size_t current_code = 0;
        // Grab the rest of the code chars.
        while (s[i] && !colr_char_is_code_end(s[i])) {
            i++;
            current_code++;
            if (current_code > code_max) {
                // Overflow. Ignore this crazy code.
                current_code = 0;
                break;
            }
        }
        if (current_code && (current_code <= code_max)) {
            // Have a complete code.
            total += 1;
        }
    }
    return total;
}
/*! Return the number of bytes that make up all the escape-codes in a \string.

    \pi s   \parblock
                A string to count the code-chars for.
                \mustnull
            \endparblock
    \return The number of escape-code characters, or `0` if \p s is `NULL`, or
            doesn't contain any escape-codes.

    \examplecodefor{colr_str_code_len,.c}
    char* s = colr("Testing this out.", fore(RED), back(WHITE));
    if (!s) exit(1);
    size_t code_len = colr_str_code_len(s);
    assert(code_len == 14); // The reset code is also appended.
    printf("Found code chars: %zu\n", code_len);
    free(s);
    \endexamplecode
*/
size_t colr_str_code_len(const char* s) {
    if (!s) return 0;
    if (s[0] == '\0') return 0;
    // Length of code, minus the 'm' and '\0'.
    size_t code_max = CODE_RGB_LEN - 2;
    size_t total = 0;
    size_t i = 0;
    while (s[i]) {
        // Skip past non code stuff, if any.
        while (s[i] && s[i] != '\x1b') i++;
        if (s[i] == '\0') break;
        // Have code.
        size_t current_code = 0;
        // Grab the rest of the code chars.
        while (s[i] && !colr_char_is_code_end(s[i])) {
            i++;
            current_code++;
            if (current_code > code_max) {
                // Overflow. Ignore this crazy code.
                current_code = 0;
                break;
            }
        }
        if (current_code && (current_code <= code_max)) {
            // Have a complete code, don't forget the last 'm'.
            total += current_code + 1;
        }
    }
    return total;
}

/*! Copies a \string like strncpy, but ensures null-termination.

    \details
    If \p src is `NULL`, or \p dest is `NULL`, `NULL` is returned.

    \details
    If \p src does not contain a null-terminator, _this function
    will truncate at `length` characters_.

    \details
    If \p src is an empty string, then `dest[0]` will be `'\0'` (an empty string).

    \details
    A null-terminator is always appended to \p dest.

    \details
    \p src and \p dest must not overlap.

    \pi dest   \parblock
                    Memory allocated for new string.
                    <em>Must have room for `strlen(src) + 1` or `length + 1`</em>.
               \endparblock
    \pi src    Source string to copy.
    \pi length \parblock
                    Maximum characters to copy.
                    <em>This does not include the null-terminator</em>.
                    Usually set to `strlen(dest)`.
               \endparblock
    \returns On success, a pointer to dest is returned.

    \examplecodefor{colr_str_copy,.c}
    char* s = "testing";
    size_t length = strlen(s);
    char* dest = malloc(length + 1);

    // Copy the entire string:
    colr_str_copy(dest, s, length);
    assert(strcmp(dest, "testing") == 0);
    printf("Copied: %s\n", dest);

    // Copy only 4 bytes:
    colr_str_copy(dest, s, 4);
    assert(strcmp(dest, "test") == 0);
    printf("Truncated: %s\n", dest);

    // String gets truncated when dest is too small:
    colr_str_copy(dest, "this is too long.", length);
    assert(strcmp(dest, "this is") == 0);
    assert(dest[length] == '\0');
    printf("Truncated: %s\n", dest);
    free(dest);
    \endexamplecode
*/
char* colr_str_copy(char* restrict dest, const char* restrict src, size_t length) {
    if (!(src && dest)) {
        return NULL;
    }
    size_t pos = 0;
    while (pos < length && src[pos]) {
        dest[pos] = src[pos];
        pos++;
    }
    // This will make `dest` and empty string if `src` was an empty string.
    dest[pos] = '\0';
    return dest;
}

/*! Determine if one \string ends with another.

    \details
    \p str and \p suffix must not overlap.

    \pi s      \parblock
                    String to check.
                    \mustnull
               \endparblock
    \pi suffix \parblock
                    Suffix to check for.
                    \mustnull
               \endparblock
    \return    True if `str` ends with `suffix`.
    \return    False if either is NULL, or the string doesn't end with the suffix.
*/
bool colr_str_ends_with(const char* restrict s, const char* restrict suffix) {
    if (!s || !suffix) {
        return false;
    }
    size_t strlength = strlen(s);
    size_t suflength = strlen(suffix);
    if ((!(suflength && strlength)) || (suflength > strlength)) {
        // Empty strings, or suffix is longer than the entire string.
        return false;
    }
    return (strncmp(s + (strlength - suflength), suffix, suflength) == 0);
}

/*! Get an array of escape-codes from a \string.

    \details
    This function copies the escape-code strings, and the pointers to the heap,
    if any escape-codes are found in the string.

    \details
    colr_str_array_free() can be used to easily `free()` the result of this function.

    \pi s      \parblock
                    A string to get the escape-codes from.
                    \mustnull
               \endparblock
    \pi unique Whether to only include _unique_ escape codes.
    \return    \parblock
                    An allocated array of \string pointers, where the last element is `NULL`.
                    \mustfree
               \endparblock
    \retval    If \p s is `NULL`, or empty, or there are otherwise no escape-codes
               found in the string, or allocation fails for the strings/array, then
               `NULL` is returned.
    \retval    On success, there will be at least two pointers behind the return
               value. The last pointer is always `NULL`.

    \examplecodefor{colr_str_get_codes,.c}
    #include <stdio.h>
    #include "colr.h"
    int main(void) {
        char* s = colr_cat(
            Colr("Testing this out.", fore(RED), back(WHITE)),
            Colr("Again.", fore(RED), style(UNDERLINE))
        );
        if (!s) return EXIT_FAILURE;
        char** code_array = colr_str_get_codes(s, false);
        free(s);
        if (!code_array) {
            fprintf(stderr, "No code found? Impossible!\n");
            return EXIT_FAILURE;
        }
        // Iterate over the code array.
        for (size_t i = 0; code_array[i]; i++) {
            char* code_repr = colr_repr(code_array[i]);
            printf("Found code: %s\n", code_repr);
            free(code_repr);
        }
        // Free the strings, and the array of pointers.
        colr_str_array_free(code_array);
    }
    \endexamplecode
*/
char** colr_str_get_codes(const char* s, bool unique) {
    if (!s) return NULL;
    if (s[0] == '\0') return NULL;
    // They may all be unique, so leave room just in case.
    size_t code_cnt = colr_str_code_count(s);
    if (!code_cnt) return NULL;
    // Allocate memory for some string pointers.
    char** code_array = calloc(code_cnt + 1, sizeof(char*));
    if (!code_array) return NULL;
    char** array_start = code_array;
    size_t array_pos = 0;
    size_t i = 0;
    char current_code[CODE_RGB_LEN] = {0};
    // Length of code, minus the 'm'.
    size_t code_max = CODE_RGB_LEN - 2;
    while (s[i]) {
        // Skip past non code stuff, if any.
        while (s[i] && s[i] != '\x1b') i++;
        if (s[i] == '\0') break;
        // Have code, copy the start char.
        size_t pos = 0;
        current_code[pos] = s[i++];
        pos++;
        // Grab the rest of the code chars.
        while (s[i] && !colr_char_is_code_end(s[i])) {
            if (pos < code_max) {
                current_code[pos++] = s[i];
            } else {
                // Overflowed the code buffer because the code is probably
                // malformed. Just ignore this one.
                current_code[0] = '\0';
            }
            i++;
        };
        if (current_code[0]) {
            // Have a complete code.
            current_code[pos++] = 'm';
            current_code[pos] = '\0';
            // Make a copy of it and add it to the array of pointers.
            char* code_copy = strndup(current_code, pos);
            if (!code_copy) return NULL;
            if (unique) {
                // Mark this position as NULL, until we know that it will be added.
                // The NULL marker will be overwritten for the next code, or
                // will symbolize the end of the array.
                code_array[array_pos] = NULL;
                if (colr_str_array_contains(code_array, code_copy)) {
                    // Skip it.
                    free(code_copy);
                    current_code[0] = '\0';
                    continue;
                }
            }
            code_array[array_pos++] = code_copy;
            // Reset current code.
            current_code[0] = '\0';
        }
    }
    // Set the last item to NULL, even if it's the first item.
    code_array[array_pos] = NULL;
    return array_start;
}

/*! Determines if a \string has ANSI escape codes in it.

    \details
    This will detect any ansi escape code, not just colors.

    \pi s   \parblock
                The string to check. Can be `NULL`.
                \mustnullin
            \endparblock

    \return `true` if the string has at least one escape code, otherwise `false`.

    \sa colr_str_is_codes
*/
bool colr_str_has_codes(const char* s) {
    if (!s) return false;
    size_t length = strlen(s);
    size_t i = 0;
    while ((i < length) && s[i]) {
        if ((s[i] == '\x1b') && (s[i + 1] == '[')) {
            // Skip past "\x1b["
            i += 2;
            while ((i < length) && s[i]) {
                if (s[i] == 'm') return true;
                if (!(isdigit(s[i]) || s[i] == ';')) return false;
                i++;
            }
        }
        i++;
    }
    return false;
}

/*! Hash a string using [djb2](http://www.cse.yorku.ca/~oz/hash.html).

    \details
    This is only used for simple, short, \string hashing.
    It is not designed for cryptography.

    \details
    There are some notes about collision rates for this function
    [here](https://softwareengineering.stackexchange.com/a/145633).

    \pi s   \parblock
                The string to hash.
                \mustnull
            \endparblock
    \return A \ColrHash value with the hash.
    \retval 0 if \p s is `NULL`.
    \retval COLR_HASH_SEED if \p s is an empty string.

    \examplecodefor{colr_str_hash,.c}
    char* strings[] = {
        "fore",
        "back",
        "style",
        "invalid",
        "red",
        "\x1b[4m",
        "\x1b[0m"
    };
    size_t strings_len = sizeof(strings) / sizeof(strings[0]);
    for (size_t i = 0; i < strings_len; i++) {
        ColrHash hashval = colr_str_hash(strings[i]);
        printf("%8s: hash=%lu\n", strings[i], hashval);
    }
    \endexamplecode
*/

ColrHash colr_str_hash(const char* s) {
    if (!s) return 0;
    // This is also the default value for empty strings.
    ColrHash hash = COLR_HASH_SEED;
    int c;

    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/*! Determines whether a \string consists of only one character, possibly repeated.

    \pi s   String to check.
    \pi c   Character to test for. Must not be `0`.

    \return `true` if \p s contains only the character \p c, otherwise `false`.
*/
bool colr_str_is_all(const char* s, const char c) {
    if (!(s && c)) return false;
    size_t i = 0;
    while (s[i]) {
        if (s[i] != c) return false;
        i++;
    }
    return true;
}

/*! Determines if a \string is composed entirely of escape codes.

    \details
    Returns `false` if the string is `NULL`, or empty.

    \pi s   \parblock
                The string to check.
                \mustnullin
            \endparblock
    \return `true` if the string is escape-codes only, otherwise `false`.

    \sa colr_str_has_codes
*/
bool colr_str_is_codes(const char* s) {
    if (!s) return false;
    if (s[0] == '\0') return false;
    size_t i = 0;
    while (s[i]) {
        if (s[i] == '\x1b') {
            // Skip past the code.
            while (!colr_char_is_code_end(s[i++]));
            continue;
        }
        // Found a non-escape-code char.
        return false;
    }
    return true;
}
/*! Determines whether all characters in a \string are digits.

    \details
    If \p s is NULL or an empty string (`""`), `false` is returned.

    \pi s   \parblock
                String to check.
                \mustnullin
            \endparblock
    \return `true` if all characters are digits (0-9), otherwise `false`.
*/
bool colr_str_is_digits(const char* s) {
    if (!s) return false;
    if (s[0] == '\0') return false;

    size_t i = 0;
    while (s[i]) {
        if (!isdigit(s[i])) return false;
        i++;
    }
    return true;
}


/*! Left-justifies a \string, ignoring escape codes when measuring the width.

    \pi s       \parblock
                    The string to justify.
                    \mustnullin
                \endparblock
    \pi width   The overall width for the resulting string.
                If set to '0', the terminal width will be used from colr_term_size().
    \pi padchar The character to pad with. If '0', then `' '` is used.

    \return     \parblock
                    An allocated string with the result, or `NULL` if \p s is `NULL`.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa colr_str_center
    \sa colr_str_rjust
    \sa colr_term_size

    \examplecodefor{colr_str_ljust,.c}
    char* colorized = colr("This.", fore(RED), back(WHITE));
    char* justified = colr_str_ljust(colorized, 8, ' ');
    free(colorized);
    // The string still has codes, but only 3 spaces were added.
    assert(colr_str_ends_with(justified, "   "));
    // It was "justified" to 8 characters long, but it is well over that.
    assert(strlen(justified) > 8);
    printf("'%s'\n", justified);
    free(justified);
    \endexamplecode
*/
char* colr_str_ljust(const char* s, int width, const char padchar) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = colr_str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    char* start = result;
    sprintf(result, "%s", s);
    int pos = 0;
    while (pos < diff) {
        result[length + pos++] = pad;
    }
    return start;
}

/*! Converts a \string into lower case in place.
    \details
    \mustnullin

    \details
    If `s` is `NULL`, nothing is done.

    \pi s The input string to convert to lower case.
*/
void colr_str_lower(char* s) {
    if (!s) return;
    size_t i = 0;
    while (s[i]) {
        char c = tolower(s[i]);
        s[i] = c;
        i++;
    }
    // This works for empty strings too.
    if (s[i] != '\0') s[i] = '\0';
}


/*! Strip a leading character from a \string, filling another \string with the
    result.

    \details
    \p dest and \p s should not overlap.

    \po dest   Destination `char` array. Must have room for `strlen(s) + 1`.
    \pi s      String to strip the character from.
    \pi length Length of \p s, the input string.
    \pi c      Character to strip. If set to `0`, all whitespace characters will
               be used (' ', '@\n', '@\t', '@\v', '@\f', '@\r').
    \return    The number of \p c characters removed.
               May return `0` if \p s is `NULL`/empty, \p dest is `NULL`.
*/
size_t colr_str_lstrip(char* restrict dest, const char* restrict s, size_t length, const char c) {
    if (!(s && dest)) return 0;
    if (s[0] == '\0') return 0;
    bool start = true;
    size_t pos = 0;
    size_t char_cnt = 0;
    for (size_t i = 0; i < length && s[i]; i++) {
        if (start && ((c && (s[i] == c)) || ((!c) && isspace(s[i])))) {
            char_cnt++;
            continue;
        }
        start = false;
        dest[pos] = s[i];
        pos++;
    }
    dest[pos] = '\0';
    return char_cnt;
}

/*! Strips a leading character from a \string, and allocates a new string with
    the result.

    \pi s      String to strip the character from.
    \pi c      Character to strip. If set to `0`, all whitespace characters will
               be used (' ', '@\n', '@\t').
    \return    \parblock
                   An allocated string with the result.
                   May return `NULL` if \p s is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

*/
char* colr_str_lstrip_char(const char* s, const char c) {
    if (!s) return NULL;
    if (s[0] == '\0') return NULL;

    size_t length = strlen(s);
    size_t finallen = length - colr_str_char_lcount(s, c);
    char* dest = calloc(finallen + 1, sizeof(char));
    if (!dest) return NULL;
    // All characters were removed? Return the empty string.
    if (finallen == 1) return dest;
    colr_str_lstrip(dest, s, length, c);
    return dest;
}

/*! Removes certain characters from the start of a \string and allocates
    a new string with the result.

    \details
    The order of the characters in \p chars does not matter. If any of them
    are found at the start of a string, they will be removed.

    `colr_str_lstrip_chars("aabbccTEST", "bca") == "TEST"`

    \details
    \p s and \p chars must not overlap.

    \pi s     \parblock
                    The string to strip.
                    \p s \mustnull
              \endparblock
    \pi chars \parblock
                    A string of characters to remove. Each will be removed from
                    the start of the string.
                    \p chars \mustnull
              \endparblock
    \return   \parblock
                    An allocated string with the result.
                    May return NULL if  \p s or \p chars is NULL.
                    \mustfree
                    \maybenullalloc
              \endparblock
*/
char* colr_str_lstrip_chars(const char* restrict s, const char* restrict chars) {
    if (!(s && chars)) return NULL;
    if ((s[0] == '\0') || (chars[0] == '\0')) return NULL;

    // The string may not even contain the `chars`. Worst case is the same
    // string, with the same length.
    size_t length = strlen(s);
    size_t finallen = length - colr_str_chars_lcount(s, chars);
    char* result = calloc(finallen + 1, sizeof(char));
    if (!result) return NULL;
    // All characters were removed? Return the empty string.
    if (finallen == 1) return result;
    size_t result_pos = 0;
    bool done_trimming = false;
    for (size_t i = 0; i < length; i++) {
        if ((!done_trimming) && colr_char_in_str(chars, s[i])) {
            continue;
        } else {
            // First non-`chars` character. We're done.
            done_trimming = true;
        }
        result[result_pos] = s[i];
        result_pos++;
    }
    return result;
}


/*! Returns the number of characters in a \string, taking into account possibly
    multibyte characters.

    \pi s The string to get the length of.
    \return The number of characters, single and multibyte, or `0` if \p s is
            `NULL`, empty, or has invalid multibyte sequences.

    \sa colr_mb_len

    \examplecodefor{colr_str_mb_len,.c}
    #include "colr.h"
    int main(void) {
        char* s = "１３３７";
        // There are 4 multibyte characters in the string.
        assert(colr_str_mb_len(s) == 4);
        // There are multiple bytes making up each characters, probably 3 each,
        // which makes the byte-length 12.
        assert(strlen(s) > 4);
    }
    \endexamplecode
*/
size_t colr_str_mb_len(const char* s) {
    if ((!s) || (s[0] == '\0')) return 0;
    colr_set_locale();
    int i = 0;
    int next_len = 0;
    size_t total = 0;
    mbstate_t st;
    memset(&st, 0, sizeof(mbstate_t));
    while ((next_len = mbrlen(s + i, MB_LEN_MAX, &st))) {
        if (next_len < 0) {
            #if defined(COLR_DEBUG) && !defined(COLR_TEST)
                // Make sure this is a no-op when not explicitly requesting
                // colrc-debug-mode. It helps to know which part was "invalid".
                char* repr = colr_repr(s + i);
                dbug("Invalid multibyte sequence at: %d, Repr: %s\n", i, repr ? repr : "NULL");
                if (repr) free(repr);
            #endif
            return 0;
        }
        i += next_len;
        total++;
    }
    return total;
}

/*! Returns the length of \string, ignoring escape codes and the the null-terminator.

    \pi s   \parblock
                String to get the length for.
                \mustnullin
            \endparblock
    \return The length of the string, as if it didn't contain escape codes.
            For non-escape-code strings, this is like `strlen()`.
            For `NULL` or "empty" strings, `0` is returned.

    \sa colr_str_strip_codes
*/
size_t colr_str_noncode_len(const char* s) {
    if (!s) return 0;
    if (s[0] == '\0') return 0;
    size_t i = 0, total = 0;
    while (s[i]) {
        if (s[i] == '\x1b') {
            // Skip past the code.
            while (!colr_char_is_code_end(s[i++]));
            continue;
        }
        i++;
        total++;
    }
    return total;
}

/*! Returns all `regmatch_t` matches for regex pattern in a \string.

    \pi s         The string to search.
    \pi repattern The pattern to look for.
    \return       \parblock
                    A pointer to an allocated array of `regmatch_t*`,
                    or `NULL` if \p s is `NULL` or \p repattern is `NULL`.
                    The last member is always `NULL`.
                    \mustfree
                  \endparblock

    \examplecodefor{colr_re_matches,.c}
        #include "colr.h"
        int main(void) {
            // Compile a regex pattern.
            regex_t pat;
            if (regcomp(&pat, "foo", 0)) {
                regfree(&pat);
                return EXIT_FAILURE;
            }
            // The string to operate on.
            const char* s = "This foo is a foo string, foo?";
            // Get all matches.
            regmatch_t** matches = colr_re_matches(s, &pat);
            if (!matches) {
                // This should be impossible (for this example).
                fprintf(stderr, "No matches?\n");
                return EXIT_FAILURE;
            }
            puts(s);
            // `colr_re_matches` always leaves `NULL` as the last member.
            size_t cnt = 0;
            while (matches[cnt]) {
                printf("Found a match at: %d-%d\n", matches[cnt]->rm_so, matches[cnt]->rm_eo);
                cnt++;
            }
            // Free all of your resources.
            colr_free(matches);
            regfree(&pat);
        }
    \endexamplecode
*/
regmatch_t** colr_re_matches(const char* s, regex_t* repattern) {
    if (!(s && repattern)) return NULL;

    size_t num_matches_all = strlen(s);
    size_t num_submatches = 1;
    regmatch_t match_current[num_submatches];
    regmatch_t** matches = calloc(num_matches_all + 1, sizeof(regmatch_t));
    int ret = regexec(repattern, s, num_submatches, match_current, 0);
    if (ret) {
        // No match found.
        free(matches);
        return NULL;
    }
    size_t index = 0;
    while (ret == 0) {
        // Found a match, add it to the collection.
        matches[index++] = colr_alloc_regmatch(match_current[0]);
        size_t offset_diff = match_current[0].rm_eo;
        const char* rest = s + offset_diff;
        ret = regexec(repattern, rest, num_submatches, match_current, 0);
        match_current[0].rm_so = match_current[0].rm_so + offset_diff;
        match_current[0].rm_eo = match_current[0].rm_eo + offset_diff;
    }
    // Mark the end.
    free(matches[index]);
    matches[index++] = NULL;
    // Free any unused matches.
    while (index <= num_matches_all) {
        free(matches[index]);
        matches[index++] = NULL;
    }

    return matches;
}

/*! Replaces the first substring found in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    For a more dynamic version, see the colr_replace and colr_replace_re macros.

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The string to replace with.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace,.c}
    #include "colr.h"

    int main(void) {
        char* mystring = "This is a foo foo foo line.";
        char* replaced = colr_str_replace_cnt(mystring, "foo", "replaced", 2);
        if (!replaced) {
            fprintf(stderr, "Failed to allocate for new string!\n");
            return EXIT_FAILURE;
        }
        printf("%s\n", replaced);
        // Don't forget to free the new string.
        free(replaced);
        return EXIT_SUCCESS;
    }
    \endexamplecodefor
*/
char* colr_str_replace(const char* restrict s, const char* restrict target, const char* restrict repl) {
    return colr_str_replace_cnt(s, target, repl, 1);
}

/*! Replaces the first substring found in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    For a more dynamic version, see the colr_replace and colr_replace_re macros.

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The string to replace with.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_all,.c}
    #include "colr.h"

    int main(void) {
        char* mystring = "This is a foo foo foo line.";
        char* replaced = colr_str_replace_all(mystring, "foo", "replaced");
        if (!replaced) {
            fprintf(stderr, "Failed to allocate for new string!\n");
            return EXIT_FAILURE;
        }
        printf("%s\n", replaced);
        // Don't forget to free the new string.
        free(replaced);
        return EXIT_SUCCESS;
    }
    \endexamplecodefor
*/
char* colr_str_replace_all(const char* restrict s, const char* restrict target, const char* restrict repl) {
    return colr_str_replace_cnt(s, target, repl, 0);
}

/*! Replace all substrings in a \string with a ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorArg to produce escape-codes to replace with.
               ColorArg_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_all_ColorArg(const char* restrict s, const char* restrict target, ColorArg* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_all(s, target, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace all substrings in a \string with a ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorResult to produce escape-codes to replace with.
               ColorResult_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_all_ColorResult(const char* restrict s, const char* restrict target, ColorResult* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_all(s, target, replstr);
    ColorResult_free(repl);
    return result;
}
/*! Replace all substrings in a \string with a ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorText to produce text/escape-codes to replace with.
               ColorText_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is
                   `NULL`/empty, or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_all_ColorText(const char* restrict s, const char* restrict target, ColorText* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_all(s, target, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replaces one or more substrings in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    For a more dynamic version, see the colr_replace and colr_replace_re macros.

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The string to replace with.
    \pi count  Number of substrings to replace, or `0` to replace all substrings.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_cnt,.c}
    #include "colr.h"

    int main(void) {
        char* mystring = "This is a foo foo foo line.";
        char* replaced = colr_str_replace_cnt(mystring, "foo", "replaced", 2);
        if (!replaced) {
            fprintf(stderr, "Failed to allocate for new string!\n");
            return EXIT_FAILURE;
        }
        printf("%s\n", replaced);
        // Don't forget to free the new string.
        free(replaced);
        return EXIT_SUCCESS;
    }
    \endexamplecodefor
*/
char* colr_str_replace_cnt(const char* restrict s, const char* restrict target, const char* restrict repl, int count) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    if (!repl) repl = "";
    // Keeps track of the target strings.
    char* ins = (char*)s;
    // Count the number of replacements needed, by using strstr and skipping
    // past the targets for each call.
    size_t target_len = strlen(target);
    if (count < 1) {
        size_t actual_count;
        for (actual_count = 0; (ins = strstr(ins, target)); ++actual_count) ins += target_len;
        count = actual_count;
    }

    size_t repl_len = strlen(repl);
    size_t extra_chars = repl_len >= target_len ? (repl_len - target_len) : 0;

    char* result;
    // Pointer for writing results to each position in the string.
    char* tmp;
    tmp = result = calloc(
        strlen(s) + (extra_chars * count) + 1,
        sizeof(char)
    );
    if (!result) return NULL;

    // Start writing replacements.
    while (count--) {
        // Find the next target.
        ins = strstr(s, target);
        // Write everything before the target.
        size_t front_len = ins - s;
        tmp = strncpy(tmp, s, front_len);
        // Write the replacement to the end of `tmp`.
        tmp += front_len;
        tmp = strcpy(tmp, repl);
        tmp += repl_len;
        // Skip past our last writes.
        s += front_len + target_len;
    }
    // Write any remaining characters.
    strcpy(tmp, s);
    return result;
}

/*! Replace a substring in a \string with a ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorArg to produce escape-codes to replace with.
               ColorArg_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_ColorArg(const char* restrict s, const char* restrict target, ColorArg* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace(s, target, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace a substring in a \string with a ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorResult to produce escape-codes to replace with.
               ColorResult_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is `NULL`/empty,
                   or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_ColorResult(const char* restrict s, const char* restrict target, ColorResult* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace(s, target, replstr);
    ColorResult_free(repl);
    return result;
}
/*! Replace a substring in a \string with a ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s      The string to operate on.
    \pi target The string to replace.
    \pi repl   The ColorText to produce text/escape-codes to replace with.
               ColorText_free() is called after the replacement is done.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is
                   `NULL`/empty, or \p target is `NULL`/empty.
                   \mustfree
                   \maybenullalloc
               \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_ColorText(const char* restrict s, const char* restrict target, ColorText* repl) {
    if (!(s && target)) return NULL;
    if ((s[0] == '\0') || (target[0] == '\0')) return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace(s, target, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replaces a substring from a regex pattern \string in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The string to replace with.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern

                     doesn't compile/match.
                     \maybenullalloc
                 \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re,.c}
    #include "colr.h"

    int main(void) {
        char* mystring = "This is a foo line.";
        char* replaced = colr_str_replace_re(mystring, "foo", "replaced", 0);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
        replaced = colr_str_replace_re(mystring, "^([^ ]+)", "That", 0);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
    }
    \endexamplecode

*/
char* colr_str_replace_re(const char* restrict s, const char* restrict pattern, const char* restrict repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if ((s[0] == '\0') || (pattern[0] == '\0')) return NULL;
    regex_t repat;
    // Always use extended regex.
    if (!(re_flags & REG_EXTENDED)) re_flags = re_flags | REG_EXTENDED;
    if (regcomp(&repat, pattern, re_flags)) {
        return NULL;
    }
    char* result = colr_str_replace_re_pat(s, &repat, repl);
    regfree(&repat);
    return result;
}

/*! Replaces all substrings from a regex pattern \string in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The string to replace with.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern

                     doesn't compile/match.
                     \maybenullalloc
                 \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re_all,.c}
    #include "colr.h"

    int main(void) {
        char* mystring = "This is a foo bar line.";
        char* replaced = colr_str_replace_re_all(mystring, "(foo)|(bar)", "replaced", 0);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
        replaced = colr_str_replace_re_all(mystring, "^([^ ]+)", "That", 0);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
    }
    \endexamplecode

*/
char* colr_str_replace_re_all(const char* restrict s, const char* restrict pattern, const char* restrict repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if ((s[0] == '\0') || (pattern[0] == '\0')) return NULL;
    regex_t repat;
    // Always use extended regex.
    if (!(re_flags & REG_EXTENDED)) re_flags = re_flags | REG_EXTENDED;
    if (regcomp(&repat, pattern, re_flags)) {
        return NULL;
    }
    char* result = colr_str_replace_re_pat_all(s, &repat, repl);
    regfree(&repat);
    return result;
}

/*! Replace all substrings from a regex pattern \string in a \string with a
    ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex pattern to compile.
    \pi repl     The ColorArg to produce escape-codes to replace with.
                 ColorArg_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_all_ColorArg(const char* restrict s, const char* restrict pattern, ColorArg* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re_all(s, pattern, replstr, re_flags);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace all substrings from a regex pattern \string in a \string with a
    ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The ColorResult to produce escape-codes to replace with.
                 ColorResult_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_all_ColorResult(const char* restrict s, const char* restrict pattern, ColorResult* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re_all(s, pattern, replstr, re_flags);
    ColorResult_free(repl);
    return result;
}

/*! Replace all substrings from a regex pattern \string in a \string with a
    ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The ColorText to produce text/escape-codes to replace with.
                 ColorText_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_all_ColorText(const char* restrict s, const char* restrict pattern, ColorText* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re_all(s, pattern, replstr, re_flags);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replace substrings from a regex pattern \string in a \string with a
    ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex pattern to compile.
    \pi repl     The ColorArg to produce escape-codes to replace with.
                 ColorArg_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_ColorArg(const char* restrict s, const char* restrict pattern, ColorArg* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re(s, pattern, replstr, re_flags);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace substrings from a regex pattern \string in a \string with a
    ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The ColorResult to produce escape-codes to replace with.
                 ColorResult_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_ColorResult(const char* restrict s, const char* restrict pattern, ColorResult* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re(s, pattern, replstr, re_flags);
    ColorResult_free(repl);
    return result;
}
/*! Replace substrings from a regex pattern \string in a \string with a
    ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi pattern  The regex match object to find text to replace.
    \pi repl     The ColorText to produce text/escape-codes to replace with.
                 ColorText_free() is called after the replacement is done.
    \pi re_flags Flags for `regcomp()`. `REG_EXTENDED` is always used, whether
                 flags are provided or not.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p pattern is `NULL`, or the regex pattern
                     doesn't compile/match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_ColorText(const char* restrict s, const char* restrict pattern, ColorText* repl, int re_flags) {
    if (!(s && pattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re(s, pattern, replstr, re_flags);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replaces substrings from a single regex match (`regmatch_t*`) in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s     The string to operate on.
    \pi match The regex match object to find text to replace.
    \pi repl  The string to replace with.
    \return   \parblock
                  An allocated string with the result, or `NULL` if \p s is
                  `NULL`/empty, \p match is `NULL`, or the regex pattern
                  doesn't match.
                  \mustfree
                  \maybenullalloc
              \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re_match,.c}
    #include "colr.h"

    int main(void) {
        // Build a regex pattern.
        regex_t pat;
        if (regcomp(&pat, "foo", 0)) {
            // Failed to compile the pattern.
            regfree(&pat);
            return EXIT_FAILURE;
        }

        // Set up our matches (only 1 can be used with colr_replace_re_match).
        size_t matchcnt = 1;
        regmatch_t rematches[matchcnt];

        // Run the regex and we should get an initialized `regmatch_t`.
        char* mystring = "This is a foo line.";
        if (regexec(&pat, mystring, matchcnt, rematches, 0)) {
            fprintf(stderr, "No matches!?");
            return EXIT_FAILURE;
        }

        // Replace the matched text with our string.
        char* replaced = colr_str_replace_re_match(
            mystring,
            rematches,
            "REPLACED"
        );
        // Don't forget to free your regex object (even if stack-allocated).
        regfree(&pat);
        if (!replaced) {
                fprintf(stderr, "Cannot allocate for replaced string!\n");
                return EXIT_FAILURE;
        }
        printf("%s\n", replaced);
        free(replaced);
        return EXIT_SUCCESS;
    }
    \endexamplecode
*/
char* colr_str_replace_re_match(const char* restrict s, regmatch_t* match, const char* restrict repl) {
    if (!(s && match)) return NULL;
    if (s[0] == '\0') return NULL;
    if (!repl) repl = "";
    char* result = NULL;
    if (match->rm_so > 0) {
        // Starting in the middle of the string.
        char line_begin[match->rm_so + 1];
        strncpy(line_begin, s, match->rm_so);
        line_begin[match->rm_so] = '\0';
        asprintf_or_return(NULL, &result, "%s%s%s", line_begin, repl, s + match->rm_eo);
    } else {
        // Replace the beginning of the string.
        int written = asprintf(&result, "%s%s", repl, s + match->rm_eo);
        if (written < 0) {
            // Cannot allocate.
            return NULL; // LCOV_EXCL_LINE
        } else if (written < 1) {
            // Happens when "removing" patterns by replacing with "" and
            // the entire string gets replaced (`result` is untouched, and `NULL`).
            if (result) free(result);
            result = colr_empty_str();
        }
    }
    return result;
}

/*! Replaces substrings from a regex match (`regmatch_t*`) in a \string.

    \details
    This modifies `target` in place. It must have capacity for the result.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi ref    \parblock
                    The string to use for offset references. Can be `target`.
                    Set this to the source string if `target` has not been filled
                    yet. If `target` has been filled, you may use `target` for
                    both `ref` and `target`.
                \endparblock
    \po target \parblock
                    The string to modify.
                    Must have room for the resulting string.
               \endparblock
    \pi match  The regex match object to find text to replace.
    \pi repl   The string to replace with.
    \return    \parblock
                   An allocated string with the result, or `NULL` if \p s is
                   `NULL`/empty, \p match is `NULL`, or the regex pattern
                   doesn't match.
                   \mustfree
                   \maybenullalloc
               \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re_match_i,.c}
    #include "colr.h"

    int main(void) {
        // Build a regex pattern.
        regex_t pat;
        if (regcomp(&pat, "foo", 0)) {
            // Failed to compile the pattern.
            regfree(&pat);
            return EXIT_FAILURE;
        }

        // Set up our matches (only 1 can be used with colr_replace_re_match_i).
        size_t matchcnt = 1;
        regmatch_t rematches[matchcnt];

        // Run the regex and we should get an initialized `regmatch_t`.
        char* mystring = "This is a foo line.";
        char* replacement = "REPLACED";
        size_t length = strlen(mystring);
        char* targetstring = calloc(length + strlen(replacement) + 1, sizeof(char));
        strncpy(targetstring, mystring, length + 1);
        if (regexec(&pat, mystring, matchcnt, rematches, 0)) {
            fprintf(stderr, "No matches!?");
            return EXIT_FAILURE;
        }
        // Don't forget to free your regex object (even if stack-allocated).
        regfree(&pat);

        // Replace the matched text with our string.
        char* replaced = colr_str_replace_re_match_i(
            mystring,
            targetstring,
            rematches,
            replacement
        );
        if (!replaced) {
                fprintf(stderr, "An error occurred!\n");
                return EXIT_FAILURE;
        }
        printf("%s\n", replaced);
        free(targetstring);
        return EXIT_SUCCESS;
    }
    \endexamplecode
*/
char* colr_str_replace_re_match_i(const char* restrict ref, char* target, regmatch_t* match, const char* restrict repl) {
    if (!match) return NULL;
    size_t repl_len = 0;
    if (repl) {
        repl_len = strlen(repl);
    } else {
        // Callers of this function already take care of this.
        // This is only here as a fail-safe.
        repl = ""; // LCOV_EXCL_LINE
    }
    size_t end_len = strlen(ref) - match->rm_eo;
    char line_end[end_len + 1];
    strncpy(line_end, ref + match->rm_eo, end_len);
    line_end[end_len] = '\0';
    if (match->rm_so > 0) {
        // Starting in the middle of the string.
        char line_begin[match->rm_so + 1];
        strncpy(line_begin, ref, match->rm_so);
        line_begin[match->rm_so] = '\0';
        int expected_len = strlen(line_begin) + repl_len + strlen(line_end) + 1;
        int written =snprintf(
            target,
            expected_len,
            "%s%s%s",
            line_begin,
            repl,
            line_end
        );
        if (written >= expected_len) {
            return NULL;
        }
    } else {
        // Replace the beginning of the string.
        int expected_len = repl_len + strlen(line_end) + 1;
        int written = snprintf(
            target,
            expected_len,
            "%s%s",
            repl,
            line_end
        );
        target[expected_len] = '\0';
        if (written == 0) {
            // Happens when "removing" patterns by replacing with "" and
            // the entire string gets replaced (`result` is untouched, and `NULL`).
            target[0] = '\0';
            return target;
        } else if (written >= expected_len) {
            // Miscalculated length.
            return NULL; // LCOV_EXCL_LINE
        }
    }
    return target;
}

/*! Replaces substrings from an array of regex match (`regmatch_t*`) in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s        The string to operate on.
    \pi matches  \parblock
                    Regex match objects to find text to replace.
                    The array must have `NULL` as the last member.
                 \endparblock
    \pi repl     The string to replace with.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re
*/
char* colr_str_replace_re_matches(const char* restrict s, regmatch_t** matches, const char* restrict repl) {
    if (!(s && matches)) return NULL;
    if (s[0] == '\0') return NULL;
    size_t final_len = 0;
    // Count matches, this better be NULL-terminated!
    size_t cnt = 0;
    while (matches[cnt]) cnt++;

    if (repl) {
        size_t repl_len = strlen(repl);
        final_len = strlen(s) + (repl_len * cnt);
    } else {
        repl = "";
        final_len = strlen(s);
    }

    char* result = calloc(final_len + 1, sizeof(char));
    // Using the original string as a reference for the first time around.
    const char* using = s;
    // Loop through the matches backwards, so starting/ending offsets don't change.
    cnt--;
    while (cnt > 0) {
        regmatch_t* match = matches[cnt];
        if (!match) break;
        char* replaced = colr_str_replace_re_match_i(using, result, match, repl);
        if (!replaced) {
            // LCOV_EXCL_START
            // Could happen if `result` is not big enough.
            free(result);
            return NULL;
            // LCOV_EXCL_STOP
        }
        // We need to reference the result now, to pick up line changes.
        using = result;
        cnt--;
    }
    if (!matches[0]) return result;
    char* finalrepl = colr_str_replace_re_match_i(using, result, matches[0], repl);
    if (!finalrepl) {
        // LCOV_EXCL_START
        // Could happen if `result` is not big enough.
        free(result);
        return NULL;
        // LCOV_EXCL_STOP
    }
    return result;
}

/*! Replace substrings from an array of regex matches (`regmatch_t**`) in a \string with a
    ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi matches  The regex match objects to find text to replace.
    \pi repl     The ColorArg to produce escape-codes to replace with.
                 ColorArg_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_matches_ColorArg(const char* restrict s, regmatch_t** matches, ColorArg* repl) {
    if (!(s && matches)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re_matches(s, matches, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace substrings from an array of regex matches (`regmatch_t**`) in a \string with a
    ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi matches  The regex match objects to find text to replace.
    \pi repl     The ColorResult to produce escape-codes to replace with.
                 ColorResult_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_matches_ColorResult(const char* restrict s, regmatch_t** matches, ColorResult* repl) {
    if (!(s && matches)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re_matches(s, matches, replstr);
    ColorResult_free(repl);
    return result;
}

/*! Replace substrings from an array of regex matches (`regmatch_t**`) in a \string
    with a ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi matches  The regex match objects to find text to replace.
    \pi repl     The ColorText to produce text/escape-codes to replace with.
                 ColorText_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_matches_ColorText(const char* restrict s, regmatch_t** matches, ColorText* repl) {
    if (!(s && matches)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re_matches(s, matches, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replace substrings from a regex match (`regmatch_t*`) in a \string with a
    ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi match    The regex match object to find text to replace.
    \pi repl     The ColorArg to produce escape-codes to replace with.
                 ColorArg_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_match_ColorArg(const char* restrict s, regmatch_t* match, ColorArg* repl) {
    if (!(s && match)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re_match(s, match, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace substrings from a regex match (`regmatch_t*`) in a \string with a
    ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi match    The regex match object to find text to replace.
    \pi repl     The ColorResult to produce escape-codes to replace with.
                 ColorResult_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_match_ColorResult(const char* restrict s, regmatch_t* match, ColorResult* repl) {
    if (!(s && match)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re_match(s, match, replstr);
    ColorResult_free(repl);
    return result;
}

/*! Replace substrings from a regex match (`regmatch_t*`) in a \string with a
    ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s        The string to operate on.
    \pi match    The regex match object to find text to replace.
    \pi repl     The ColorText to produce text/escape-codes to replace with.
                 ColorText_free() is called after the replacement is done.
    \return      \parblock
                     An allocated string with the result, or `NULL` if \p s is
                     `NULL`/empty, \p match is `NULL`, or the regex pattern
                     doesn't match.
                     \mustfree
                     \maybenullalloc
                 \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_match_ColorText(const char* restrict s, regmatch_t* match, ColorText* repl) {
    if (!(s && match)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re_match(s, match, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replaces regex patterns in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The string to replace with.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re_pat,.c}
    #include "colr.h"

    int main(void) {
        // Compile a regex pattern.
        regex_t pat;
        if (regcomp(&pat, "foo", 0)) {
            // Failed to compile the pattern.
            regfree(&pat);
            return EXIT_FAILURE;
        }
        char* mystring = "This is a foo line.";
        char* replaced = colr_str_replace_re_pat(mystring, &pat, "replaced");
        // Don't forget to free your regex pattern.
        regfree(&pat);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
    }
    \endexamplecode
*/
char* colr_str_replace_re_pat(const char* restrict s, regex_t* repattern, const char* restrict repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    if (!repl) repl = "";

    size_t num_matches = 1;
    regmatch_t matches[num_matches];
    if (regexec(repattern, s, num_matches, matches, 0)) {
        // No match found.
        return NULL;
    }
    return colr_str_replace_re_match(s, &matches[0], repl);
}

/*! Replaces all matches to a regex pattern in a \string.

    \details
    Using `NULL` as a replacement is like using an empty string (""), which
    removes the \p target string from \p s.

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The string to replace with.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock


    \sa colr_replace
    \sa colr_replace_re

    \examplecodefor{colr_str_replace_re_pat_all,.c}
    #include "colr.h"

    int main(void) {
        // Compile a regex pattern.
        regex_t pat;
        if (regcomp(&pat, "[1-9]", REG_EXTENDED)) {
            // Failed to compile the pattern.
            fprintf(stderr, "Cannot compile the pattern!\n");
            regfree(&pat);
            return EXIT_FAILURE;
        }
        char* mystring = "1 2 everywhere, but still no 3 to spare.";
        char* replaced = colr_str_replace_re_pat_all(mystring, &pat, "numbers");
        // Don't forget to free your regex pattern.
        regfree(&pat);
        if (!replaced) return EXIT_FAILURE;
        puts(replaced);
        free(replaced);
    }
    \endexamplecode
*/
char* colr_str_replace_re_pat_all(const char* restrict s, regex_t* repattern, const char* restrict repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    if (!repl) repl = "";
    regmatch_t** matches = colr_re_matches(s, repattern);
    if (!matches) return NULL;

    // Found all matches, now replace them.
    char* result = colr_str_replace_re_matches(s, matches, repl);
    // Free all of the matches.
    colr_free_re_matches(matches);
    matches = NULL;
    return result;
}

/*! Replace all matches to a regex pattern in a \string with a ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorArg to produce escape-codes to replace with.
                  ColorArg_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_all_ColorArg(const char* restrict s, regex_t* repattern, ColorArg* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re_pat_all(s, repattern, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace all matches to a regex pattern in a \string with a ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorResult to produce escape-codes to replace with.
                  ColorResult_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_all_ColorResult(const char* restrict s, regex_t* repattern, ColorResult* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re_pat_all(s, repattern, replstr);
    ColorResult_free(repl);
    return result;
}

/*! Replace all matches to a regex pattern in a \string with a ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorText to produce text/escape-codes to replace with.
                  ColorText_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_all_ColorText(const char* restrict s, regex_t* repattern, ColorText* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re_pat_all(s, repattern, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Replace regex patterns in a \string with a ColorArg's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorArg to produce escape-codes to replace with.
                  ColorArg_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_ColorArg(const char* restrict s, regex_t* repattern, ColorArg* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorArg_to_esc(*repl): NULL;
    char* result = colr_str_replace_re_pat(s, repattern, replstr);
    if (replstr) free(replstr);
    ColorArg_free(repl);
    return result;
}

/*! Replace regex patterns in a \string with a ColorResult's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorResult to produce escape-codes to replace with.
                  ColorResult_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_ColorResult(const char* restrict s, regex_t* repattern, ColorResult* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorResult_to_str(*repl): NULL;
    char* result = colr_str_replace_re_pat(s, repattern, replstr);
    ColorResult_free(repl);
    return result;
}

/*! Replace regex patterns in a \string with a ColorText's string result.
    \details
    Using `NULL` as a replacement is like using an empty string ("").

    \pi s         The string to operate on.
    \pi repattern The regex pattern to match (`regex_t*`).
    \pi repl      The ColorText to produce text/escape-codes to replace with.
                  ColorText_free() is called after the replacement is done.
    \return       \parblock
                      An allocated string with the result, or `NULL` if \p s is
                      `NULL`/empty, \p repattern is `NULL`, or the regex pattern
                      doesn't match.
                      \mustfree
                      \maybenullalloc
                  \endparblock

    \sa colr_replace
    \sa colr_replace_re

*/
char* colr_str_replace_re_pat_ColorText(const char* restrict s, regex_t* repattern, ColorText* repl) {
    if (!(s && repattern)) return NULL;
    if (s[0] == '\0') return NULL;
    char* replstr = repl ? ColorText_to_str(*repl): NULL;
    char* result = colr_str_replace_re_pat(s, repattern, replstr);
    if (replstr) free(replstr);
    ColorText_free(repl);
    return result;
}

/*! Convert a \string into a representation of a string, by wrapping it in
    quotes and escaping characters that need escaping.

    \details
    If \p s is NULL, then an allocated string containing the string "NULL" is
    returned (without quotes).

    \details
    Escape codes will be escaped, so the terminal will ignore them if the
    result is printed.

    \pi     s The string to represent.
    \return \parblock
                An allocated string with the representation.
                \mustfree
                \maybenullalloc
            \endparblock

    \sa colr_char_should_escape
    \sa colr_char_escape_char

    \examplecodefor{colr_str_repr,.c}
    char* s = colr_str_repr("This\nhas \bspecial\tchars.");
    // The string `s` contains an escaped string, it *looks like* the definition,
    // but no real newlines, backspaces, or tabs are in it.
    assert(strcmp(s, "\"This\\nhas \\bspecial\\tchars.\"") == 0);
    free(s);
    \endexamplecode
*/
char* colr_str_repr(const char* s) {
    if (!s) {
        char* nullrepr;
        asprintf_or_return(NULL, &nullrepr, "NULL");
        return nullrepr;
    }
    if (s[0] == '\0') {
        char* emptyrepr;
        asprintf_or_return(NULL, &emptyrepr, "\"\"");
        return emptyrepr;
    }
    size_t length = strlen(s);
    size_t esc_chars = 0;
    size_t i;
    for (i = 0; i < length; i++) {
        if (colr_char_should_escape(s[i])) esc_chars++;
        else if (s[i] == '\x1b') esc_chars += 4;
    }
    size_t repr_length = length + (esc_chars * 2);
    // Make room for the wrapping quotes, and a null-terminator.
    repr_length += 3;
    char *repr = calloc(repr_length + 1, sizeof(char));
    size_t inew = 0;
    repr[0] = '"';
    for (i = 0, inew = 1; i < length; i++) {
        char c = s[i];
        if (colr_char_should_escape(c)) {
            repr[inew++] = '\\';
            repr[inew++] = colr_char_escape_char(c);
        } else if (c == '\x1b') {
            repr[inew++] = '\\';
            repr[inew++] = 'x';
            repr[inew++] = '1';
            repr[inew++] = 'b';
        } else {
            repr[inew++] = c;
        }
    }
    repr[inew] = '"';
    return repr;
}

/*! Right-justifies a \string, ignoring escape codes when measuring the width.

    \pi s       \parblock
                    The string to justify.
                    \mustnullin
                \endparblock
    \pi width   The overall width for the resulting string.
                If set to '0', the terminal width will be used from colr_term_size().
    \pi padchar The character to pad with. If '0', then `' '` is used.

    \return     \parblock
                    An allocated string with the result, or `NULL` if \p s is `NULL`.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa colr_str_center
    \sa colr_str_ljust
    \sa colr_term_size

    \examplecodefor{colr_str_rjust,.c}
    char* colorized = colr("This.", fore(RED), back(WHITE));
    char* justified = colr_str_rjust(colorized, 8, ' ');
    free(colorized);
    // The string still has codes, but only 3 spaces were added.
    assert(colr_str_starts_with(justified, "   "));
    // It was "justified" to 8 characters long, but it is well over that.
    assert(strlen(justified) > 8);
    printf("'%s'\n", justified);
    free(justified);
    \endexamplecode
*/
char* colr_str_rjust(const char* s, int width, const char padchar) {
    if (!s) return NULL;
    char pad = padchar == '\0' ? ' ' : padchar;
    size_t length = strlen(s);
    size_t noncode_len = colr_str_noncode_len(s);
    if (width == 0) {
        TermSize ts = colr_term_size();
        width = ts.columns;
    }
    int diff = width - noncode_len;
    char* result;
    if (diff < 1) {
        // No room for padding, also asprintf can't do empty strings.
        if (s[0] == '\0') return colr_empty_str();
        asprintf_or_return(NULL, &result, "%s", s);
        return result;
    }
    size_t final_len = length + diff + 1;
    result = calloc(final_len, sizeof(char));
    if (!result) return NULL;
    if (s[0] == '\0') {
        // Shortcut for a simple pad-only string.
        memset(result, pad, final_len - 1);
        return result;
    }
    char* start = result;
    int pos = 0;
    while (pos < diff) {
        result[pos++] = pad;
    }
    result = result + pos;
    sprintf(result, "%s", s);
    return start;
}

/*! Checks a \string for a certain prefix substring.

    \details
    `prefix` \mustnull

    \pi s      The string to check.
    \pi prefix The prefix string to look for.

    \return True if the string `s` starts with prefix.
    \return False if one of the strings is null, or the prefix isn't found.

    \examplecodefor{colr_str_starts_with,.c}
    char* s = "This is my string.";
    assert(colr_str_starts_with(s, "This"));
    assert(!colr_str_starts_with(s, "that"));
    \endexamplecode
*/
bool colr_str_starts_with(const char* restrict s, const char* restrict prefix) {
    if (!(s && prefix)) {
        // One of the strings is null.
        return false;
    }
    if ((s[0] == '\0') || (prefix[0] == '\0')) {
        // One of the strings is empty.
        return false;
    }
    size_t pre_len = strlen(prefix);
    for (size_t i = 0; i < pre_len; i++) {
        if (s[i] == '\0') {
            // Reached the end of s before the end of prefix.
            return false;
        }
        if (prefix[i] != s[i]) {
            // Character differs from the prefix.
            return false;
        }
    }
    return true;
}

/*! Strips escape codes from a \string, resulting in a new allocated string.

    \pi s   \parblock
                The string to strip escape codes from.
                \mustnullin
            \endparblock
    \return \parblock
                An allocated string with the result.
                \mustfree
                \maybenullalloc
            \endparblock

    \sa colr_str_noncode_len

    \examplecodefor{colr_str_strip_codes,.c}
    char* s = "\x1b[31mRed text.\x1b[0m";
    printf("With codes: %s\n", s);
    char* stripped = colr_str_strip_codes(s);
    if (!stripped) exit(1);
    assert(!colr_str_has_codes(stripped));
    assert(strlen(stripped) == 9);
    printf("Without codes: %s\n", stripped);
    // Don't forget to free the string:
    free(stripped);
    \endexamplecode
*/
char* colr_str_strip_codes(const char* s) {
    if (!s) return NULL;
    if (s[0] == '\0') return colr_empty_str();
    size_t length = strlen(s);
    char* final = calloc(length + 1, sizeof(char));
    size_t i = 0, pos = 0;
    while (s[i]) {
        if (s[i] == '\x1b') {
            // Skip past the code.
            while (!colr_char_is_code_end(s[i++]));
            continue;
        }
        final[pos++] = s[i++];
    }
    return final;
}

/*! Allocate a new lowercase version of a \string.

    \details
    \mustfree

    \pi s   \parblock
                The input string to convert to lower case.
                \mustnull
            \endparblock
    \return \parblock
                The allocated string, or `NULL` if \p s is `NULL`.
                \mustfree
                \maybenullalloc
            \endparblock
*/
char* colr_str_to_lower(const char* s) {
    if (!s) return NULL;
    size_t length = strlen(s);
    char* out = calloc(length + 1, sizeof(char));
    if (!out) return NULL;
    if (s[0] == '\0') return out;

    size_t i = 0;
    while (s[i]) {
        out[i] = tolower(s[i]);
        i++;
    }
    return out;
}

/*! Determine whether the current environment support RGB (True Colors).

    \details
    This checks `$COLORTERM` for the appropriate value (`'truecolor'` or `'24bit'`).
    On "dumber" terminals, RGB codes are probably ignored or mistaken for a
    256-color or even 8-color value.

    \details
    For instance, RGB is supported in `konsole`, but not in `xterm` or `linux`
    ttys. Using RGB codes in `xterm` makes the colors appear as though a 256-color
    value was used (closest matching value, like RGB_to_term_RGB()).
    Using RGB codes in a simpler `linux` tty makes them appear as though an 8-color
    value was used. Very ugly, but not a disaster.

    \details
    I haven't seen a <em>modern</em> linux terminal spew garbage across the screen
    from using RGB codes when they are not supported, but I could be wrong.
    I would like to see that terminal if you know of one.

    \return `true` if 24-bit (true color, or "rgb") support is detected, otherwise `false`.
*/
bool colr_supports_rgb(void) {
    char* colorterm;
    // Check $COLORTERM for 'truecolor' or '24bit'
    if ((colorterm = getenv("COLORTERM"))) {
        if (colr_istr_either(colorterm, "truecolor", "24bit")) return true;
    }
    // TODO: Send an rgb code, test the terminal response?
    // char* testcode = "\x1b[38:2:255:255:255m\x1bP$qm\x1b\\\n";
    // Should get: 2:255:255:255m
    return false;
}

/*! Same as colr_supports_rgb(), but the environment is only checked on the
    first call. All other calls return the same result as the first call.

    \return `true` if 24-bit (true color, or "rgb") support is detected, otherwise `false`.
*/
bool colr_supports_rgb_static(void) {
    static int support_state = 0;
    // If we've already checked, return the previous result.
    if (support_state) return (support_state == 1);
    // Check for rgb support, and store the result for later calls.
    support_state = colr_supports_rgb() ? 1 : -1;
    return (support_state == 1);
}

/*! Attempts to retrieve the row/column size of the terminal and returns a TermSize.

    \details
    If the call fails, the environment variables `LINES` and `COLUMNS` are checked.
    If that fails, a default `TermSize` struct is returned:
    \code
    (TermSize){.rows=35, .columns=80}
    \endcode

    \return A TermSize struct with terminal size information.
*/
TermSize colr_term_size(void) {
    struct winsize ws = colr_win_size();
    return (TermSize){.rows=ws.ws_row, .columns=ws.ws_col};
}

/*! Attempts to retrieve a `winsize` struct from an `ioctl` call.

    \details
    If the call fails, the environment variables `LINES` and `COLUMNS` are checked.
    If that fails, a default `winsize` struct is returned:
    \code
    (struct winsize){.ws_row=35, .ws_col=80, .ws_xpixel=0, .ws_ypixel=0}
    \endcode

    \details
    `man ioctl_tty` says that `.ws_xpixel` and `.ws_ypixel` are unused.

    \return A `winsize` struct (`sys/ioctl.h`) with window size information.
*/
struct winsize colr_win_size(void) {
    struct winsize ws = {.ws_row=0, .ws_col=0, .ws_xpixel=0, .ws_ypixel=0};
    if (ioctl(0, TIOCGWINSZ, &ws) < 0) {
        // No support? Print a message for debug builds, when not testing.
        #ifndef COLR_TEST
            dbug("No support for ioctl TIOCGWINSZ, using defaults.\n");
        #endif
        return colr_win_size_env();
    }
    return ws;
}

/*! Get window/terminal size using the environment variables `LINES`, `COLUMNS`,
    or `COLS`.

    \details
    This is used as a fallback if the `ioctl()` call fails in colr_win_size().
    If environment variables are not available, a default `winsize` struct is returned:
    \code
    (struct winsize){.ws_row=35, .ws_col=80, .ws_xpixel=0, .ws_ypixel=0}
    \endcode


    \return A `winsize` struct (`sys/ioctl.h`) with window size information.
*/
struct winsize colr_win_size_env(void) {
    char* env_rows = getenv("LINES");
    unsigned short default_rows = 0;
    if (!env_rows || sscanf(env_rows, "%hu", &default_rows) != 1) {
        default_rows = 35;
    }
    char* env_cols = getenv("COLUMNS");
    if (!env_cols) env_cols = getenv("COLS");

    unsigned short default_cols = 0;
    if (!env_cols || sscanf(env_cols, "%hu", &default_cols) != 1) {
        default_cols = 80;
    }
    return (struct winsize){
        .ws_row=default_rows,
        .ws_col=default_cols,
        .ws_xpixel=0,
        .ws_ypixel=0
    };
}

/*! Create an escape code for a background color.

    \po out   Memory allocated for the escape code string.
              <em>Must have enough room for `CODEX_LEN`</em>.
    \pi value BasicValue value to use for background.
*/
void format_bg(char* out, BasicValue value) {
    if (!out) return;
    snprintf(out, CODE_LEN, "\x1b[%dm", BasicValue_to_ansi(BACK, value));
}

/*! Create an escape code for an extended background color.

    \po out Memory allocated for the escape code string.
            <em>Must have enough room for `CODEX_LEN`</em>.
    \pi num Value to use for background.
*/
void format_bgx(char* out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\x1b[48;5;%dm", num);
}

/*! Create an escape code for a true color (rgb) background color
    using values from an RGB struct.

    \po out Memory allocated for the escape code string.
            <em>Must have enough room for `CODE_RGB_LEN`</em>.
    \pi rgb RGB struct to get red, blue, and green values from.
*/
void format_bg_RGB(char* out, RGB rgb) {
    if (!out) return;
    snprintf(out, CODE_RGB_LEN, "\x1b[48;2;%d;%d;%dm", rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values, approximating 256-color values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_bg_RGB_term(char* out, RGB rgb) {
    format_bgx(out, ExtendedValue_from_RGB(rgb));
}

/*! Create an escape code for a fore color.

    \po out   Memory allocated for the escape code string.
              <em>Must have enough room for `CODEX_LEN`</em>.
    \pi value BasicValue value to use for fore.
*/
void format_fg(char* out, BasicValue value) {
    if (!out) return;
    snprintf(out, CODE_LEN, "\x1b[%dm", BasicValue_to_ansi(FORE, value));
}

/*! Create an escape code for an extended fore color.

    \po out Memory allocated for the escape code string.
            <em>Must have enough room for `CODEX_LEN`</em>.
    \pi num Value to use for fore.
*/
void format_fgx(char* out, unsigned char num) {
    if (!out) return;
    snprintf(out, CODEX_LEN, "\x1b[38;5;%dm", num);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB(char* out, RGB rgb) {
    if (!out) return;
    snprintf(out, CODE_RGB_LEN, "\x1b[38;2;%d;%d;%dm", rgb.red, rgb.green, rgb.blue);
}

/*! Create an escape code for a true color (rgb) fore color using an
    RGB struct's values, approximating 256-color values.

    \po out Memory allocated for the escape code string.
    \pi rgb Pointer to an RGB struct.
*/
void format_fg_RGB_term(char* out, RGB rgb) {
    format_fgx(out, ExtendedValue_from_RGB(rgb));
}

/*! Create an escape code for a style.

    \po out   Memory allocated for the escape code string.
              <em>Must have enough room for `STYLE_LEN`</em>.
    \pi style StyleValue value to use for style.
*/
void format_style(char* out, StyleValue style) {
    if (!out) return;
    snprintf(out, STYLE_LEN, "\x1b[%dm", style < 0 ? RESET_ALL: style);
}


/* ---------------------------- ColrC Functions ---------------------------- */
/*! Calls Colr `*_free()` functions for Colr objects, otherwise just calls `free()`.

    \details
    You should use the colr_free() macro instead.

    \warninternal

    \pi p Pointer to a heap-allocated object.
*/
void _colr_free(void* p) {
    if (!p) return;
    if (ColorArg_is_ptr(p)) ColorArg_free(p);
    else if (ColorResult_is_ptr(p)) ColorResult_free(p);
    else if (ColorText_is_ptr(p)) ColorText_free(p);
    else free(p);
}

/*! Determines if a void pointer is _ColrLastArg (the last-arg-marker).

    \warninternal

    \pi p The pointer to check.
    \return `true` if the pointer is _ColrLastArg, otherwise `false`.
*/
bool _colr_is_last_arg(void* p) {
    if (!p) return false;
    // Most likely the very same memory.
    if (p == _ColrLastArg) return true;
    if (!colr_check_marker(_ColrLastArgValue.marker, p)) return false;
    // Check to see if someone allocated their own _ColrLastArgValue.
    struct _ColrLastArg_s* clastp = p;
    return (
        (clastp->marker == _ColrLastArgValue.marker) &&
        (clastp->value == _ColrLastArgValue.value)
    );
}

/*! Joins ColorArgs, ColorTexts, and \strings into one long string separated
    by it's first argument.

    \details
    This will free() any \colrfreestructs that are passed in. It is
    backing the \colrusingmacros, and enables easy throw-away color values.

    \details
    Any plain strings that are passed in are left alone. It is up to the caller
    to free those. ColrC only manages the temporary Colr-based objects needed
    to build up these strings.

    \details
    You should use \colrjoinmacros instead.

    \warninternal

    \pi joinerp The joiner (any ColorArg, ColorResult, ColorText, or string).
    \pi ...     Zero or more ColorArgs, ColorResults, ColorTexts, or strings to
                join by the joiner.
    \return     \parblock
                    An allocated string with mixed escape codes/strings.
                    CODE_RESET_ALL is appended to all ColorText arguments.
                    This allows easy part-colored messages.
                    \mustfree
                    \maybenullalloc
                    Also, `NULL` will be returned if \p joinerp is `NULL`.
                \endparblock
*/
char* _colr_join(void* joinerp, ...) {
    if (!joinerp) {
        return NULL;
    }
    va_list args;
    va_start(args, joinerp);
    va_list argcopy;
    va_copy(argcopy, args);
    size_t length = _colr_join_size(joinerp, argcopy);
    va_end(argcopy);

    char* final = calloc(length, sizeof(char));
    char* joiner;
    ColorArg* joiner_cargp = NULL;
    ColorResult* joiner_cresp = NULL;
    ColorText* joiner_ctextp = NULL;

    bool needs_reset = false;
    if (ColorArg_is_ptr(joinerp)) {
        // It's a ColorArg.
        joiner_cargp = joinerp;
        joiner = ColorArg_to_esc(*joiner_cargp);
        ColorArg_free(joiner_cargp);
        needs_reset = true;
    } else if (ColorResult_is_ptr(joinerp)) {
        joiner_cresp = joinerp;
        joiner = ColorResult_to_str(*joiner_cresp);
    } else if (ColorText_is_ptr(joinerp)) {
        joiner_ctextp = joinerp;
        joiner = ColorText_to_str(*joiner_ctextp);
        ColorText_free(joiner_ctextp);
        // ColorText already has it's own reset code.
    } else {
        // It's a string, or it better be anyway.
        joiner = (char* )joinerp;
    }
    if (!joiner) {
        // This is mostly impossible because of the first NULL check.
        // LCOV_EXCL_START
        free(final);
        va_end(args);
        return NULL;
        // LCOV_EXCL_STOP
    }

    int count = 0;
    ColorArg* cargp = NULL;
    ColorResult* cresp = NULL;
    ColorText* ctextp = NULL;
    char* piece;
    void *arg = NULL;
    while_colr_va_arg(args, void*, arg) {
        if (!arg) continue;
        cargp = NULL;
        cresp = NULL;
        ctextp = NULL;
        // These ColorArgs/ColorTexts were heap allocated through the fore,
        // back, style, and ColrC macros. I'm going to free them, so the user
        // doesn't have to keep track of all the temporary pieces that built
        // this string.
        if (ColorArg_is_ptr(arg)) {
            // It's a ColorArg.
            cargp = arg;
            piece = ColorArg_to_esc(*cargp);
            ColorArg_free(cargp);
            needs_reset = true;
        } else if (ColorResult_is_ptr(arg)) {
            cresp = arg;
            piece = ColorResult_to_str(*cresp);
        } else if (ColorText_is_ptr(arg)) {
            ctextp = arg;
            piece = ColorText_to_str(*ctextp);
            // ColorText already has it's own reset code.
            ColorText_free(ctextp);
        } else {
            // It better be a string.
            piece = (char* )arg;
        }
        if (!piece) continue;
        if (count++) strcat(final, joiner);
        strcat(final, piece);

        // Free the temporary string from those ColorArgs/ColorTexts.
        if (cargp || ctextp) free(piece);
        else if (cresp) ColorResult_free(cresp);
    }
    va_end(args);
    if (!count) {
        // All pieces were NULL/empty, just act like strdup(joiner).
        strcat(final, joiner);
    }
    if (joiner_cargp || joiner_ctextp) {
        free(joiner);
    } else if (joiner_cresp) {
        ColorResult_free(joiner_cresp);
    }
    if (needs_reset) colr_append_reset(final);
    return final;
}

/*! Parse arguments, just as in _colr_join(), but only return the size needed to
    allocate the resulting string.

    \details
    This allows _colr_join() to allocate once, instead of reallocating for each
    argument that is passed.

    \warninternal

    \pi joinerp The joiner (any ColorArg, ColorText, or \string).
    \pi args    A `va_list` with zero or more `ColorArg`s, `ColorText`s, or \strings to join.
    \return     \parblock
                    The length (in bytes) needed to allocate a string built
                    with _colr_cat().
                    This function will return `0` if \p joinerp is `NULL`/empty).
                    Except for `0`, it will never return anything less than
                    `CODE_RESET_LEN`.
                \endparblock

    \sa _colr
*/
size_t _colr_join_size(void* joinerp, va_list args) {
    // No joiner, no strings. NULL will be returned.
    if (!joinerp) return 0;

    size_t length = 0;
    size_t joiner_len = _colr_ptr_length(joinerp);
    if (joiner_len < 1) return 0;

    bool need_join = false;
    void *arg = NULL;
    while_colr_va_arg(args, void*, arg) {
        if (!arg) continue;
        length += _colr_ptr_length(arg);
        if (need_join) {
            length += joiner_len;
        } else {
            // First time through.
            need_join = true;
        }
    }
    length += CODE_RESET_LEN;
    return length;
}

/*! Join an array of \strings, `ColorArg`s, or `ColorText`s by another \string,
    ColorArg, or ColorText.

    \pi joinerp The joiner (any ColorArg, ColorText, or \string).
    \pi ps      An array of pointers to `ColorArg`s, `ColorText`s, or \strings.
                The array must have `NULL` as the last item.
    \return     \parblock
                    An allocated string with the result.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \examplecodefor{colr_join_array,.c}
    char* joiner = " [and] ";
    ColorText* words[] = {
        Colr("this", fore(RED)),
        Colr("that", fore(hex("ff3599"))),
        Colr("the other", fore(BLUE), style(UNDERLINE)),
        NULL
    };
    char* s = colr_join_array(joiner, words);
    if (!s) exit(1);
    printf("%s\n", s);
    free(s);

    // Don't forget to free the ColorTexts/ColorArgs.
    size_t i = 0;
    while (words[i]) ColorText_free(words[i++]);
    \endexamplecode

    \sa colr
    \sa colr_join
    \sa colr_join_arrayn
*/
char* colr_join_array(void* joinerp, void* ps) {
    if (!(joinerp && ps)) return 0;
    size_t length = _colr_join_array_length(ps);
    return colr_join_arrayn(joinerp, ps, length);
}
/*! Join an array of \strings, `ColorArg`s, or `ColorText`s by another \string,
    ColorArg, or ColorText.

    \pi joinerp The joiner (any ColorArg, ColorText, or \string).
    \pi ps      An array of pointers to `ColorArg`s, `ColorText`s, or \strings.
                The array must have at least a length of `count`, unless a
                `NULL` element is placed at the end.
    \pi count   The total number of items in the array.
    \return     \parblock
                    An allocated string with the result.
                    \mustfree
                    \maybenullalloc
                    If any parameter is `NULL`, `NULL` is returned.
                \endparblock

    \examplecodefor{colr_join_arrayn,.c}
    char* joiner = " [and] ";
    ColorText* words[] = {
        Colr("this", fore(RED)),
        Colr("that", fore(hex("ff3599"))),
        Colr("the other", fore(BLUE), style(UNDERLINE))
    };
    // This only works for actual arrays, not malloc'd stuff.
    size_t arr_length = sizeof(words) / sizeof(words[0]);
    char* s = colr_join_arrayn(joiner, words, arr_length);
    if (!s) exit(1);
    printf("%s\n", s);
    free(s);

    // Don't forget to free the ColorTexts/ColorArgs.
    for (size_t i = 0; i < arr_length; i++) ColorText_free(words[i]);
    \endexamplecode

    \sa colr
    \sa colr_join
*/
char* colr_join_arrayn(void* joinerp, void* ps, size_t count) {
    if (!(joinerp && ps && count)) return NULL;
    size_t length = _colr_join_arrayn_size(joinerp, ps, count);
    if (length < 2) return NULL;
    ColorArg* joiner_cargp = NULL;
    ColorResult* joiner_cresp = NULL;
    ColorText* joiner_ctextp = NULL;
    char* joiner = NULL;
    if (ColorArg_is_ptr(joinerp)) {
        joiner_cargp = joinerp;
        joiner = ColorArg_to_esc(*joiner_cargp);
    } else if (ColorResult_is_ptr(joinerp)) {
        joiner_cresp = joinerp;
        joiner = ColorResult_to_str(*joiner_cresp);
    } else if (ColorText_is_ptr(joinerp)) {
        joiner_ctextp = joinerp;
        joiner = ColorText_to_str(*joiner_ctextp);
    } else {
        // Better be a string!
        joiner = joinerp;
    }
    if (!joiner) return NULL;
    bool do_reset = (joiner_cargp || joiner_ctextp);

    char* final = calloc(length, sizeof(char));
    size_t i = 0;
    ColorArg** cargps = ps;
    ColorResult** cresps = ps;
    ColorText** ctextps = ps;
    if (ColorArg_is_ptr(*cargps)) {
        while ((i < count) && cargps[i]) {
            char* s = ColorArg_to_esc(*(cargps[i++]));
            if (!s || s[0] == '\0') continue;
            if (i > 1) strcat(final, joiner);
            strcat(final, s);
            free(s);
            // Set more than once, but only if there was a non-empty ColorArg.
            do_reset = true;
        }
    } else if (ColorResult_is_ptr(*cresps)) {
        while ((i < count) && cresps[i]) {
            char* s = ColorResult_to_str(*(cresps[i++]));
            if (!s || s[0] == '\0') continue;
            if (i > 1) strcat(final, joiner);
            strcat(final, s);
        }
    } else if (ColorText_is_ptr(*ctextps)) {
        while ((i < count) && ctextps[i]) {
            char* s = ColorText_to_str(*(ctextps[i++]));
            if (!s || s[0] == '\0') continue;
            if (i > 1) strcat(final, joiner);
            strcat(final, s);
            free(s);
        }
    } else {
        char** sps = ps;
        while ((i < count) && sps[i]) {
            if (!sps[i] || sps[i][0] == '\0') {
                i++;
                continue;
            }
            if (i) strcat(final, joiner);
            strcat(final, sps[i++]);
        }
    }
    if (final[0] == '\0') {
        // All pieces were NULL/empty. Act like strdup(joiner).
        strcat(final, joiner);
    }
    if (joiner_cargp || joiner_ctextp) free(joiner);
    if (do_reset) colr_append_reset(final);
    return final;
}

/*! Get the size in bytes needed to join an array of \strings, `ColorArg`s,
    `ColorResult`s, or `ColorText`s by another \string, ColorArg, ColorResult,
    or ColorText.

    \details
    This is used to allocate memory in the _colr_join_array() function.

    \warninternal

    \pi joinerp The joiner (any ColorArg, ColorResult, ColorText, or string).
    \pi ps      An array of pointers to `ColorArg`s, `ColorResult`s, `ColorText`s,
                or strings.
                The array must have `NULL` as the last item if \p count is
                greater than the total number of items.
    \pi count   Total number of items in the array.
    \return     The number of bytes needed to allocate the result of
                colr_join_arrayn(), possibly `0`.

    \sa colr
    \sa colr_join
    \sa colr_join_array
*/
size_t _colr_join_arrayn_size(void* joinerp, void* ps, size_t count) {
    if (!(joinerp && ps && count)) return 0;
    size_t length = 0;
    size_t joiner_len = _colr_ptr_length(joinerp);
    if (joiner_len < 2) return 0;
    length += joiner_len;

    size_t i = 0;
    ColorArg** cargps = ps;
    ColorResult** cresps = ps;
    ColorText** ctextps = ps;
    if (ColorArg_is_ptr(*cargps)) {
        while ((i < count) && cargps[i]) length += ColorArg_length(*(cargps[i++]));
    } else if (ColorResult_is_ptr(*cresps)) {
        while ((i < count) && cresps[i]) length += ColorResult_length(*(cresps[i++]));
    } else if (ColorText_is_ptr(*ctextps)) {
        while ((i < count) && ctextps[i]) length += ColorText_length(*(ctextps[i++]));
    } else {
        char** sps = ps;
        while ((i < count) && sps[i]) length += strlen(sps[i++]);
    }
    length += joiner_len * i;
    // Add room for the reset_code.
    length += CODE_RESET_LEN;
    // One more for the null.
    return length++;
}

/*! Determine the length of a `NULL`-terminated array of \strings, `ColorArg`s,
    `ColorResult`s, or `ColorText`s.

    \warninternal

    \pi ps  A `NULL`-terminated array of `ColorArg`s, `ColorResult`s, `ColorText`s,
            or \strings.
    \return The number of items (before `NULL`) in the array.
*/
size_t _colr_join_array_length(void* ps) {
    if (!ps) return 0;
    size_t i = 0;
    ColorArg** cargps = ps;
    ColorResult** cresps = ps;
    ColorText** ctextps = ps;
    if (ColorArg_is_ptr(*cargps)) {
        while (cargps[i++]);
    } else if (ColorResult_is_ptr(*cresps)) {
        while (cresps[i++]);
    } else if (ColorText_is_ptr(*ctextps)) {
        while (ctextps[i++]);
    } else {
        char** sps = ps;
        while (sps[i++]);
    }
    return i - 1;
}

/*! Get the size, in bytes, needed to convert a ColorArg, ColorResult, ColorText,
    or \string into a string.

    \details
    This is used in the variadic _colr* functions.

    \warninternal

    \pi p   A ColorArg pointer, ColorResult pointer, ColorText pointer, or \string.
    \return The length needed to convert the object into a string
            (`strlen() + 1` for strings).
*/
size_t _colr_ptr_length(void* p) {
    if (!p) return 0;
    size_t length = 0;
    if (ColorArg_is_ptr(p)) {
        // It's a ColorArg.
        ColorArg* cargp = p;
        length = ColorArg_length(*cargp);
        // Gonna need a reset to close this code if a plain string follows it
        // in the _colr_cat() function arguments.
        length += CODE_RESET_LEN;
    } else if (ColorResult_is_ptr(p)) {
        ColorResult* cresp = p;
        length = ColorResult_length(*cresp);
    } else if (ColorText_is_ptr(p)) {
        ColorText* ctextp = p;
        length = ColorText_length(*ctextp);
    } else {
        // It's a string, or it better be anyway.
        length = strlen((char* )p) + 1;
    }
    return length;
}

/*! Determine what kind of pointer is being passed, and call the appropriate
    \<type\>_repr function to obtain an allocated string representation.

    \details
    You should use colr_repr() instead.

    \warninternal

    \pi p   A ColorArg pointer, ColorResult pointer, ColorText pointer, or string.
    \return \parblock
                An allocated string with the result.
                \mustfree
                \maybenullalloc
            \endparblock

    \sa colr_repr
*/
char* _colr_ptr_repr(void* p) {
    if (!p) return NULL;
    if (ColorArg_is_ptr(p)) {
        ColorArg* cargp = p;
        return ColorArg_repr(*cargp);
    } else if (ColorResult_is_ptr(p)) {
        ColorResult* cresp = p;
        return ColorResult_repr(*cresp);
    } else if (ColorText_is_ptr(p)) {
        ColorText* ctextp = p;
        return ColorText_repr(*ctextp);
    }
    // Better be a string.
    return colr_str_repr((char*)p);
}

/*! Determine what kind of pointer is being passed, and call the appropriate
    \<type\>_to_str function to obtain an allocated string.

    \warninternal

    \pi p   A ColorArg pointer, ColorResult pointer, ColorText pointer, or string.
    \return \parblock
                An allocated string with the result.
                \mustfree
                \maybenullalloc
            \endparblock

*/
char* _colr_ptr_to_str(void* p) {
    if (!p) return NULL;
    if (ColorArg_is_ptr(p)) {
        ColorArg* cargp = p;
        return ColorArg_to_esc(*cargp);
    } else if (ColorResult_is_ptr(p)) {
        ColorResult* cresp = p;
        return ColorResult_to_str(*cresp);
    } else if (ColorText_is_ptr(p)) {
        ColorText* ctextp = p;
        return ColorText_to_str(*ctextp);
    }
    // Better be a string.
    return strdup((char*)p);
}

/*! Compares two ArgTypes.

    \details
    This is used to implement colr_eq().

    \pi a   The first ArgType to compare.
    \pi b   The second ArgType to compare.
    \return `true` if they are equal, otherwise `false`.
*/
bool ArgType_eq(ArgType a, ArgType b) {
    return (a == b);
}

/*! Creates a \string representation of a ArgType.

    \pi type An ArgType to get the type from.
    \return  \parblock
                A pointer to an allocated string.
                \mustfree
                \maybenullalloc
             \endparblock

    \sa ArgType
*/
char* ArgType_repr(ArgType type) {
    char* typestr;
    switch (type) {
        case ARGTYPE_NONE:
            asprintf_or_return(NULL, &typestr, "ARGTYPE_NONE");
            break;
        case FORE:
            asprintf_or_return(NULL, &typestr, "FORE");
            break;
        case BACK:
            asprintf_or_return(NULL, &typestr, "BACK");
            break;
        case STYLE:
            asprintf_or_return(NULL, &typestr, "STYLE");
            break;
    }
    return typestr;
}

/*! Creates a human-friendly \string from an ArgType.

    \pi type An ArgType to get the type from.
    \return  \parblock
                A pointer to an allocated string.
                \mustfree
                \maybenullalloc
             \endparblock

    \sa ArgType
*/
char* ArgType_to_str(ArgType type) {
    char* typestr;
    switch (type) {
        case ARGTYPE_NONE:
            asprintf_or_return(NULL, &typestr, "none");
            break;
        case FORE:
            asprintf_or_return(NULL, &typestr, "fore");
            break;
        case BACK:
            asprintf_or_return(NULL, &typestr, "back");
            break;
        case STYLE:
            asprintf_or_return(NULL, &typestr, "style");
            break;
    }
    return typestr;
}

/*! Create a ColorArg with ARGTYPE_NONE and ColorValue.type.TYPE_NONE.
    \details
    This is used to pass "empty" fore/back/style args to the \colrmacros,
    where `NULL` may have a different meaning for users of the ColorArg.

    \return `(ColorArg){.type=ARGTYPE_NONE, .value.type=TYPE_NONE}`

    \sa ColorArg_is_empty
    \sa ColorValue_empty
*/
ColorArg ColorArg_empty(void) {
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=ARGTYPE_NONE,
        .value=ColorValue_empty()
    };
}

/*! Compares two ColorArg structs.
    \details
    They are considered "equal" if their `.type` and `.value` match.

    \pi a   First ColorArg to compare.
    \pi b   Second ColorArg to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_eq(ColorArg a, ColorArg b) {
    return (a.type == b.type) && ColorValue_eq(a.value, b.value);
}

/*! Create a \string representation of a ColorArg with a stylized type/name
    using escape codes built from the ColorArg's values.

    \pi carg      A ColorArg to get an example string for.
    \pi colorized Whether to include a colorized example.
                  If set to `false`, there will be no escape-codes in the string.
    \return       \parblock
                        An allocated string with the result.
                        \mustfree
                        \maybenullalloc
                  \endparblock

    \sa ColorArg

    \examplecodefor{ColorArg_example,.c}
    ColorArg args[] = {
        fore_arg(rgb(75, 25, 155)),
        fore_arg(ext_hex("#ff00bb")),
        back_arg(RED),
        style_arg(UNDERLINE),
        fore_arg("NOT_VALID"),
        back_arg("NOT_VALID"),
        style_arg("NOT_VALID")
    };
    size_t arg_len = sizeof(args) / sizeof(args[0]);
    for (size_t i = 0; i < arg_len; i++) {
        char* example = ColorArg_example(args[i], true);
        if (!example) continue;
        printf("%s\n", example);
        free(example);
    }
    \endexamplecode
*/
char* ColorArg_example(ColorArg carg, bool colorized) {
    char* argtype_name = ArgType_to_str(carg.type);
    if (!argtype_name) return NULL;
    char* val_example = ColorValue_example(carg.value);
    if (!val_example) {
        // Don't have a test for allocation failures, yet.
        // LCOV_EXCL_START
        free(argtype_name);
        return NULL;
        // LCOV_EXCL_STOP
    }
    // Always use fore-codes for example colors.
    if (carg.type == BACK) carg.type = FORE;
    char* codes = ColorArg_to_esc(carg);
    if (!codes) {
        // Don't have a test for allocation failures, yet.
        // LCOV_EXCL_START
        free(argtype_name);
        free(val_example);
        return NULL;
        // LCOV_EXCL_STOP
    }

    char* code_repr = NULL;
    if (ColorArg_is_valid(carg)) {
        code_repr = colr_str_repr(codes);
        if (!code_repr) {
            // Don't have a test for allocation failures, yet.
            // LCOV_EXCL_START
            free(argtype_name);
            free(val_example);
            free(codes);
            return NULL;
            // LCOV_EXCL_STOP
        }
    }
    char* example = NULL;
    if (colorized) {
        asprintf_or_return(
            NULL,
            &example,
            "%7s:%s●" CODE_RESET_ALL " %s%s", // ⬔ ◨
            argtype_name,
            codes,
            val_example,
            (code_repr ? code_repr : "-")
        );
    } else {
        asprintf_or_return(
            NULL,
            &example,
            "%7s:  %s%s",
            argtype_name,
            val_example,
            (code_repr ? code_repr : "-")
        );
    }
    free(argtype_name);
    free(val_example);
    if (codes) free(codes);
    if (code_repr) free(code_repr);
    return example;
}
/*! Free allocated memory for a ColorArg.

    \details
    This has no advantage over `free(colorarg)` right now, it is used in
    debugging, and may be extended in the future. It's better just to use it
    (or the colr_free() macro).

    \pi p ColorArg to free.

    \sa ColorArg
*/
void ColorArg_free(ColorArg* p) {
    if (!p) return;
    free(p);
}

/*! Explicit version of ColorArg_from_value that only handles BasicValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value BasicValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_BasicValue(ArgType type, BasicValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    BasicValue val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_BASIC, &val),
    };
}

/*! Explicit version of ColorArg_from_value that only handles ExtendedValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value ExtendedValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_ExtendedValue(ArgType type, ExtendedValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    ExtendedValue val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_EXTENDED, &val),
    };
}

/*! Explicit version of ColorArg_from_value that only handles RGB structs.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value RGB struct to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_RGB(ArgType type, RGB value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    RGB val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_RGB, &val),
    };
}

/*! Parse an escape-code \string into a ColorArg.

    \details
    For malformed escape-codes the `.type` member will be `ARGTYPE_NONE`,
    and the `.value.type` member will be set to `TYPE_INVALID`.
    This means that `ColorArg_is_invalid(carg) == true`.

    \pi s   The escape code to parse. It must not have extra characters.
    \return An initialized ColorArg, possibly invalid.

    \sa ColorArg
    \sa colr_str_get_codes
    \sa ColorValue_from_esc
    \sa BasicValue_from_esc
    \sa ExtendedValue_from_esc
    \sa StyleValue_from_esc
    \sa RGB_from_esc
*/
ColorArg ColorArg_from_esc(const char* s) {
    ColorValue cval = ColorValue_from_esc(s);
    if (ColorValue_is_invalid(cval)) {
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=ARGTYPE_NONE,
            .value=cval
        };
    }
    // The value is good, I hate that the fore/back info was lost.
    if (cval.type == TYPE_STYLE) {
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=STYLE,
            .value=cval
        };
    }
    // So, I'll parse out the fore/back information.
    // Fore colors start with: "\x1b[3" or "\x1b[9".
    // Back colors with: "\x1b[4" or "\x1b[1".
    if ((s[2] == '3') || (s[2] == '9')) {
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=FORE,
            .value=cval
        };
    }
    assert((s[2] == '4') || (s[2] == '1'));
    // Back color.
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=BACK,
        .value=cval
    };
}

/*! Build a ColorArg (fore, back, or style value) from a known color name/style.

    \details
    The `.value.type` attribute can be checked for an invalid type, or you
    can call ColorArg_is_invalid(x).

    \pi type      ArgType (FORE, BACK, STYLE).
    \pi colorname A known color name/style.

    \return A ColorArg struct with usable values.

    \sa ColorArg
*/
ColorArg ColorArg_from_str(ArgType type, const char* colorname) {
    ColorValue cval = ColorValue_from_str(colorname);
    if ((type == STYLE) && (cval.type != TYPE_STYLE)) {
        // Bad style string.
        cval.type = TYPE_INVALID_STYLE;
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=STYLE,
            .value=cval,
        };
    } else if (((type == BACK) || (type == FORE)) && (cval.type == TYPE_STYLE)) {
        // Trying to use a valid style name for a color.
        cval.type = TYPE_INVALID;
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=type,
            .value=cval,
        };
    }
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=cval
    };

}

/*! Explicit version of ColorArg_from_value that only handles StyleValues.

    \details
    This is used in some macros to aid in dynamic escape code creation.

    \pi type  ArgType (FORE, BACK, STYLE).
    \pi value StyleValue to use.

    \return A ColorArg, with the `.value.type` member possibly set to `TYPE_INVALID`.

    \sa ColorArg
*/
ColorArg ColorArg_from_StyleValue(ArgType type, StyleValue value) {
    // Saving a copy on the stack, in case an anonymous value was given.
    // As long as the address is good through _from_value() we're good.
    StyleValue val = value;
    return (ColorArg){
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(TYPE_STYLE, &val),
    };
}

/*! Used with the color_arg macro to dynamically create a ColorArg based
    on it's argument type.

    \pi type     ArgType value, to mark the type of ColorArg.
    \pi colrtype ColorType value, to mark the type of ColorValue.
    \pi p        A pointer to either a BasicValue, ExtendedValue, or a RGB.

    \return A ColorArg struct with the appropriate `.value.type` member set for
            the value that was passed. For invalid types the `.value.type` member may
            be set to one of:
        - TYPE_INVALID
        - TYPE_INVALID_EXT_RANGE
        - TYPE_INVALID_RGB_RANGE

    \sa ColorArg
*/
ColorArg ColorArg_from_value(ArgType type, ColorType colrtype, void* p) {
    if (!p) {
        return (ColorArg){
            .marker=COLORARG_MARKER,
            .type=ARGTYPE_NONE,
            .value=ColorValue_from_value(TYPE_INVALID, NULL)
        };
    }
    ColorArg carg = {
        .marker=COLORARG_MARKER,
        .type=type,
        .value=ColorValue_from_value(colrtype, p),
    };
    return carg;
}


/*! Checks to see if a ColorArg is an empty placeholder.

    \details
    A ColorArg is empty if it's `.type` is set to `ARGTYPE_NONE`.

    \pi carg A ColorArg to check.
    \return  `true` if the ColorArg is considered "empty", otherwise `false`.
*/
bool ColorArg_is_empty(ColorArg carg) {
    return (carg.type == ARGTYPE_NONE) || ColorValue_is_empty(carg.value);
}

/*! Checks to see if a ColorArg holds an invalid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is invalid, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_invalid(ColorArg carg) {
    return !(bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type));
}

/*! Checks a void pointer to see if it contains a ColorArg struct.

    \details The first member of a ColorArg is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorArg, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_ptr(void* p) {
    if (!p) return false;
    if (!colr_check_marker(COLORARG_MARKER, p)) return false;
    // The head of a ColorArg is always a valid marker.
    // This is probably not needed, now that colr_check_marker is implemented.
    ColorArg *cargp = p;
    return cargp->marker == COLORARG_MARKER;
}

/*! Checks to see if a ColorArg holds a valid value.

    \pi carg ColorArg struct to check.
    \return `true` if the value is valid, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_is_valid(ColorArg carg) {
    return bool_colr_enum(carg.value.type) && bool_colr_enum(carg.type);
}


/*! Returns the length in bytes needed to allocate a \string built with
    ColorArg_to_esc().

    \pi carg ColorArg to use.

    \return  The length (`size_t`) needed to allocate a ColorArg's string,
             or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorArg
*/
size_t ColorArg_length(ColorArg carg) {
    // Empty color args turn into empty strings, so just "\0".
    if (ColorArg_is_empty(carg)) return 1;
    return ColorValue_length(carg.type, carg.value);
}

/*! Creates a \string representation for a ColorArg.

    \details
    Allocates memory for the string representation.

    \pi carg ColorArg struct to get the representation for.
    \return \parblock
                Allocated string for the representation.
                \mustfree
            \endparblock

    \sa ColorArg
*/
char* ColorArg_repr(ColorArg carg) {
    char* type = ArgType_repr(carg.type);
    char* value = ColorValue_repr(carg.value);
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "ColorArg {.type=%s, .value=%s}",
        type,
        value
    );
    free(type);
    free(value);
    return repr;
}


/*! Converts a ColorArg into an escape code \string.

    \details
    Allocates memory for the string.

    \details
    If the ColorArg is empty (`ARGTYPE_NONE`), an empty string is returned.

    \details
    If the ColorValue is invalid, an empty string is returned.
    You must still free the empty string.

    \pi carg ColorArg to get the ArgType and ColorValue from.
    \return  \parblock
                 Allocated string for the escape code.
                 \mustfree
                 If the ColorArg is considered "empty", or the ColorValue is
                 invalid, then `NULL` is returned.
             \endparblock

    \sa ColorArg
*/
char* ColorArg_to_esc(ColorArg carg) {
    if (ColorArg_is_empty(carg)) return NULL;
    return ColorValue_to_esc(carg.type, carg.value);
}

/*! Converts a ColorArg into an escape code \string and fills the destination
    string.

    \details
    If the ColorArg is empty (`ARGTYPE_NONE`), `dest[0]` is set to `'\0'`.

    \details
    If the ColorValue is invalid, `dest[0]` is set to `'\0'`.

    \pi dest \parblock
                 Destination for the escape code string.
                 <em>Must have room for the code type being used</em>.
                 See ColorArg_length() for determining the size needed.
             \endparblock
    \pi carg ColorArg to get the ArgType and ColorValue from.
    \return  `true` if the ColorArg was valid, otherwise `false`.

    \sa ColorArg
*/
bool ColorArg_to_esc_s(char* dest, ColorArg carg) {
    if (ColorArg_is_empty(carg)) {
        dest[0] = '\0';
        return false;
    }
    return ColorValue_to_esc_s(dest, carg.type, carg.value);
}
/*! Copies a ColorArg into memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi carg ColorArg to copy/allocate for.
    \return  \parblock
                 Pointer to a heap-allocated ColorArg.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorArg
*/
ColorArg* ColorArg_to_ptr(ColorArg carg) {
    ColorArg* p = malloc(sizeof(carg));
    if (!p) return NULL;
    carg.marker = COLORARG_MARKER;
    *p = carg;
    return p;
}


/*! Free an allocated array of ColorArgs, including the array itself.

    \details
    Each individual ColorArg will be released, and finally the allocated memory
    for the array of pointers will be released.

    \pi ps A pointer to an array of ColorArgs, where `NULL` is the last item.

    \examplecodefor{ColorArgs_array_free,.c}
    char* s = colr("Test", fore(RED), back(WHITE), style(BRIGHT));
    if (!s) exit(1);
    // Call something that creates an array of ColorArgs on the heap.
    ColorArg** carg_array = ColorArgs_from_str(s, false);
    free(s);
    if (!carg_array) exit(1);
    // ... do something with the array of ColorArgs.

    // And then free it:
    ColorArgs_array_free(carg_array);
    \endexamplecode
*/
void ColorArgs_array_free(ColorArg** ps) {
    if (!ps) return;
    // Free the individual items, until NULL is hit.
    for (size_t i = 0; ps[i]; i++) {
        free(ps[i]);
    }
    // Free the pointer array.
    free(ps);
}

/*! Creates a string representation for an array of ColorArg pointers.

    \pi lst The ColorArg array to create the representation for (`ColorArg**`).
    \return An allocated string, or `NULL` if \p lst is `NULL`, or the allocation
            fails.

    \examplecodefor{ColorArgs_array_repr,.c}
    char* s = colr("Test", fore(RED), back(WHITE), style(BRIGHT));
    if (!s) exit(1);
    // Call something that creates an array of ColorArgs on the heap.
    ColorArg** carg_array = ColorArgs_from_str(s, false);
    free(s);
    if (!carg_array) exit(1);
    char* lst_repr = ColorArgs_array_repr(carg_array);
    ColorArgs_array_free(carg_array);
    if (!lst_repr) exit(1);
    printf("ColorArg carg_array[] = %s\n", lst_repr);
    // And then free it:
    free(lst_repr);
    \endexamplecode
*/
char* ColorArgs_array_repr(ColorArg** lst) {
    char* repr = NULL;
    if (!lst) {
        asprintf_or_return(NULL, &repr, "NULL");
        return repr;
    }
    // Get count.
    size_t count = 0;
    for (size_t i = 0; lst[i]; i++) {
        count++;
    }
    // Get length, and ColorArg reprs.
    char* strings[count];
    size_t length = 0;
    for (size_t i = 0; lst[i]; i++) {
        char* srepr = colr_repr(*lst[i]);
        length += srepr ? strlen(srepr) : 4;
        strings[i] = srepr;
    }
    // Account for last NULL item.
    length += 4;
    char* sep = ",\n";
    size_t sep_len = strlen(sep);
    char* indent = "    ";
    size_t indent_len = strlen(indent);
    // Account for commas/spaces/newlines, and the NULL item.
    length += (count + 1) * (indent_len + sep_len);
    // Account for wrappers.
    length += 2;
    repr = calloc(length + 1, sizeof(char));
    char* repr_start = repr;
    repr[0] = '{';
    repr[1] = '\n';
    repr += 2;
    size_t append_len = 0;
    size_t written = 0;
    for (size_t i = 0; i < count; i++) {
        append_len = indent_len + strlen(strings[i]);
        written = snprintf(repr, append_len + 1, "%s%s", indent, strings[i]);
        free(strings[i]);
        if (written > append_len) {
            // String was truncated. Something is wrong with `length` or `calloc`.
            // LCOV_EXCL_START
            for (size_t j = i + 1; j < count; j++) free(strings[j]);
            free(repr);
            return NULL;
            // LCOV_EXCL_STOP
        }
        repr += append_len;

        written = snprintf(repr, sep_len + 1, "%s", sep);
        if (written > append_len) {
            // String was truncated.
            // LCOV_EXCL_START
            for (size_t j = i + 1; j < count; j++) free(strings[j]);
            free(repr);
            return NULL;
            // LCOV_EXCL_STOP
        }
        // Skip past the separator string.
        repr += sep_len;
    }

    char* tail = "NULL\n}";
    append_len = indent_len + strlen(tail);
    written = snprintf(repr, append_len + 1, "%s%s", indent, tail);
    if (written > append_len) {
        // String was truncated.
        // LCOV_EXCL_START
        free(repr);
        return NULL;
        // LCOV_EXCL_STOP
    }
    return repr_start;
}

/*! Create an array of ColorArgs from escape-codes found in a \string.

    \details
    This uses ColorArg_from_esc() and colr_str_get_codes() to build a heap-allocated
    array of heap-allocated ColorArgs.

    \pi s      \parblock
                    A string to get the escape-codes from.
                    \mustnull
               \endparblock
    \pi unique Whether to only include _unique_ ColorArgs.
    \return    \parblock
                    An allocated array of ColorArg pointers, where the last element is `NULL`.
                    \mustfree
               \endparblock
    \retval    If \p s is `NULL`, or empty, or there are otherwise no escape-codes
               found in the string, then `NULL` is returned.
    \retval    On success, there will be at least two pointers behind the return
               value. The last pointer is always `NULL`.

    \examplecodefor{ColorArgs_from_str,.c}
    #include <stdio.h>
    #include "colr.h"

    int main(void) {
        char* s = colr_cat(
            Colr("Testing this out.", fore(RED), back(WHITE)),
            Colr("Again.", fore(RED), style(UNDERLINE))
        );
        if (!s) return EXIT_FAILURE;
        ColorArg** carg_array = ColorArgs_from_str(s, false);
        free(s);
        if (!carg_array) {
            fprintf(stderr, "No code found? Impossible!\n");
            return EXIT_FAILURE;
        }
        // Iterate over the ColorArg array.
        for (size_t i = 0; carg_array[i]; i++) {
            char* carg_example = ColorArg_example(*(carg_array[i]), true);
            if (!carg_example) continue;
            printf("%s\n", carg_example);
            free(carg_example);
        }
        // Free the ColorArgs, and the array of pointers.
        ColorArgs_array_free(carg_array);
    }
    \endexamplecode
*/
ColorArg** ColorArgs_from_str(const char* s, bool unique) {
    // It's okay to pass a NULL pointer to colr_str_get_codes().
    char** codes = colr_str_get_codes(s, unique);
    // There may not be any escape-codes in the string, or allocation failed.
    if (!codes) return NULL;
    // Count ColorArgs needed.
    size_t count = 0;
    while (codes[count++]);
    // There will be a NULL element added to this array, just like the code array.
    ColorArg** cargs = malloc(sizeof(ColorArg*) * count);
    count--;
    for (size_t i = 0; i < count; i++) {
        cargs[i] = ColorArg_to_ptr(ColorArg_from_esc(codes[i]));
    }
    colr_str_array_free(codes);
    cargs[count] = NULL;
    return cargs;
}

/*! Creates an "empty" ColorJustify, with JUST_NONE set.

    \return An initialized ColorJustify, with no justification method set.

    \sa ColorJustify
*/
ColorJustify ColorJustify_empty(void) {
    return (ColorJustify){
        .marker=COLORJUSTIFY_MARKER,
        .method=JUST_NONE,
        .width=0,
        .padchar=0,
    };
}

/*! Compares two ColorJustify structs.
    \details
    They are considered "equal" if their member values match.

    \pi a   First ColorJustify to compare.
    \pi b   Second ColorJustify to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorJustify
*/
bool ColorJustify_eq(ColorJustify a, ColorJustify b) {
    return (
        (a.method == b.method) &&
        (a.width == b.width) &&
        (a.padchar == b.padchar)
    );
}

/*! Checks to see if a ColorJustify is "empty".

    \details
    A ColorJustify is considered "empty" if the `.method` member is set to
    `JUST_NONE`.

    \pi cjust The ColorJustify to check.
    \return   `true` if the ColorJustify is empty, otherwise `false`.

    \sa ColorJustify
    \sa ColorJustify_empty
*/
bool ColorJustify_is_empty(ColorJustify cjust) {
    return cjust.method == JUST_NONE;
}

/*! Creates a ColorJustify.

    \details
    This is used to ensure every ColorJustify has it's `.marker` member set
    correctly.

    \pi method  ColorJustifyMethod to use.
    \pi width   Width for justification.
                If `0` is given, ColorText will use the width from colr_term_size().
    \pi padchar Padding character to use.
                If `0` is given, the default, space (`' '`), is used.

    \return     An initialized ColorJustify.
*/
ColorJustify ColorJustify_new(ColorJustifyMethod method, int width, char padchar) {
    return (ColorJustify){
        .marker=COLORJUSTIFY_MARKER,
        .method=method,
        .width=width,
        .padchar=padchar
    };
}

/*! Creates a \string representation for a ColorJustify.

    \details
    Allocates memory for the string representation.

    \pi cjust ColorJustify struct to get the representation for.
    \return   \parblock
                    Allocated string for the representation.
                    \mustfree
              \endparblock

    \sa ColorJustify
*/
char* ColorJustify_repr(ColorJustify cjust) {
    char* meth_repr = ColorJustifyMethod_repr(cjust.method);
    char* pad_repr = colr_char_repr(cjust.padchar);
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "ColorJustify {.method=%s, .width=%d, .padchar=%s}",
        meth_repr,
        cjust.width,
        pad_repr
    );
    free(meth_repr);
    free(pad_repr);
    return repr;
}

/*! Creates a \string representation for a ColorJustifyMethod.

    \details
    Allocates memory for the string representation.

    \pi meth ColorJustifyMethod to get the representation for.
    \return  \parblock
                Allocated string for the representation.
                \mustfree
             \endparblock

    \sa ColorJustifyMethod
*/
char* ColorJustifyMethod_repr(ColorJustifyMethod meth) {
    char* repr;
    switch (meth) {
        case JUST_NONE:
            asprintf_or_return(NULL, &repr, "JUST_NONE");
            break;
        case JUST_LEFT:
            asprintf_or_return(NULL, &repr, "JUST_LEFT");
            break;
        case JUST_RIGHT:
            asprintf_or_return(NULL, &repr, "JUST_RIGHT");
            break;
        case JUST_CENTER:
            asprintf_or_return(NULL, &repr, "JUST_CENTER");
            break;
    }
    return repr;
}

/*! Colorize a ColorResult, and return a new allocated ColorResult. This is like
    ColorText_from_value(), except it accepts an allocated ColorResult as the
    first argument.

    \pi cres  \parblock
                An allocated ColorResult to colorize.
                This will be released to create the new ColorResult.
              \endparblock
    \pi ...   \parblock
                One or more fore(), back(), or style() arguments (ColorArgs).
                The last argument must be _ColrLastArg.
                The allocated ColorArgs will be `free()`'d.
              \endparblock
    \return   \parblock
                An allocated ColorResult.
                \maybenull
                \colrmightfree
              \endparblock

    \sa ColorResult
*/
ColorResult* ColorResult_Colr(ColorResult* cres, ...) {
    // Last argument must be _ColrLastArg.
    char* s = ColorResult_to_str(*cres);
    if (!s) return cres;
    va_list args;
    va_start(args, cres);
    va_list argcopy;
    va_copy(argcopy, args);
    ColorText ctext = ColorText_from_valuesv(s, argcopy);
    va_end(argcopy);
    va_end(args);
    char* final = ColorText_to_str(ctext);
    ColorResult_free(cres);
    ColorText_free_args(&ctext);
    return ColorResult_to_ptr(ColorResult_new(final));
}

/*! Creates a ColorResult with `.result=NULL` and `.length=-1`, with the
    appropriate struct marker.

    \return An "empty" (initialized) ColorResult.

    \sa ColorResult
*/
ColorResult ColorResult_empty(void) {
    return (ColorResult){
        .marker=COLORRESULT_MARKER,
        .result=NULL,
        .length=-1,
    };
}

/*! Compares two ColorResults.

    \details
    They are equal if all of their members are equal, excluding the memory
    address for the `.result` member.

    \pi a First ColorResult to compare.
    \pi b Second ColorResult to compare.
    \return `true` if they are equal, otherwise `false`.

    \sa ColorResult
*/
bool ColorResult_eq(ColorResult a, ColorResult b) {
    // Purposely doing pointer comparison here.
    if ((a.marker != b.marker) || (a.length != b.length)) return false;
    // They are both NULL?
    if (!(a.result && b.result)) return true;
    return colr_str_eq(a.result, b.result);
}

/*! Free allocated memory for a ColorResult and it's `.result` member.

    \pi p A ColorResult with a `NULL` or heap-allocated `.result` member.

    \sa ColorResult
*/
void ColorResult_free(ColorResult* p) {
    if (!p) return;
    if (p->result) free(p->result);
    p->result = NULL;
    free(p);
}

/*! Allocates a copy of a string, and creates a ColorResult from it.

    \pi s   The string to copy.
    \return \parblock
                An initialized ColorResult.
                The ColorResult may be "empty" if \p s is `NULL`.
            \endparblock

    \sa ColorResult
*/
ColorResult ColorResult_from_str(const char* s) {
    char* copied = s ? strdup(s) : NULL;
    return ColorResult_new(copied);
}

/*! Checks a void pointer to see if it contains a ColorResult struct.

    \details The first member of a ColorResult is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorResult, otherwise `false`.

    \sa ColorResult
*/
bool ColorResult_is_ptr(void* p) {
    if (!p) return false;
    if (!colr_check_marker(COLORRESULT_MARKER, p)) return false;
    // The head of a ColorResult is always a valid marker.
    // This is probably not needed, now that colr_check_marker is implemented.
    ColorArg *cargp = p;
    return cargp->marker == COLORRESULT_MARKER;
}

/*! Return the length in bytes (including the null-terminator), that is needed
    to store the return from ColorResult_to_str() (`.result`).

    \pi cres A ColorResult to calculate the length for.
    \return  The length of a ColorResult, possibly `0` if `.result` is `NULL`.

    \sa ColorResult
*/
size_t ColorResult_length(ColorResult cres) {
    if (!cres.result) return 0;
    if (cres.length > 0) return cres.length;
    cres.length = strlen(cres.result) + 1;
    return cres.length;
}

/*! Initialize a new ColorResult with an allocated \string.

    \pi s   An allocated string to use for the `.result` member.
    \return \parblock
                An initialized ColorResult.
                The ColorResult will be considered "empty" if \p s is `NULL`
            \endparblock

    \sa ColorResult
*/
ColorResult ColorResult_new(char* s) {
    ColorResult cres = ColorResult_empty();
    cres.result = s;
    cres.length = s ? (strlen(s) + 1) : 0;
    return cres;
}

/*! Create a string representation for a ColorResult.

    \details
    This happens to be the same as `colr_str_repr(cres.result)` right now.

    \pi cres A ColorResult to create the representation string for.
    \return  \parblock
                 An allocated string with the result.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorResult
*/
char* ColorResult_repr(ColorResult cres) {
    return colr_str_repr(cres.result);
}

/*! Allocate memory for a ColorResult, fill it, and return it.

    \details
    This ensure the appropriate struct marker is set, for use with Colr.

    \pi cres A ColorResult to use.
    \return  \parblock
                 An allocated ColorResult.
                 \mustfree
                 \colrmightfree
                 \maybenullalloc
             \endparblock

    \sa ColorResult
*/
ColorResult* ColorResult_to_ptr(ColorResult cres) {
    ColorResult *p = malloc(sizeof(cres));
    if (!p) return NULL;
    cres.marker = COLORRESULT_MARKER;
    *p = cres;
    return p;
}

/*! Convert a ColorResult into a \string.

    \details
    This simply returns the `.result` member right now. It is used for
    compatibility with the colr_to_str() macro.

    \pi cres A ColorResult to use.
    \return  \parblock
                 A stringified-version if this ColorResult, which happens to be
                 the `.result` member.
                 <em>If you free the result of this function, the original string
                 used to create the ColorResult will be lost</em>.
             \endparblock

    \sa ColorResult
*/
char* ColorResult_to_str(ColorResult cres) {
    return cres.result;
}

/*! Creates an "empty" ColorText with pointers set to `NULL`.

    \return An initialized ColorText.

    \sa ColorText
*/
ColorText ColorText_empty(void) {
    return (ColorText){
        .marker=COLORTEXT_MARKER,
        .text=NULL,
        .fore=NULL,
        .back=NULL,
        .style=NULL,
        .just=ColorJustify_empty(),
    };
}

/*! Frees a ColorText and it's ColorArgs.

    \details
    The text member is left alone, because it wasn't created by ColrC.

    \pi p Pointer to ColorText to free, along with it's Colr-based members.

    \sa ColorText
*/
void ColorText_free(ColorText* p) {
    if (!p) return;
    ColorText_free_args(p);
    free(p);
}

/*! Frees the ColorArg members of a ColorText. The ColorText itself is not
    free'd.

    \details
    This is safe to use on a stack-allocated ColorText with heap-allocated
    ColorArgs.

    \pi p Pointer to a ColorText.

    \sa ColorText
*/
void ColorText_free_args(ColorText* p) {
    if (!p) return;
    ColorArg_free(p->fore);
    p->fore = NULL;
    ColorArg_free(p->back);
    p->back = NULL;
    ColorArg_free(p->style);
    p->style = NULL;
}


/*! Builds a ColorText from 1 mandatory \string, and optional fore, back, and
    style args (pointers to ColorArgs).
    \pi text Text to colorize (a regular string).
    \pi ... \parblock
                ColorArgs for fore, back, and style, in any order.
                The last argument must be _ColrLastArg. The Colr() macro
                takes care of this for you.
            \endparblock
    \return An initialized ColorText struct.

    \sa ColorText
*/
ColorText ColorText_from_values(char* text, ...) {
    // Argument list must have ColorArg with _ColrLastArg at the end.
    va_list args;
    va_start(args, text);
    va_list argcopy;
    va_copy(argcopy, args);
    ColorText ctext = ColorText_from_valuesv(text, argcopy);
    va_end(argcopy);
    va_end(args);
    return ctext;
}

/*! Builds a ColorText from 1 mandatory \string, and a `va_list` with optional
    fore, back, and style args (pointers to ColorArgs).

    \pi text Text to colorize (a regular string).
    \pi args \parblock
                `va_list` with ColorArgs for fore, back, and style, in any order.
                The last argument must be _ColrLastArg.
                The Colr() macro takes care of this for you, and should be used
                for basic text colorization.
             \endparblock
    \return An initialized ColorText struct.

    \sa ColorText
*/
ColorText ColorText_from_valuesv(char* text, va_list args) {
    // Argument list must have ColorArg with _ColorLastArg at the end.
    ColorText ctext = ColorText_empty();
    ctext.text = text;

    ColorArg *arg = NULL;
    while_colr_va_arg(args, ColorArg*, arg) {
        if (!arg) continue;
        assert(ColorArg_is_ptr(arg));
        // It's a ColorArg.
        if (arg->type == FORE) {
            ctext.fore = arg;
        } else if (arg->type == BACK) {
            ctext.back = arg;
        } else if (arg->type == STYLE) {
            ctext.style = arg;
        } else if (ColorArg_is_empty(*arg)) {
            // Empty ColorArgs are assigned in the order they were passed in.
            if (!ctext.fore) {
                ctext.fore = arg;
            } else if (!ctext.back) {
                ctext.back = arg;
            } else if (!ctext.style) {
                ctext.style = arg;
            }
        }
    }
    return ctext;
}

/*! Checks to see if a ColorText has a certain ColorArg value set.

    \details
    Uses ColorArg_eq() to inspect the `fore`, `back`, and `style` members.

    \pi ctext The ColorText to inspect.
    \pi carg  The ColorArg to look for.
    \return   `true` if the `fore`, `back`, or `style` arg matches `carg`,
              otherwise `false`.

    \sa ColorText
*/
bool ColorText_has_arg(ColorText ctext, ColorArg carg) {
    return (
        (ctext.fore && ColorArg_eq(*(ctext.fore), carg)) ||
        (ctext.back && ColorArg_eq(*(ctext.back), carg)) ||
        (ctext.style && ColorArg_eq(*(ctext.style), carg))
    );
}

/*! Checks to see if a ColorText has any argument values set.

    \pi ctext A ColorText to check.
    \return   `true` if `.fore`, `.back`, or .`style` is set to a non-empty ColorArg,
              otherwise `false`.

    \sa ColorText
*/
bool ColorText_has_args(ColorText ctext) {
    return (
        (ctext.fore && !ColorArg_is_empty(*(ctext.fore))) ||
        (ctext.back && !ColorArg_is_empty(*(ctext.back))) ||
        (ctext.style && !ColorArg_is_empty(*(ctext.style)))
    );
}

/*! Checks to see if a ColorText has no usable values.

    \details
    A ColorText is considered "empty" if the `.text`, `.fore`, `.back`, and
    `.style` pointers are `NULL`, and the `.just` member is set to an "empty"
    ColorJustify.

    \pi ctext The ColorText to check.
    \return   `true` if the ColorText is empty, otherwise `false`.

    \sa ColorText
    \sa ColorText_empty
*/
bool ColorText_is_empty(ColorText ctext) {
    return (
        !(ctext.text || ctext.fore || ctext.back || ctext.style) &&
        ColorJustify_is_empty(ctext.just)
    );
}

/*! Checks a void pointer to see if it contains a ColorText struct.

    \details The first member of a ColorText is a marker.

    \pi     p A void pointer to check.
    \return `true` if the pointer is a ColorText, otherwise `false`.

    \sa ColorText
*/
bool ColorText_is_ptr(void* p) {
    if (!p) return false;
    if (!colr_check_marker(COLORTEXT_MARKER, p)) return false;

    // The head of a ColorText is always a valid marker.
    // This is probably not needed, now that colr_check_marker is implemented.
    ColorText *ctextp = p;
    return ctextp->marker == COLORTEXT_MARKER;
}

/*! Returns the length in bytes needed to allocate a \string built with
    ColorText_to_str() with the current `text`, `fore`, `back`, and `style` members.

    \pi ctext ColorText to use.

    \return   The length (`size_t`) needed to allocate a ColorText's string,
              or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorText
*/
size_t ColorText_length(ColorText ctext) {
    // Empty text yields an empty string, so just "\0".
    if (!ctext.text) return 1;
    size_t length = strlen(ctext.text);
    if (ctext.fore) length+=ColorArg_length(*(ctext.fore));
    if (ctext.back) length+=ColorArg_length(*(ctext.back));
    if (ctext.style) length+=ColorArg_length(*(ctext.style));
    if (ctext.style || ctext.fore || ctext.back) length += CODE_RESET_LEN;
    if (!ColorJustify_is_empty(ctext.just)) {
        // Justification will be used, calculate that in.
        size_t noncode_len = colr_str_noncode_len(ctext.text);
        if (ctext.just.width == 0) {
            // Go ahead and set the actual width, to reduce calls to colr_term_size().
            TermSize ts = colr_term_size();
            ctext.just.width = ts.columns;
        }
        int diff = ctext.just.width - noncode_len;
        // 0 or negative difference means no extra chars are added anyway.
        length += colr_max(0, diff);
    }
    // And the null-terminator.
    length++;
    return length;
}

/*! Allocate a \string representation for a ColorText.

    \pi ctext ColorText to get the string representation for.
    \return Allocated string for the ColorText.

    \sa ColorText
*/
char* ColorText_repr(ColorText ctext) {
    char* repr;
    char* stext = ctext.text ? colr_str_repr(ctext.text) : NULL;
    char* sfore = ctext.fore ? ColorArg_repr(*(ctext.fore)) : NULL;
    char* sback = ctext.back ? ColorArg_repr(*(ctext.back)) : NULL;
    char* sstyle = ctext.style ? ColorArg_repr(*(ctext.style)) : NULL;
    char* sjust = ColorJustify_repr(ctext.just);
    asprintf_or_return(
        NULL,
        &repr,
        "ColorText {.text=%s, .fore=%s, .back=%s, .style=%s, .just=%s}",
        stext ? stext : "NULL",
        sfore ? sfore : "NULL",
        sback ? sback : "NULL",
        sstyle ? sstyle : "NULL",
        sjust ? sjust : "<couldn't allocate for .just repr>"
    );
    free(stext);
    free(sfore);
    free(sback);
    free(sstyle);
    free(sjust);
    return repr;
}

/*! Set the ColorJustify method for a ColorText, and return the ColorText.

    \details
    This is to facilitate the justification macros. If you already have a pointer
    to a ColorText, you can just do `ctext->just = just;`. The purpose of this
    is to allow `ColorText_set_just(ColorText_to_ptr(...), ...)` to work.

    \po ctext The ColorText to set the justification method for.
    \pi cjust The ColorJustify struct to use.

    \return   The same pointer that was given as `ctext`.

    \sa ColorText
*/
ColorText* ColorText_set_just(ColorText* ctext, ColorJustify cjust) {
    if (!ctext) return ctext;
    ctext->just = cjust;
    return ctext;
}

/*! Initializes an existing ColorText from 1 mandatory \string, and optional
    fore, back, and style args (pointers to ColorArgs).

    \po ctext A ColorText to initialize with values.
    \pi text  Text to colorize (a regular string).
    \pi ...   A `va_list` with ColorArgs pointers for fore, back, and style, in any order.
    \return   An initialized ColorText struct.

    \sa ColorText
*/
void ColorText_set_values(ColorText* ctext, char* text, ...) {
    // Argument list must have ColorArg with NULL members at the end.
    *ctext = ColorText_empty();
    ctext->text = text;

    va_list args;
    va_start(args, text);

    ColorArg *arg = NULL;
    while_colr_va_arg(args, ColorArg*, arg) {
        if (!arg) continue;
        assert(ColorArg_is_ptr(arg));
        // It's a ColorArg.
        if (arg->type == FORE) {
            ctext->fore = arg;
        } else if (arg->type == BACK) {
            ctext->back = arg;
        } else if (arg->type == STYLE) {
            ctext->style = arg;
        } else if (ColorArg_is_empty(*arg)) {
            // Empty ColorArgs are assigned in the order they were passed in.
            if (!ctext->fore) {
                ctext->fore = arg;
            } else if (!ctext->back) {
                ctext->back = arg;
            } else if (!ctext->style) {
                ctext->style = arg;
            }
        }
    }
    va_end(args);
}
/*! Copies a ColorText into allocated memory and returns the pointer.

    \details
    You must free() the memory if you call this directly.

    \pi ctext ColorText to copy/allocate for.
    \return   \parblock
                  Pointer to a heap-allocated ColorText.
                  \mustfree
                  \maybenullalloc
              \endparblock

    \sa ColorText
*/
ColorText* ColorText_to_ptr(ColorText ctext) {
    ColorText* p = malloc(sizeof(ColorText));
    if (!p) return NULL;
    ctext.marker = COLORTEXT_MARKER;
    *p = ctext;
    return p;
}

/*! Stringifies a ColorText struct, creating a mix of escape codes and text.

    \pi ctext ColorText to stringify.
    \return   \parblock
                  An allocated string with text/escape-codes.
                  \mustfree
                  \maybenullalloc
                  If the ColorText has a `NULL` `.text` member, `NULL` is returned.
              \endparblock

    \sa ColorText
*/
char* ColorText_to_str(ColorText ctext) {
    // No text? No string.
    if (!ctext.text) return NULL;
    // Make room for any fore/back/style code combo plus the reset_all code.
    char* final = calloc(ColorText_length(ctext), sizeof(char));
    bool do_reset = (ctext.style || ctext.fore || ctext.back);
    if (ctext.style && !ColorArg_is_empty(*(ctext.style))) {
        char* stylecode = ColorArg_to_esc(*(ctext.style));
        if (stylecode) {
            strcat(final, stylecode);
            free(stylecode);
        }
    }
    if (ctext.fore && !ColorArg_is_empty(*(ctext.fore))) {
        char* forecode = ColorArg_to_esc(*(ctext.fore));
        if (forecode) {
            strcat(final, forecode);
            free(forecode);
        }
    }
    if (ctext.back && !ColorArg_is_empty(*(ctext.back))) {
        char* backcode = ColorArg_to_esc(*(ctext.back));
        if (backcode) {
            strcat(final, backcode);
            free(backcode);
        }
    }
    strcat(final, ctext.text);
    if (do_reset) colr_append_reset(final);
    char* justified = NULL;
    switch (ctext.just.method) {
        // TODO: It would be nice to do this all in one pass, but this works.
        case JUST_NONE:
            break;
        case JUST_LEFT:
            justified = colr_str_ljust(final, ctext.just.width, ctext.just.padchar);
            free(final);
            return justified;
        case JUST_RIGHT:
            justified = colr_str_rjust(final, ctext.just.width, ctext.just.padchar);
            free(final);
            return justified;
        case JUST_CENTER:
            justified = colr_str_center(final, ctext.just.width, ctext.just.padchar);
            free(final);
            return justified;
    }
    return final;
}

/*! Compares two ColorTypes.

    \details
    This is used to implement colr_eq().

    \pi a   The first ColorType to compare.
    \pi b   The second ColorType to compare.
    \return `true` if they are equal, otherwise `false`.

    \sa ColorType
*/
bool ColorType_eq(ColorType a, ColorType b) {
    return (a == b);
}

/*! Determine which type of color value is desired by name.

    \details
    Example:
        - "red" == TYPE_BASIC
        - "253" == TYPE_EXTENDED
        - "123,55,67" == TYPE_RGB

    \pi arg Color name to get the ColorType for.

    \retval ColorType value on success.
    \retval TYPE_INVALID for invalid color names/strings.
    \retval TYPE_INVALID_EXT_RANGE for ExtendedValues outside of 0-255.
    \retval TYPE_INVALID_RGB_RANGE for rgb values outside of 0-255.

    \examplecodefor{ColorType_from_str,.c}
    #include "colr.h"

    int main(int argc, char** argv) {
        char* userarg;
        if (argc == 1) {
            if (asprintf(&userarg, "%s", "123,54,25") < 1) return EXIT_FAILURE;
        } else {
            if (asprintf(&userarg, "%s",  argv[1]) < 1) return EXIT_FAILURE;
        }
        ColorType type = ColorType_from_str(userarg);
        if (!ColorType_is_invalid(type)) {
            char* repr = colr_repr(type);
            printf("User passed in a %s, %s\n", repr, userarg);
            free(repr);
        } else {
            printf("User passed in an invalid color name: %s\n", userarg);
        }
        free(userarg);
    }
    \endexamplecode

    \sa ColorType
*/
ColorType ColorType_from_str(const char* arg) {
    if (!arg) return TYPE_INVALID;
    if (arg[0] == '\0') return TYPE_INVALID;
    // Try basic colors.
    if (BasicValue_is_valid(BasicValue_from_str(arg))) {
        return TYPE_BASIC;
    }
    // Extended colors.
    int x_ret = ExtendedValue_from_str(arg);
    if (x_ret == COLOR_INVALID_RANGE) {
        return TYPE_INVALID_EXT_RANGE;
    } else if (ExtendedValue_is_valid(x_ret)) {
        return TYPE_EXTENDED;
    }
    // Try styles.
    if (StyleValue_is_valid(StyleValue_from_str(arg))) {
        return TYPE_STYLE;
    }
    // Try rgb.
    RGB rgb;
    int rgb_ret = RGB_from_str(arg, &rgb);
    if (rgb_ret == COLOR_INVALID_RANGE) {
        return TYPE_INVALID_RGB_RANGE;
    } else if (rgb_ret != COLOR_INVALID) {
        return TYPE_RGB;
    }
    return TYPE_INVALID;
}

/*! Check to see if a ColorType value is considered invalid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered invalid, otherwise `false`.

    \sa ColorType
*/
bool ColorType_is_invalid(ColorType type) {
    return !(bool_colr_enum(type));
}

/*! Check to see if a ColorType value is considered valid.

    \pi type ColorType value to check.
    \return  `true` if the value is considered valid, otherwise `false`.

    \sa ColorType
*/
bool ColorType_is_valid(ColorType type) {
    return bool_colr_enum(type);
}

/*! Creates a \string representation of a ColorType.

    \pi type A ColorType to get the type from.
    \return  \parblock
                 A pointer to an allocated string.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorType
*/
char* ColorType_repr(ColorType type) {
    char* typestr;
    switch (type) {
        case TYPE_NONE:
            asprintf_or_return(NULL, &typestr, "TYPE_NONE");
            break;
        case TYPE_BASIC:
            asprintf_or_return(NULL, &typestr, "TYPE_BASIC");
            break;
        case TYPE_EXTENDED:
            asprintf_or_return(NULL, &typestr, "TYPE_EXTENDED");
            break;
        case TYPE_RGB:
            asprintf_or_return(NULL, &typestr, "TYPE_RGB");
            break;
        case TYPE_STYLE:
            asprintf_or_return(NULL, &typestr, "TYPE_STYLE");
            break;
        case TYPE_INVALID:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID");
            break;
        case TYPE_INVALID_STYLE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_STYLE");
            break;
        case TYPE_INVALID_EXT_RANGE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_EXT_RANGE");
            break;
        case TYPE_INVALID_RGB_RANGE:
            asprintf_or_return(NULL, &typestr, "TYPE_INVALID_RGB_RANGE");
            break;
    }
    return typestr;
}

/*! Create a human-friendly \string representation for a ColorType.

    \pi type A ColorType to get the name for.
    \return  \parblock
                 An allocated string with the result.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorType
*/
char* ColorType_to_str(ColorType type) {
    char* typestr = NULL;
    switch (type) {
        case TYPE_NONE:
            asprintf_or_return(NULL, &typestr, "none");
            break;
        case TYPE_BASIC:
            asprintf_or_return(NULL, &typestr, "basic");
            break;
        case TYPE_EXTENDED:
            asprintf_or_return(NULL, &typestr, "ext");
            break;
        case TYPE_RGB:
            asprintf_or_return(NULL, &typestr, "rgb");
            break;
        case TYPE_STYLE:
            asprintf_or_return(NULL, &typestr, "style");
            break;
        case TYPE_INVALID:
            asprintf_or_return(NULL, &typestr, "invalid");
            break;
        case TYPE_INVALID_STYLE:
            asprintf_or_return(NULL, &typestr, "invalid style");
            break;
        case TYPE_INVALID_EXT_RANGE:
            asprintf_or_return(NULL, &typestr, "invalid ext");
            break;
        case TYPE_INVALID_RGB_RANGE:
            asprintf_or_return(NULL, &typestr, "invalid rgb");
            break;
    }
    return typestr;

}
/*! Create an "empty" ColorValue.

    \details
    This is used with ColorArg_empty() to build ColorArgs that don't do anything,
    where using `NULL` has a different meaning inside the \colrmacros.

    \return `(ColorValue){.type=TYPE_NONE, .basic=0, .ext=0, .rgb=(RGB){0, 0, 0}}`

    \sa ColorArg
    \sa ColorArg_empty
    \sa ColorArg_is_empty
    \sa ColorValue_is_empty
*/
ColorValue ColorValue_empty(void) {
    return (ColorValue){
        .type=TYPE_NONE,
        .basic=basic(0),
        .ext=ext(0),
        .rgb=rgb(0, 0, 0),
        .style=RESET_ALL,
    };
}

/*! Compares two ColorValue structs.
    \details
    They are considered "equal" if all of their members match.

    \pi a   First ColorValue to compare.
    \pi b   Second ColorValue to compare.

    \return `true` if they are equal, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_eq(ColorValue a, ColorValue b) {
    return (
        (a.type == b.type) &&
        (a.basic == b.basic) &&
        (a.ext == b.ext) &&
        (a.style == b.style) &&
        RGB_eq(a.rgb, b.rgb)
    );
}

/*! Create a \string representation of a ColorValue with a human-friendly
    type/name.

    \pi cval A ColorValue to get an example string for.
    \return  \parblock
                 An allocated string with the result.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorValue
*/
char* ColorValue_example(ColorValue cval) {
    char* valstr;
    char* typestr = ColorType_to_str(cval.type);
    if (!typestr) return NULL;

    switch (cval.type) {
        case TYPE_RGB:
            valstr = RGB_to_str(cval.rgb);
            break;
        case TYPE_BASIC:
            valstr = BasicValue_to_str(cval.basic);
            break;
        case TYPE_EXTENDED:
            valstr = ExtendedValue_to_str(cval.ext);
            break;
        case TYPE_STYLE:
            valstr = StyleValue_to_str(cval.style);
            break;
        default:
            asprintf_or_return(NULL, &valstr, "-");
    }
    if (!valstr) return NULL;
    char* example;
    asprintf_or_return(
        NULL,
        &example,
        "%13s %-12s",
        typestr,
        valstr
    );
    free(typestr);
    free(valstr);
    return example;
}

/*! Convert an escape-code \string into a ColorValue.

    \pi s    \parblock
                 An escape-code string to parse.
                 \mustnull
             \endparblock
    \return  A ColorValue (with no fore/back information, only the color type and value).
    \retval  For invalid strings, the `.type` member can be one of:
             - TYPE_INVALID
             - TYPE_INVALID_EXT_RANGE
             - TYPE_INVALID_RGB_RANGE

    \sa ColorValue
    \sa ColorArg_from_esc
*/
ColorValue ColorValue_from_esc(const char* s) {
    if (!s || s[0] == '\0') return ColorValue_from_value(TYPE_INVALID, NULL);
    // RGB?
    RGB rgb;
    int rgb_ret = RGB_from_esc(s, &rgb);
    if (rgb_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_RGB_RANGE, NULL);
    } else if (rgb_ret != COLOR_INVALID) {
        return ColorValue_from_value(TYPE_RGB, &rgb);
    }
    // Extended colors?
    int x_ret = ExtendedValue_from_esc(s);
    if (x_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_EXT_RANGE, NULL);
    } else if (ExtendedValue_is_valid(x_ret)) {
        // Need to cast back into a real ExtendedValue now that I know it's
        // not invalid. Also, ColorValue_from_value expects a pointer, to
        // help with it's "dynamic" uses.
        ExtendedValue xval = ext(x_ret);
        return ColorValue_from_value(TYPE_EXTENDED, &xval);
    }
    // Basic?
    int b_ret = BasicValue_from_esc(s);
    if (BasicValue_is_valid(b_ret)) {
        BasicValue bval = (BasicValue)b_ret;
        return ColorValue_from_value(TYPE_BASIC, &bval);
    }
    // Style?
    int s_ret = StyleValue_from_esc(s);
    if (StyleValue_is_valid(s_ret)) {
        StyleValue sval = (StyleValue)s_ret;
        return ColorValue_from_value(TYPE_STYLE, &sval);
    }
    return ColorValue_from_value(TYPE_INVALID, NULL);
}

/*! Create a ColorValue from a known color name, or RGB \string.

    \pi s    A string to parse the color name from (can be an RGB string).
    \return  A ColorValue (with no fore/back information, only the color type and value).

    \retval  For invalid strings, the `.type` member can be one of:
             - TYPE_INVALID
             - TYPE_INVALID_EXT_RANGE
             - TYPE_INVALID_RGB_RANGE

    \sa ColorValue
*/
ColorValue ColorValue_from_str(const char* s) {
    if (!s || s[0] == '\0') return ColorValue_from_value(TYPE_INVALID, NULL);
    // Hex colors should be converted to RGB first.
    if (s[0] == '#') {
        // This ensure that translations always show the correct RGB,
        // even though the ExtendedValue will be fudged to the closest match.
        RGB rgbhex;
        int rgb_ret = RGB_from_str(s, &rgbhex);
        if (rgb_ret == COLOR_INVALID_RANGE) {
            return ColorValue_from_value(TYPE_INVALID_RGB_RANGE, NULL);
        } else if (rgb_ret != TYPE_INVALID) {
            return ColorValue_from_value(TYPE_RGB, &rgbhex);
        }
    }
    // Basic color name?
    int b_ret = BasicValue_from_str(s);
    if (BasicValue_is_valid(b_ret)) {
        BasicValue bval = (BasicValue)b_ret;
        return ColorValue_from_value(TYPE_BASIC, &bval);
    }
    // Extended colors, or known extended name?
    int x_ret = ExtendedValue_from_str(s);
    if (x_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_EXT_RANGE, NULL);
    } else if (ExtendedValue_is_valid(x_ret)) {
        // Need to cast back into a real ExtendedValue now that I know it's
        // not invalid. Also, ColorValue_from_value expects a pointer, to
        // help with it's "dynamic" uses.
        ExtendedValue xval = ext(x_ret);
        return ColorValue_from_value(TYPE_EXTENDED, &xval);
    }
    // Style name?
    int s_ret = StyleValue_from_str(s);
    if (StyleValue_is_valid(s_ret)) {
        StyleValue sval = (StyleValue)s_ret;
        return ColorValue_from_value(TYPE_STYLE, &sval);
    }
    // RGB strings?
    RGB rgb;
    int rgb_ret = RGB_from_str(s, &rgb);
    if (rgb_ret == COLOR_INVALID_RANGE) {
        return ColorValue_from_value(TYPE_INVALID_RGB_RANGE, NULL);
    } else if (rgb_ret != TYPE_INVALID) {
        return ColorValue_from_value(TYPE_RGB, &rgb);
    }

    return ColorValue_from_value(TYPE_INVALID, NULL);
}

/*! Used with the color_val macro to dynamically create a ColorValue based
    on it's argument type.

    \pi type A ColorType value, to mark the type of ColorValue.
    \pi p    A pointer to either a BasicValue, ExtendedValue, or a RGB.

    \return \parblock
                A ColorValue struct with the appropriate `.type` member set for
                the value that was passed. For invalid types the `.type` member may
                be set to one of:
                - TYPE_INVALID
                - TYPE_INVALID_EXT_RANGE
                - TYPE_INVALID_RGB_RANGE
            \endparblock

    \sa ColorValue
*/
ColorValue ColorValue_from_value(ColorType type, void* p) {
    if (
        type == TYPE_INVALID ||
        type == TYPE_INVALID_EXT_RANGE ||
        type == TYPE_INVALID_RGB_RANGE
        ) {
        return (ColorValue){.type=type};
    } else if (!p) {
        return (ColorValue){.type=TYPE_INVALID};
    }
    if (type == TYPE_BASIC) {
        BasicValue *bval = p;
        BasicValue use_bval = *bval;
        // NONE has special meaning. It's not invalid, it's just "no preference".
        if (use_bval == BASIC_NONE) use_bval = basic(RESET);
        return (ColorValue){.type=TYPE_BASIC, .basic=use_bval};
    } else if (type == TYPE_EXTENDED) {
        ExtendedValue *eval = p;
        return (ColorValue){.type=TYPE_EXTENDED, .ext=*eval};
    } else if (type == TYPE_STYLE) {
        StyleValue *sval = p;
        ColorType ctype = TYPE_STYLE;
        if ((*sval < STYLE_MIN_VALUE) || (*sval > STYLE_MAX_VALUE)) {
            ctype = TYPE_INVALID_STYLE;
            *sval = STYLE_INVALID;
        } else if (*sval == STYLE_INVALID) {
            ctype = TYPE_INVALID_STYLE;
        }
        return (ColorValue){.type=ctype, .style=*sval};
    } else if (type == TYPE_RGB) {
        RGB *rgbval = p;
        return (ColorValue){.type=TYPE_RGB, .rgb=*rgbval};
    }
    // TYPE_NONE:
    return (ColorValue){.type=type};
}

/*! Checks to see if a ColorValue has a BasicValue set.

    \pi cval ColorValue to check.
    \pi bval BasicValue to look for.

    \return `true` if the ColorValue has the exact BasicValue set.

    \sa ColorValue
*/
bool ColorValue_has_BasicValue(ColorValue cval, BasicValue bval) {
    return (cval.type == TYPE_BASIC) && (cval.basic == bval);
}

/*! Checks to see if a ColorValue has a ExtendedValue set.

    \pi cval ColorValue to check.
    \pi eval ExtendedValue to look for.

    \return `true` if the ColorValue has the exact ExtendedValue set.

    \sa ColorValue
*/
bool ColorValue_has_ExtendedValue(ColorValue cval, ExtendedValue eval) {
    return (cval.type == TYPE_EXTENDED) && (cval.ext == eval);
}

/*! Checks to see if a ColorValue has a StyleValue set.

    \pi cval ColorValue to check.
    \pi sval StyleValue to look for.

    \return `true` if the ColorValue has the exact StyleValue set.

    \sa ColorValue
*/
bool ColorValue_has_StyleValue(ColorValue cval, StyleValue sval) {
    return (cval.type == TYPE_STYLE) && (cval.style == sval);
}

/*! Checks to see if a ColorValue has a RGB value set.

    \pi cval   ColorValue to check.
    \pi rgb    RGB value to look for.

    \return    `true` if the ColorValue has the exact RGB value set.

    \sa ColorValue
*/
bool ColorValue_has_RGB(ColorValue cval, RGB rgb) {
    return (cval.type == TYPE_RGB) && RGB_eq(cval.rgb, rgb);
}

/*! Checks to see if a ColorValue is an empty placeholder.

    \pi cval ColorValue to check.
    \return  `true` if the ColorValue is "empty", otherwise `false`.

    \sa ColorValue
    \sa ColorValue_empty
    \sa ColorArg_empty
    \sa ColorArg_is_empty
*/
bool ColorValue_is_empty(ColorValue cval) {
    return (cval.type == TYPE_NONE);
}

/*! Checks to see if a ColorValue holds an invalid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is invalid, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_is_invalid(ColorValue cval) {
    return !(bool_colr_enum(cval.type));
}

/*! Checks to see if a ColorValue holds a valid value.

    \pi cval ColorValue struct to check.
    \return `true` if the value is valid, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_is_valid(ColorValue cval) {
    return bool_colr_enum(cval.type);
}

/*! Returns the length in bytes needed to allocate a \string built with
    ColorValue_to_esc() with the specified ArgType and ColorValue.

    \pi type ArgType (`FORE`, `BACK`, `STYLE`)
    \pi cval ColorValue to use.

    \return  The length (`size_t`) needed to allocate a ColorValue's string,
             or `1` (size of an empty string) for invalid/empty arg types/values.

    \sa ColorValue
*/
size_t ColorValue_length(ArgType type, ColorValue cval) {
    switch (type) {
        case FORE:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                // This case is not valid, but I will try to do the right thing.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    return STYLE_LEN;
                // LCOV_EXCL_STOP
                default:
                    // Empty string for invalid/empty values.
                    return 1;
                }
        case BACK:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                // This case is not even valid, but okay.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    return STYLE_LEN;
                // LCOV_EXCL_STOP
                default:
                    // Empty string for invalid/empty values.
                    return 1;
                }
        case STYLE:
            // The default case is valid (for invalid types), but any other
            // non-STYLE type is a product of mismatched info.
            assert(cval.type == TYPE_STYLE || !bool_colr_enum(cval.type));
            switch (cval.type) {
                case TYPE_STYLE:
                    return STYLE_LEN;
                // All of these other cases are a product of mismatched info.
                // LCOV_EXCL_START
                case TYPE_BASIC:
                    return CODE_LEN;
                case TYPE_EXTENDED:
                    return CODEX_LEN;
                case TYPE_RGB:
                    return CODE_RGB_LEN;
                // LCOV_EXCL_STOP
                // Except this one, it's for TYPE_INVALID stuff.
                default:
                    // Empty string for invalid/empty values.
                    return 1;
            }
        default:
            // Empty string for invalid/empty arg type.
            return 1;
    }
    // Unreachable.
    return 1;
}
/*! Creates a \string representation of a ColorValue.

    \pi cval    A ColorValue to get the type and value from.
    \return     \parblock
                    A pointer to an allocated string.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa ColorValue
*/
char* ColorValue_repr(ColorValue cval) {
    switch (cval.type) {
        case TYPE_RGB:
            return RGB_repr(cval.rgb);
        case TYPE_BASIC:
            return BasicValue_repr(cval.basic);
        case TYPE_EXTENDED:
            return ExtendedValue_repr(cval.ext);
        case TYPE_STYLE:
            return StyleValue_repr(cval.style);
        default:
            return ColorType_repr(cval.type);
    }
}

/*! Converts a ColorValue into an escape code \string.

    \pi type ArgType (FORE, BACK, STYLE) to build the escape code for.
    \pi cval ColorValue to get the color value from.

    \return  \parblock
                 An allocated string with the appropriate escape code.
                 For invalid values, an empty string is returned.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa ColorValue
*/
char* ColorValue_to_esc(ArgType type, ColorValue cval) {
    char* codes;
    switch (type) {
        case FORE:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_fg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_fgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_fg_RGB(codes, cval.rgb);
                    return codes;
                // This case is not valid, but I will try to do the right thing.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                // LCOV_EXCL_STOP
                default:
                    return NULL;
                }
        case BACK:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_bg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_bgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_bg_RGB(codes, cval.rgb);
                    return codes;
                // This case is not even valid, but okay.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                // LCOV_EXCL_STOP
                default:
                    return NULL;
                }
        case STYLE:
            assert(cval.type == TYPE_STYLE || !bool_colr_enum(cval.type));
            switch (cval.type) {
                case TYPE_STYLE:
                    // This is the only appropriate case.
                    codes = alloc_style();
                    format_style(codes, cval.style);
                    return codes;
                // All of these other cases are a product of mismatched info.
                // LCOV_EXCL_START
                case TYPE_BASIC:
                    codes = alloc_basic();
                    format_fg(codes, cval.basic);
                    return codes;
                case TYPE_EXTENDED:
                    codes = alloc_extended();
                    format_fgx(codes, cval.ext);
                    return codes;
                case TYPE_RGB:
                    codes = alloc_rgb();
                    format_fg_RGB(codes, cval.rgb);
                    return codes;
                // LCOV_EXCL_STOP
                // Except this one, it's for TYPE_INVALID stuff.
                default:
                    return NULL;
            }
        default:
            return NULL;
    }
    return NULL;
}

/*! Converts a ColorValue into an escape code \string and fills the destination
    string.

    \details
    For invalid ArgType/ColorValue combinations, `dest[0]` is set to `'\0'`.

    \po dest \parblock
                Destination string for the escape code string.
                <em>Must have room for the code type being used</em>.
             \endparblock
    \pi type ArgType (FORE, BACK, STYLE) to build the escape code for.
    \pi cval ColorValue to get the color value from.

    \return  `true` if a proper ArgType/ColorValue combination was used, otherwise `false`.

    \sa ColorValue
*/
bool ColorValue_to_esc_s(char* dest, ArgType type, ColorValue cval) {
    switch (type) {
        case FORE:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    format_fg(dest, cval.basic);
                    return true;
                case TYPE_EXTENDED:
                    format_fgx(dest, cval.ext);
                    return true;
                case TYPE_RGB:
                    format_fg_RGB(dest, cval.rgb);
                    return true;
                // This case is not valid, but I will try to do the right thing.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    format_style(dest, cval.style);
                    return true;
                // LCOV_EXCL_STOP
                default:
                    dest[0] = '\0';
                    return false;
                }
        case BACK:
            assert(cval.type != TYPE_STYLE);
            switch (cval.type) {
                case TYPE_BASIC:
                    format_bg(dest, cval.basic);
                    return true;
                case TYPE_EXTENDED:
                    format_bgx(dest, cval.ext);
                    return true;
                case TYPE_RGB:
                    format_bg_RGB(dest, cval.rgb);
                    return true;
                // This case is not even valid, but okay.
                // LCOV_EXCL_START
                case TYPE_STYLE:
                    format_style(dest, cval.style);
                    return true;
                // LCOV_EXCL_STOP
                default:
                    dest[0] = '\0';
                    return false;
                }
        case STYLE:
            // The default case is valid (for invalid types), but any other
            // non-STYLE type is a product of mismatched info.
            assert(cval.type == TYPE_STYLE || !bool_colr_enum(cval.type));
            switch (cval.type) {
                case TYPE_STYLE:
                    // This is the only appropriate case.
                    format_style(dest, cval.style);
                    return true;
                // All of these other cases are a product of mismatched info.
                // LCOV_EXCL_START
                case TYPE_BASIC:
                    format_fg(dest, cval.basic);
                    return true;
                case TYPE_EXTENDED:
                    format_fgx(dest, cval.ext);
                    return true;
                case TYPE_RGB:
                    format_fg_RGB(dest, cval.rgb);
                    return true;
                // LCOV_EXCL_STOP
                // Except this one, it's for TYPE_INVALID stuff.
                default:
                    dest[0] = '\0';
                    return false;
            }
        default:
            dest[0] = '\0';
            return false;
    }
    dest[0] = '\0';
    return false;
}
/*! Compares two BasicValues.

    \details
    This is used to implement colr_eq().

    \pi a   The first BasicValue to compare.
    \pi b   The second BasicValue to compare.
    \return `true` if they are equal, otherwise `false`.

    \sa BasicValue
*/
bool BasicValue_eq(BasicValue a, BasicValue b) {
    return (a == b);
}

/*! Convert an escape-code \string to an actual BasicValue enum value.

    \pi s   \parblock
                Escape-code string.
                \mustnull
            \endparblock
    \retval BasicValue value on success.
    \retval BASIC_INVALID on error (or if \p s is `NULL`).
    \retval BASIC_INVALID_RANGE if the code number was outside of the range `0-255`.

    \sa BasicValue
*/
BasicValue BasicValue_from_esc(const char* s) {
    if (!s) return BASIC_INVALID;
    unsigned short escnum;
    if (sscanf(s, "\x1b[%hum", &escnum) != 1) {
        return BASIC_INVALID;
    }
    // Outside the range of a basic escape code?
    if ((escnum < 30) || (escnum > 107)) return BASIC_INVALID_RANGE;
    else if ((escnum > 49) && (escnum < 90)) return BASIC_INVALID_RANGE;
    // Within range, do some checks and subtract to get a BasicValue.
    if (escnum < 40) return basic(escnum - 30);
    else if (escnum < 50) return basic(escnum - 40);
    else if (escnum < 100) return basic(escnum - 80);
    // escnum < 108
    return basic(escnum - 90);
}

/*! Convert named argument to an actual BasicValue enum value.

    \pi arg Color name to find the BasicValue for.
    \return BasicValue value on success, or BASIC_INVALID on error.

    \sa BasicValue
*/
BasicValue BasicValue_from_str(const char* arg) {
    if (!arg) {
        return BASIC_INVALID;
    }
    char* arglower = colr_str_to_lower(arg);
    if (!arglower) return BASIC_INVALID;
    for (size_t i = 0; i < basic_names_len; i++) {
        if (!strcmp(arglower, basic_names[i].name)) {
            free(arglower);
            return basic_names[i].value;
        }
    }
    free(arglower);
    return BASIC_INVALID;
}

/*! Determines whether a BasicValue is invalid.

    \pi bval A BasicValue to check.
    \return  `true` if the value is considered invalid, otherwise `false`.

    \sa BasicValue
*/
bool BasicValue_is_invalid(BasicValue bval) {
    return ((bval == BASIC_INVALID) || (bval == BASIC_INVALID_RANGE));
}

/*! Determines whether a BasicValue is valid.

    \pi bval A BasicValue to check.
    \return  `true` if the value is considered valid, otherwise `false`.

    \sa BasicValue
*/
bool BasicValue_is_valid(BasicValue bval) {
    return ((bval != BASIC_INVALID) && (bval != BASIC_INVALID_RANGE));
}

/*! Creates a \string representation of a BasicValue.

    \pi bval A BasicValue to get the value from.
    \return  \parblock
                 A pointer to an allocated string.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa BasicValue
*/
char* BasicValue_repr(BasicValue bval) {
    char* repr;
    switch (bval) {
        case BASIC_INVALID_RANGE:
            asprintf_or_return(NULL, &repr, "(BasicValue) BASIC_INVALID_RANGE");
            break;
        case BASIC_INVALID:
            asprintf_or_return(NULL, &repr, "(BasicValue) BASIC_INVALID");
            break;
        case BASIC_NONE:
            asprintf_or_return(NULL, &repr, "(BasicValue) BASIC_NONE");
            break;
        case BLACK:
            asprintf_or_return(NULL, &repr, "(BasicValue) BLACK");
            break;
        case RED:
            asprintf_or_return(NULL, &repr, "(BasicValue) RED");
            break;
        case GREEN:
            asprintf_or_return(NULL, &repr, "(BasicValue) GREEN");
            break;
        case YELLOW:
            asprintf_or_return(NULL, &repr, "(BasicValue) YELLOW");
            break;
        case BLUE:
            asprintf_or_return(NULL, &repr, "(BasicValue) BLUE");
            break;
        case MAGENTA:
            asprintf_or_return(NULL, &repr, "(BasicValue) MAGENTA");
            break;
        case CYAN:
            asprintf_or_return(NULL, &repr, "(BasicValue) CYAN");
            break;
        case WHITE:
            asprintf_or_return(NULL, &repr, "(BasicValue) WHITE");
            break;
        case UNUSED:
            asprintf_or_return(NULL, &repr, "(BasicValue) UNUSED");
            break;
        case RESET:
            asprintf_or_return(NULL, &repr, "(BasicValue) RESET");
            break;
        case LIGHTBLACK:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTBLACK");
            break;
        case LIGHTRED:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTRED");
            break;
        case LIGHTGREEN:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTGREEN");
            break;
        case LIGHTYELLOW:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTYELLOW");
            break;
        case LIGHTBLUE:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTBLUE");
            break;
        case LIGHTMAGENTA:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTMAGENTA");
            break;
        case LIGHTCYAN:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTCYAN");
            break;
        case LIGHTWHITE:
            asprintf_or_return(NULL, &repr, "(BasicValue) LIGHTWHITE");
            break;
        default:
            // Should never happen, but the value will be known if it does.
            asprintf_or_return(NULL, &repr, "(Invalid BasicValue) %d", bval);
    }
    return repr;
}

/*! Converts a fore/back BasicValue to the actual ansi code number.

    \pi type ArgType (FORE/BACK).
    \pi bval BasicValue to convert.
    \return An integer usable with basic escape code fore/back colors.

    \sa BasicValue
*/
int BasicValue_to_ansi(ArgType type, BasicValue bval) {
    int use_value = (int)bval;
    if (bval < 0) {
        // Invalid or NONE, just use the RESET code.
        return RESET + (type == BACK ? 40 : 30);
    }
    if (bval < 10) {
        // Normal back colors.
        return use_value + (type == BACK ? 40 : 30);
    }
    // Bright back colors.
    return use_value + (type == BACK ? 90 : 80);
}

/*! Create a human-friendly \string representation for a BasicValue.

    \pi bval BasicValue to get the name for.
    \return  \parblock
                 An allocated string with the result.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa BasicValue
*/
char* BasicValue_to_str(BasicValue bval) {
    char* name;
    for (size_t i = 0; i < basic_names_len; i++) {
        if (bval == basic_names[i].value) {
            asprintf_or_return(NULL, &name, "%s", basic_names[i].name);
            return name;
        }
    }
    asprintf_or_return(NULL, &name, "unknown");
    return name;
}

/*! Compares two ExtendedValues.

    \details
    This is used to implement colr_eq().

    \pi a   The first ExtendedValue to compare.
    \pi b   The second ExtendedValue to compare.
    \return `true` if they are equal, otherwise `false`.

    \sa ExtendedValue
*/
bool ExtendedValue_eq(ExtendedValue a, ExtendedValue b) {
    return (a == b);
}

/*! Convert a BasicValue into an ExtendedValue.

    \details
    BASIC_INVALID, and other invalid BasicValues will return EXT_INVALID.

    \pi bval BasicValue to convert.
    \return  An ExtendedValue `0-15` on success, otherwise EXT_INVALID.

    \sa ExtendedValue
*/
int ExtendedValue_from_BasicValue(BasicValue bval) {
    if (BasicValue_is_invalid(bval)) return EXT_INVALID;
    if (bval < 8) return ext(bval);
    if ((bval == UNUSED) || (bval == RESET)) return ext(0);
    if (bval < 18) return ext(bval - 2);
    return ext(0);
}

/*! Convert an escape-code \string to an ExtendedValue.

    \pi s   \parblock
                Escape-code string.
                \mustnull
            \endparblock
    \retval An integer in the range `0-255` on success.
    \retval EXT_INVALID on error (or if \p s is `NULL`).
    \retval EXT_INVALID_RANGE if the code number was outside of the range `0-255`.

    \sa ExtendedValue
*/
int ExtendedValue_from_esc(const char* s) {
    if (!s) return EXT_INVALID;
    short escnum;
    if (sscanf(s, "\x1b[38;5;%hdm", &escnum) != 1) {
        if (sscanf(s, "\x1b[48;5;%hdm", &escnum) != 1) return EXT_INVALID;
    }
    // Outside the range of an extended escape code?
    if ((escnum < 0) || (escnum > 255)) return EXT_INVALID_RANGE;
    return escnum;
}

/*! Create an ExtendedValue from a hex \string.

    \details
    This is not a 1:1 translation of hex to rgb. Use RGB_from_hex() for that.
    This will convert the hex string to the closest matching ExtendedValue value.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr Hex string to convert.
    \return    A value between 0 and 255 on success.
    \retval    COLOR_INVALID on error or bad values.

    \sa ExtendedValue
*/
int ExtendedValue_from_hex(const char* hexstr) {
    RGB rgb;
    if (RGB_from_hex(hexstr, &rgb) != 0) return COLOR_INVALID;
    return ExtendedValue_from_RGB(rgb);
}

/*! Create an ExtendedValue from a hex \string, but return a default value if
    the hex string is invalid.

    \details
    This is not a 1:1 translation of hex to rgb. Use RGB_from_hex_default() for that.
    This will convert the hex string to the closest matching ExtendedValue value.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr         Hex string to convert.
    \pi default_value  ExtendedValue to use for bad hex strings.
    \return            An ExtendedValue on success, or `default_value` on error.

    \sa ExtendedValue
    \sa ExtendedValue_from_hex
*/
ExtendedValue ExtendedValue_from_hex_default(const char* hexstr, ExtendedValue default_value) {
    int ret = ExtendedValue_from_hex(hexstr);
    if (ret < 0) return default_value;
    return ret;
}

/*! Convert an RGB value into the closest matching ExtendedValue.

    \pi rgb RGB value to convert.
    \return An ExtendedValue that closely matches the original RGB value.

    \sa ExtendedValue
*/
ExtendedValue ExtendedValue_from_RGB(RGB rgb) {
    // Try known names.
    for (size_t i = 0; i < colr_name_data_len; i++) {
        ColorNameData item = colr_name_data[i];
        if (RGB_eq(item.rgb, rgb)) {
            return item.ext;
        }
    }
    // Try the closest matching.
    RGB closestrgb = RGB_to_term_RGB(rgb);
    for (size_t i = 0; i < ext2rgb_map_len; i++) {
        if (RGB_eq(closestrgb, ext2rgb_map[i])) {
            return ext(i);
        }
    }
    // This can happen when coming from RGB_inverted, RGB_monochrome, etc.
    return ext(0);
}

/*! Converts a known name, integer string (0-255), or a hex \string, into an
    ExtendedValue suitable for the extended-value-based functions.

    \details
    Hex strings can be used:
        - "#ffffff" (Leading hash symbol is __NOT__ optional)
        - "#fff" (short-form)

    \details
    The `'#'` is not optional for hex strings because it is impossible to tell
    the difference between the hex value '111' and the extended value '111'
    without it.

    \pi arg Color name to find the ExtendedValue for.

    \return A value between 0 and 255 on success.
    \retval EXT_INVALID on error or bad values.
    \retval EXT_INVALID_RANGE if the number was outside of the range `0-255`.

    \sa ExtendedValue
*/
int ExtendedValue_from_str(const char* arg) {
    if (!arg) return EXT_INVALID;
    if (arg[0] == '\0') return EXT_INVALID;

    char* arglower = colr_str_to_lower(arg);
    if (!arglower) return EXT_INVALID;
    // Check the simple extended names.
    for (size_t i = 0; i < extended_names_len; i++) {
        if (!strcmp(arglower, extended_names[i].name)) {
            // A known extended value name.
            free(arglower);
            return extended_names[i].value;
        }
    }
    // Check the big known-name array.
    for (size_t i = 0; i < colr_name_data_len; i++) {
        if (!strcmp(arglower, colr_name_data[i].name)) {
            // A known name.
            free(arglower);
            return colr_name_data[i].ext;
        }
    }
    if (arglower[0] == '#') {
        // Possibly a hex string.
        int hex_ret = ExtendedValue_from_hex(arglower);
        if (hex_ret >= 0) {
            // A valid hex string.
            free(arglower);
            return hex_ret;
        }
    }
    if (!colr_str_is_digits(arg)) {
        if ((arg[0] == '-') && (strlen(arg) > 1) && colr_str_is_digits(arg + 1)) {
            free(arglower);
            // Negative number given.
            return EXT_INVALID_RANGE;
        }
        // Not a number at all.
        free(arglower);
        return EXT_INVALID;
    }

    // Regular number, hopefully 0-255, but I'll check that in a second.
    size_t length = strnlen(arglower, 5);
    if (length > 3) {
        // Definitely not 0-255.
        free(arglower);
        return EXT_INVALID_RANGE;
    }
    short usernum;
    if (sscanf(arg, "%hd", &usernum) != 1) {
        // Zero, or more than one number provided.
        // We've already proven that it's all digits, not negative, and
        // less than 4 digits. I don't know how this could `if` could fire,
        // but if it ever does. Put a test in ./test/test_ExtendedValue.c
        // under the 'ExtendedValue_from_str' tests.
        // LCOV_EXCL_START
        free(arglower);
        return EXT_INVALID;
        // LCOV_EXCL_STOP
    }
    if (usernum < 0 || usernum > 255) {
        free(arglower);
        return EXT_INVALID_RANGE;
    }
    // A valid number, 0-255.
    free(arglower);
    return (int)usernum;
}

/*! Determines whether an integer is an invalid ExtendedValue.

    \pi eval A number to check.
    \return  `true` if the value is considered invalid, otherwise `false`.

    \sa ExtendedValue
*/
bool ExtendedValue_is_invalid(int eval) {
    return ((eval < 0) || (eval > 255));
}

/*! Determines whether an integer is a valid ExtendedValue.

    \pi eval A number to check.
    \return  `true` if the value is considered valid, otherwise `false`.

    \sa ExtendedValue
*/
bool ExtendedValue_is_valid(int eval) {
    return ((eval > -1) && (eval < 256));
}

/*! Creates a \string representation of a ExtendedValue.

    \pi eval    A ExtendedValue to get the value from.
    \return     \parblock
                    A pointer to an allocated string.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa ExtendedValue
*/
char* ExtendedValue_repr(int eval) {
    char* repr;
    switch (eval) {
        case COLOR_INVALID_RANGE:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) COLOR_INVALID_RANGE");
            break;
        case COLOR_INVALID:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) COLOR_INVALID");
            break;
        default:
            asprintf_or_return(NULL, &repr, "(ExtendedValue) %d", eval);
    }
    return repr;
}

/*! Creates a human-friendly \string from an ExtendedValue's actual value,
    suitable for use  with ExtendedValue_from_str().

    \pi eval    A ExtendedValue to get the value from.
    \return     \parblock
                    A pointer to an allocated string\n
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa ExtendedValue
*/
char* ExtendedValue_to_str(ExtendedValue eval) {
    char* repr;
    asprintf_or_return(NULL, &repr, "%d", eval);
    return repr;
}

/*! Return the average for an RGB value.

    \details
    This is also it's "grayscale" value.

    \pi rgb The RGB value to get the average for.
    \return A value between `0-255`.

    \sa RGB
*/
unsigned char RGB_average(RGB rgb) {
    int sum = rgb.red + rgb.green + rgb.blue;
    // May return 0 if red, green, or blue are zero.
    return sum / 3;
}

/*! Compare two RGB structs.

    \pi a First RGB value to check.
    \pi b Second RGB value to check.
    \return `true` if \p a and \p b have the same `r`, `g`, and `b` values, otherwise `false`.

    \sa RGB
*/
bool RGB_eq(RGB a, RGB b) {
    return (
        (a.red == b.red) &&
        (a.green == b.green) &&
        (a.blue == b.blue)
    );
}

/*! Return an RGB value from a known BasicValue.

    \details
    Terminals use different values to render basic 3/4-bit escape-codes.
    The values returned from this function match the names found in
    `colr_name_data[]`.

    \pi bval A BasicValue to get the RGB value for.
    \return  An RGB value that matches the BasicValue's color.

    \sa RGB
*/
RGB RGB_from_BasicValue(BasicValue bval) {
    switch (bval) {
    case BASIC_INVALID_RANGE:
        /* fall-through */
    case BASIC_INVALID:
        /* fall-through */
    case BASIC_NONE:
        return rgb(0, 0, 0);
    case BLACK:
        return rgb(1, 1, 1);
    case RED:
        return rgb(255, 0, 0);
    case GREEN:
        return rgb(0, 255, 0);
    case YELLOW:
        return rgb(255, 255, 0);
    case BLUE:
        return rgb(0, 0, 255);
    case MAGENTA:
        return rgb(255, 0, 255);
    case CYAN:
        return rgb(0, 255, 255);
    case WHITE:
        return rgb(255, 255, 255);
    case UNUSED:
        /* fall-through */
    case RESET:
        return rgb(0, 0, 0);
    case LIGHTBLACK:
        return rgb(128, 128, 128);
    case LIGHTRED:
        return rgb(255, 85, 85);
    case LIGHTGREEN:
        return rgb(135, 255, 135);
    case LIGHTYELLOW:
        return rgb(255, 255, 215);
    case LIGHTBLUE:
        return rgb(175, 215, 215);
    case LIGHTMAGENTA:
        return rgb(255, 85, 255);
    case LIGHTCYAN:
        return rgb(215, 255, 255);
    case LIGHTWHITE:
        return rgb(255, 255, 255);
    }
    // Shouldn't happen.
    return rgb(0,0, 0); // LCOV_EXCL_LINE
}

/*! Return an RGB value from a known ExtendedValue.

    \details
    This is just a type/bounds-checked alias for `ext2rgb_map[eval]`.

    \pi eval An ExtendedValue to get the RGB value for.
    \return  An RGB value from `ext2rgb_map[]`.

    \sa RGB
*/
RGB RGB_from_ExtendedValue(ExtendedValue eval) {
    // Casting to ExtendedValue offers some bounds checks.
    return ext2rgb_map[eval];
}

/*! Convert an escape-code \string to an actual RGB value.

    \pi s      \parblock
                   Escape-code string.
                   \mustnull
               \endparblock
    \po rgb    Pointer to an RGB struct to fill in the values for.

    \retval    `0` on success, with \p rgb filled with values.
    \retval    COLOR_INVALID on error (or if \p s is `NULL`).
    \retval    COLOR_INVALID_RANGE if any code numbers were outside of the range `0-255`.

    \sa RGB
*/
int RGB_from_esc(const char* s, RGB* rgb) {
    if (!s) return COLOR_INVALID;
    short rednum;
    short greennum;
    short bluenum;
    if (sscanf(s, "\x1b[38;2;%hd;%hd;%hdm", &rednum, &greennum, &bluenum) != 3) {
        if (sscanf(s, "\x1b[48;2;%hd;%hd;%hdm", &rednum, &greennum, &bluenum) != 3) {
            return COLOR_INVALID;
        }
    }
    // Outside the range of a basic escape code?
    if ((rednum < 0) || (rednum > 255)) return COLOR_INVALID_RANGE;
    if ((greennum < 0) || (greennum > 255)) return COLOR_INVALID_RANGE;
    if ((bluenum < 0) || (bluenum > 255)) return COLOR_INVALID_RANGE;

    // Within range.
    rgb->red = rednum;
    rgb->green = greennum;
    rgb->blue = bluenum;
    return 0;
}

/*! Convert a hex color into an RGB value.
    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr \parblock
                   String to check for hex values.
                   \mustnullin
               \endparblock
    \po rgb    Pointer to an RGB struct to fill in the values for.

    \retval    0 on success, with \p rgb filled with the values.
    \retval    COLOR_INVALID for non-hex strings.

    \sa RGB
*/
int RGB_from_hex(const char* hexstr, RGB* rgb) {
    if (!hexstr) return COLOR_INVALID;
    size_t length = strnlen(hexstr, 7);
    if ((length < 3) || (length > 7)) return COLOR_INVALID;
    // Strip leading #'s.
    char copy[] = "\0\0\0\0\0\0\0\0";
    size_t char_cnt = colr_str_lstrip(copy, hexstr, length, '#');
    size_t copy_length = strlen(copy);
    if (char_cnt > 1) {
        // There was more than one # symbol, I'm not gonna be *that* nice.
        return COLOR_INVALID;
    }
    unsigned int redval, greenval, blueval;
    switch (copy_length) {
        case 3:
            // Even though the `strlen()` is 3, there is room for 7.
            copy[5] = copy[2];
            copy[4] = copy[2];
            copy[3] = copy[1];
            copy[2] = copy[1];
            copy[1] = copy[0];
            /* fall through */
        case 6:
            if (sscanf(copy, "%02x%02x%02x", &redval, &greenval, &blueval) != 3) {

                return COLOR_INVALID;
            }
            break;
        default:
            // Not a valid length.
            return COLOR_INVALID;
    }

    rgb->red = redval;
    rgb->green = greenval;
    rgb->blue = blueval;
    return 0;
}

/*! Convert a hex color into an RGB value, but use a default value when errors
    occur.

    \details
    The format for hex strings can be one of:
        - "[#]ffffff" (Leading hash symbol is optional)
        - "[#]fff" (short-form)

    \pi hexstr        \parblock
                          String to check for RGB values.
                          \mustnullin
                      \endparblock
    \po default_value An RGB value to use when errors occur.

    \return           A valid RGB value on success, or `default_value` on error.

    \sa RGB
    \sa hex
*/
RGB RGB_from_hex_default(const char* hexstr, RGB default_value) {
    if (!hexstr) return default_value;
    RGB rgb;
    int ret = RGB_from_hex(hexstr, &rgb);
    if (ret) {
        // An error occurred.
        return default_value;
    }
    return rgb;
}
/*! Convert an RGB \string into an RGB value.

    \details
    The format for RGB strings can be one of:
        - "RED,GREEN,BLUE"
        - "RED GREEN BLUE"
        - "RED:GREEN:BLUE"
        - "RED;GREEN;BLUE"
    Or hex strings can be used:
        - "#ffffff" (Leading hash symbol is __NOT__ optional)
        - "#fff" (short-form)

    \pi arg    \parblock
                   String to check for RGB values.
                   \mustnullin
               \endparblock
    \po rgb    Pointer to an RGB struct to fill in the values for.

    \retval 0 on success, with \p rgb filled with the values.
    \retval COLOR_INVALID for non-rgb strings.
    \retval COLOR_INVALID_RANGE for rgb values outside of 0-255.

    \examplecodefor{RGB_from_str,.c}
    RGB rgbval;
    int ret = RGB_from_str("123,0,234", &rgbval);
    if (ret == 0) {
        char* s = colr_cat(Colr("Test", fore(rgbval)));
        printf("%s\n", s);
        free(s);
    }
    \endexamplecode

    \sa RGB
*/
int RGB_from_str(const char* arg, RGB* rgb) {
    if (!arg) return COLOR_INVALID;
    const char* formats[] = {
        "%ld,%ld,%ld",
        "%ld %ld %ld",
        "%ld:%ld:%ld",
        "%ld;%ld;%ld",
        NULL
    };
    long userred, usergreen, userblue;
    int i = 0;
    while (formats[i]) {
        if (sscanf(arg, formats[i], &userred, &usergreen, &userblue) == 3) {
            // Found a match.
            if (userred < 0 || userred > 255) return COLOR_INVALID_RANGE;
            if (usergreen < 0 || usergreen > 255) return COLOR_INVALID_RANGE;
            if (userblue < 0 || userblue > 255) return COLOR_INVALID_RANGE;
            // Valid ranges, set values for out parameters.
            rgb->red = (unsigned char)userred;
            rgb->green = (unsigned char)usergreen;
            rgb->blue = (unsigned char)userblue;
            return 0;
        }
        i++;
    }

    // Try hex strings.
    if (arg[0] == '#') return RGB_from_hex(arg, rgb);

    // Try known names.
    char* arglower = colr_str_to_lower(arg);
    if (!arglower) return COLOR_INVALID;
    for (size_t j = 0; j < colr_name_data_len; j++) {
        if (colr_str_eq(arglower, colr_name_data[j].name)) {
            free(arglower);
            *rgb = colr_name_data[j].rgb;
            return 0;
        }
    }
    free(arglower);
    return COLOR_INVALID;
}

/*! Return a grayscale version of an RGB value.

    \pi rgb The RGB value to convert.
    \return A grayscale RGB value.

    \sa RGB
*/
RGB RGB_grayscale(RGB rgb) {
    short avg = RGB_average(rgb);
    // Don't want to return rgb(0, 0, 0) (the "reset" rgb value).
    if (!avg) avg = 1;
    return rgb(avg, avg, avg);
}

/*! Make a copy of an RGB value, with the colors "inverted" (like highlighting
    text in the terminal).

    \pi rgb The RGB value to invert.
    \return An "inverted" RGB value.

    \sa RGB
*/
RGB RGB_inverted(RGB rgb) {
    // RGB uses unsigned char, so wrap-around is okay
    // (if that's what is going to happen?).
    unsigned char r = 255 - rgb.red;
    unsigned char g = 255 - rgb.green;
    unsigned char b = 255 - rgb.blue;
    // Don't want to return rgb(0, 0, 0) (which is "reset")
    return rgb(r ? r : 1, g ? g : 1, b ? b : 1);
}

/*! Convert an RGB value into either black or white, depending on it's average
    grayscale value.

    \pi rgb The RGB value to convert.
    \return Either `rgb(1, 1, 1)` or `rgb(255, 255, 255)`.

    \sa RGB
*/
RGB RGB_monochrome(RGB rgb) {
    unsigned char avg = (rgb.red + rgb.green + rgb.blue) / 3;
    // rgb(0, 0, 0) is the "reset" rgb code.
    return avg > 128 ? rgb(255, 255, 255) : rgb(1, 1, 1);
}

/*! Converts an RGB value into a hex \string.

    \pi rgb RGB value to convert.
    \return An allocated string.
            \mustfree
            \maybenullalloc

    \sa RGB
*/
char* RGB_to_hex(RGB rgb) {
    char* s;
    asprintf_or_return(NULL, &s, "#%02x%02x%02x", rgb.red, rgb.green, rgb.blue);
    return s;
}

/*! Convert an RGB value into a human-friendly RGB \string suitable for input
    to RGB_from_str().

    \pi rgb RGB value to convert.
    \return \parblock
                An allocated string in the form `"red;green;blue"`.
                \mustfree
                \maybenullalloc
            \endparblock

    \sa RGB
*/
char* RGB_to_str(RGB rgb) {
    char* s;
    asprintf_or_return(NULL, &s, "%03d;%03d;%03d", rgb.red, rgb.green, rgb.blue);
    return s;
}

/*! Convert an RGB value into it's nearest terminal-friendly RGB value.
    \details
    This is a helper for the 'to_term' functions.

    \pi rgb RGB to convert.
    \return A new RGB with values close to a terminal code color.

    \sa RGB
*/
RGB RGB_to_term_RGB(RGB rgb) {
    int incs[6] = {0, 95, 135, 175, 215, 0xff};
    size_t inc_len = sizeof(incs) / sizeof(incs[0]);
    size_t inc_max = inc_len - 1;
    unsigned char res[3] = {-1, -1, -1};
    size_t res_pos = 0;
    unsigned char parts[3] = {rgb.red, rgb.blue, rgb.green};
    size_t part_len = sizeof(parts) / sizeof(parts[0]);
    for (size_t part_num = 0; part_num < part_len; part_num++) {
        unsigned char part = parts[part_num];
        for (size_t inc_num = 0; inc_num < inc_max; inc_num++) {
            int s = incs[inc_num]; // smaller
            int b = incs[inc_num + 1]; // bigger
            if ((s <= part) && (part <= b)) {
                int s1 = abs(s - part);
                int b1 = abs(b - part);
                unsigned char closest = b;
                if (s1 < b1) closest = s;
                res[res_pos] = closest;
                res_pos++;
                break;
            }
        }
    }
    // Convert back into nearest rgb value.
    return (RGB){.red=res[0], .blue=res[1], .green=res[2]};
}


/*! Creates a \string representation for an RGB value.
    \details
    Allocates memory for the string representation.

    \pi rgb RGB struct to get the representation for.
    \return \parblock
                Allocated string for the representation.
                \mustfree
            \endparblock

    \sa RGB
*/
char* RGB_repr(RGB rgb) {
    char* repr;
    asprintf_or_return(NULL,
        &repr,
        "RGB {.red=%d, .green=%d, .blue=%d}",
        rgb.red,
        rgb.green,
        rgb.blue
    );
    return repr;
}

/*! Compares two StyleValues.

    \details
    This is used to implement colr_eq().

    \pi a   The first StyleValue to compare.
    \pi b   The second StyleValue to compare.
    \return `true` if they are equal, otherwise `false`.

    \sa StyleValue
*/
bool StyleValue_eq(StyleValue a, StyleValue b) {
    return (a == b);
}

/*! Convert an escape-code \string to an actual StyleValue enum value.

    \pi s   \parblock
                Escape-code string.
                \mustnull
            \endparblock
    \retval StyleValue value on success.
    \retval STYLE_INVALID on error (or if \p s is `NULL`).
    \retval STYLE_INVALID_RANGE if the code number was outside of the range `0-255`.

    \sa StyleValue
*/
StyleValue StyleValue_from_esc(const char* s) {
    if (!s) return STYLE_INVALID;
    unsigned short escnum;
    if (sscanf(s, "\x1b[%hum", &escnum) != 1) {
        return STYLE_INVALID;
    }
    // Outside the range of a ColrC style escape code?
    if ((escnum > 9) && (escnum < 22)) return STYLE_INVALID_RANGE;
    else if ((escnum > 22) && (escnum < 51)) return STYLE_INVALID_RANGE;
    else if ((escnum > STYLE_MAX_VALUE)) return STYLE_INVALID_RANGE;

    // Within range.
    return (StyleValue)escnum;
}

/*! Convert a named argument to actual StyleValue enum value.

    \pi arg Style name to convert into a StyleValue.
    \return A usable StyleValue value on success, or STYLE_INVALID on error.

    \sa StyleValue
*/
StyleValue StyleValue_from_str(const char* arg) {
    if (!arg) {
        return STYLE_INVALID;
    }
    char* arglower = colr_str_to_lower(arg);
    if (!arglower) return STYLE_INVALID;
    for (size_t i = 0; i < style_names_len; i++) {
        if (!strcmp(arglower, style_names[i].name)) {
            free(arglower);
            return style_names[i].value;
        }
    }
    free(arglower);
    return STYLE_INVALID;
}

/*! Determines whether a StyleValue is invalid.

    \pi sval A StyleValue to check.
    \return  `true` if the value is considered invalid, otherwise `false`.

    \sa StyleValue
*/
bool StyleValue_is_invalid(StyleValue sval) {
    return ((sval == STYLE_INVALID) || (sval == STYLE_INVALID_RANGE));
}

/*! Determines whether a StyleValue is valid.

    \pi sval A StyleValue to check.
    \return  `true` if the value is considered valid, otherwise `false`.

    \sa StyleValue
*/
bool StyleValue_is_valid(StyleValue sval) {
    return ((sval != STYLE_INVALID) && (sval != STYLE_INVALID_RANGE));
}

/*! Create a human-friendly \string representation for a StyleValue.

    \pi sval StyleValue to get the name for.
    \return  \parblock
                 An allocated string with the result.
                 \mustfree
                 \maybenullalloc
             \endparblock

    \sa StyleValue
*/
char* StyleValue_to_str(StyleValue sval) {
    char* name;
    for (size_t i = 0; i < style_names_len; i++) {
        if (sval == style_names[i].value) {
            asprintf_or_return(NULL, &name, "%s", style_names[i].name);
            return name;
        }
    }
    asprintf_or_return(NULL, &name, "unknown");
    return name;
}

/*! Creates a \string representation of a StyleValue.

    \pi sval    A StyleValue to get the value from.
    \return     \parblock
                    A pointer to an allocated string.
                    \mustfree
                    \maybenullalloc
                \endparblock

    \sa StyleValue
*/
char* StyleValue_repr(StyleValue sval) {
    char* repr;
    switch (sval) {
        case STYLE_INVALID:
            asprintf_or_return(NULL, &repr, "(StyleValue) STYLE_INVALID");
            break;
        case STYLE_INVALID_RANGE:
            asprintf_or_return(NULL, &repr, "(StyleValue) STYLE_INVALID_RANGE");
            break;
        case STYLE_NONE:
            asprintf_or_return(NULL, &repr, "(StyleValue) STYLE_NONE");
            break;
        case RESET_ALL:
            asprintf_or_return(NULL, &repr, "(StyleValue) RESET_ALL");
            break;
        case BRIGHT:
            asprintf_or_return(NULL, &repr, "(StyleValue) BRIGHT");
            break;
        case DIM:
            asprintf_or_return(NULL, &repr, "(StyleValue) DIM");
            break;
        case ITALIC:
            asprintf_or_return(NULL, &repr, "(StyleValue) ITALIC");
            break;
        case UNDERLINE:
            asprintf_or_return(NULL, &repr, "(StyleValue) UNDERLINE");
            break;
        case FLASH:
            asprintf_or_return(NULL, &repr, "(StyleValue) FLASH");
            break;
        case HIGHLIGHT:
            asprintf_or_return(NULL, &repr, "(StyleValue) HIGHLIGHT");
            break;
        case NORMAL:
            asprintf_or_return(NULL, &repr, "(StyleValue) NORMAL");
            break;
        case STRIKETHRU:
            asprintf_or_return(NULL, &repr, "(StyleValue) STRIKETHRU");
            break;
        case FRAME:
            asprintf_or_return(NULL, &repr, "(StyleValue) FRAME");
            break;
        case ENCIRCLE:
            asprintf_or_return(NULL, &repr, "(StyleValue) ENCIRCLE");
            break;
        case OVERLINE:
            asprintf_or_return(NULL, &repr, "(StyleValue) OVERLINE");
            break;
        default:
            // Should never happen, but at least the value will be known
            // if it does.
            asprintf_or_return(NULL, &repr, "(StyleValue) %d", sval);
    }
    return repr;
}

/*! Create a \string representation for a TermSize.

    \pi ts  TermSize to get the representation for.
    \return \parblock
                An allocated string with the result.
                \mustfree
                \maybenullalloc
            \endparblock

    \sa TermSize
*/
char* TermSize_repr(TermSize ts) {
    char* repr;
    asprintf_or_return(
        NULL,
        &repr,
        "TermSize {.rows=%d, .columns=%d}",
        ts.rows,
        ts.columns
    );
    return repr;
}

/*! Rainbow-ize some text using rgb back colors, lolcat style.

    \details
    This prepends a color code to every character in the string.
    Multi-byte characters are handled properly by checking colr_mb_len(), and
    copying the bytes to the resulting string without codes between the
    multibyte characters.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      \parblock
                   The string to colorize.
                   \mustnullin
               \endparblock
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \pi spread Number of characters per color.
    \return    \parblock
                   The allocated/formatted string on success.
                   \mustfree
                   \maybenullalloc
               \endparblock
*/
char* rainbow_bg(const char* s, double freq, size_t offset, size_t spread) {
    return _rainbow(format_bg_RGB, s, freq, offset, spread);
}

/*! This is exactly like rainbow_bg(), except it uses colors that are
    closer to the standard 256-color values.

    \details
    This prepends a color code to every character in the string.
    Multi-byte characters are handled properly by checking colr_mb_len(), and
    copying the bytes to the resulting string without codes between the
    multibyte characters.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      \parblock
                   The string to colorize.
                   \mustnullin
               \endparblock
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \pi spread Number of characters per color.
    \return    \parblock
                   The allocated/formatted string on success.
                   \mustfree
                   \maybenullalloc
               \endparblock
*/
char* rainbow_bg_term(const char* s, double freq, size_t offset, size_t spread) {
    return _rainbow(format_bg_RGB_term, s, freq, offset, spread);
}

/*! Rainbow-ize some text using rgb fore colors, lolcat style.

    \details
    This prepends a color code to every character in the string.
    Multi-byte characters are handled properly by checking colr_mb_len(), and
    copying the bytes to the resulting string without codes between the
    multibyte characters.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      \parblock
                   The string to colorize.
                   \mustnullin
               \endparblock
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \pi spread Number of characters per color.
    \return    \parblock
                   The allocated/formatted string on success.
                   \mustfree
                   \maybenullalloc
               \endparblock
*/
char* rainbow_fg(const char* s, double freq, size_t offset, size_t spread) {
    return _rainbow(format_fg_RGB, s, freq, offset, spread);
}

/*! This is exactly like rainbow_fg(), except it uses colors that are
    closer to the standard 256-color values.

    \details
    This prepends a color code to every character in the string.
    Multi-byte characters are handled properly by checking colr_mb_len(), and
    copying the bytes to the resulting string without codes between the
    multibyte characters.

    \details
    The `CODE_RESET_ALL` code is appended to the result.

    \pi s      \parblock
                   The string to colorize.
                   \mustnullin
               \endparblock
    \pi freq   Frequency ("tightness") for the colors.
    \pi offset Starting offset in the rainbow.
    \pi spread Number of characters per color.
    \return    \parblock
                   The allocated/formatted string on success.
                   \mustfree
                   \maybenullalloc
               \endparblock
*/
char* rainbow_fg_term(const char* s, double freq, size_t offset, size_t spread) {
    return _rainbow(format_fg_RGB_term, s, freq, offset, spread);
}

/*! Handles multibyte character \string conversion and character iteration for
    all of the rainbow_ functions.

    \warninternal

    \pi fmter  A formatter function (RGB_fmter) that can create escape codes
               from RGB values.
    \pi s      \parblock
                   The string to "rainbowize".
                   \mustnullin
               \endparblock
    \pi freq   The "tightness" for colors.
    \pi offset The starting offset into the rainbow.
    \pi spread Number of characters per color.
    \return    \parblock
                   An allocated \string with the result.
                   \mustfree
                   \maybenullalloc
               \endparblock
*/
char* _rainbow(RGB_fmter fmter, const char* s, double freq, size_t offset, size_t spread) {
    if (!s) {
        return NULL;
    }
    if (!offset) offset = 1;
    if (freq < 0.1) freq = 0.1;
    if (spread < 1) spread = 1;

    size_t byte_len = strlen(s);
    size_t mb_len = colr_str_mb_len(s);
    if (mb_len == 0) return NULL;

    // There is an RGB code for every multibyte character in the string.
    // The entire original string, plus an rgb code for every multibyte char.
    size_t total_size = byte_len + (CODE_RGB_LEN * mb_len);
    // If we're doing a back-rainbow, we'll need to fix the newlines.
    // I don't like comparing function pointers to detect this, but it's the
    // fastest/easiest way right now.
    bool append_reset_nl = (
        (fmter == format_bg_RGB) ||
        (fmter == format_bg_RGB_term)
    );
    if (append_reset_nl) {
        // There is also a reset code prepended before every newline.
        total_size += CODE_RESET_LEN * colr_str_char_count(s, '\n');
    }
    char* out = calloc(total_size, sizeof(char));
    if (!out) return NULL;

    char codes[CODE_RGB_LEN];
    // Enough room for one (possibly multibyte) character.
    char mb_char[MB_LEN_MAX + 1];
    // Iterate over each multibyte character.
    size_t i = 0;
    size_t char_len = 0;
    while ((char_len = colr_mb_len(s + i, spread)) && colr_is_valid_mblen(char_len)) {
        // Add a rainbow code to the output.
        fmter(codes, rainbow_step(freq, offset + i));
        strcat(out, codes);
        // Write the multibyte char at (s + i), the length is char_len.
        // Basically copying the string from (s + i) through (s + i + char_len).
        snprintf(mb_char, char_len + 1, "%s", s + i);
        if (append_reset_nl && (mb_char[0] == '\n')) {
            // Prepend the reset code *before* newlines for back-rainbows.
            // NOTE: Some terminals won't colorize '\t' like they will a space.
            //       '\n\t' looks different than '\n '. I could try to account
            //       for that case, and not append the reset, but it would
            //       probably make things look worse. The style wouldn't be
            //       reset at all, and leave long lines of background color.
            //       For now, just know that '\t' is special when doing
            //       background-colors, and it's not my fault.
            strcat(out, CODE_RESET_BACK);
        }
        strcat(out, mb_char);
        // Jump past the multibyte character for the next code.
        i += char_len;
    }
    colr_append_reset(out);

    return out;
}

/*! A single step in rainbow-izing produces the next color in the "rainbow" as
    an RGB value.

    \pi freq Frequency ("tightness") of the colors.
    \pi offset Starting offset in the rainbow.

    \return  An RGB value with the next "step" in the "rainbow".
*/
RGB rainbow_step(double freq, size_t offset) {
    /*  A note about the libm (math.h) dependency:

        libm's sin() function works on every machine, gives better results
        than any hand-written sin() that I've found, and is faster.

        Something like this produces ugly "steps" in the rainbow,
        just keep libm:
            float sin(float x) {
                float res = 0, pow = x, fact = 1;
                for(int i = 0; i < 5; ++i) {
                    res += pow / fact;
                    pow *= -1 * x * x;
                    fact *= (2 * (i + 1)) * (2 * (i + 1) + 1);
                }

                return res;
            }
    */
    if (freq < 0.1) freq = 0.1;
    if (!offset) offset = 1;
    double base = freq * offset;
    double redval = sin(base + 0) * 127 + 128;
    double greenval = (sin(base + ((2 * M_PI) / 3)) * 127) + 128;
    double blueval = (sin(base + ((4 * M_PI) / 3)) * 127) + 128;
    return rgb(redval, greenval, blueval);
}
