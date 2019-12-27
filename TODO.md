# ColrC Todo List

## Warnings
[ ] Look at using a printf modifier only, instead of a whole new specifier.
    It may help with warnings (%cs instead of %R?)

## Lints
[ ] unreadVariable for sprintf() in ColorArgs_list_repr().

## Features
[ ] colr_str_replace_re(), replace a regex pattern with a string, ColorArg,
    ColorResult, or ColorText.

## Tests
Should have %100 coverage.
[ ] Tests for `colr_str_replace_re*` (regex) functions.
[ ] Tests for '*_example' functions.
[ ] Tests for _colr_ptr_repr() and _colr_ptr_to_str().
[ ] Tests for `rainbow*` functions.
[ ] Tests for ColorResult_length, when it needs to be recalculated.
[ ] Tests for ColorValue_from_esc with invalid RGB/Extended escape codes.
[ ] Tests for ExtendedValue (not fully covered).
    [ ] Coverage for `ExtendedValue_repr`.
    [ ] Tests for `ExtendedValue_from_hex_default`.
    [ ] Tests for `ExtendedValue_is_invalid`.
    [ ] Tests for `ExtendedValue_to_str`.
[ ] Tests for StyleValue (not fully covered).
[ ] Tests for ColorText with justification (with ColorJustify member set).
    [ ] Tests for `ColorText_to_str` with justification.
[ ] Tests for RGB (more tests, covering all functions)

Functions not mentioned in Tests (collected with `cfuncs -m colr.h ./test`):
ArgType_eq
ColorText_free_args
ColorType_eq
ColorValue_eq
ColorValue_example
_colr_free
_colr_is_last_arg
_colr_join_array_length
colr_join_arrayn
_colr_join_arrayn_size
_colr_join_size
colr_printf_handler
colr_printf_info
colr_printf_register
_colr_ptr_length
_colr_ptr_repr
_colr_ptr_to_str
colr_set_locale
colr_str_replace_ColorArg
colr_str_replace_ColorResult
colr_str_replace_ColorText
colr_str_replace_re
colr_str_replace_re_ColorArg
colr_str_replace_re_ColorResult
colr_str_replace_re_ColorText
colr_str_replace_re_match
colr_str_replace_re_match_ColorArg
colr_str_replace_re_match_ColorResult
colr_str_replace_re_match_ColorText
colr_str_replace_re_pat
colr_str_replace_re_pat_ColorArg
colr_str_replace_re_pat_ColorResult
colr_str_replace_re_pat_ColorText
ExtendedValue_eq
ExtendedValue_from_BasicValue
ExtendedValue_from_hex
ExtendedValue_from_hex_default
ExtendedValue_from_RGB
ExtendedValue_is_invalid
ExtendedValue_is_valid
ExtendedValue_to_str
_rainbow
rainbow_bg
rainbow_bg_term
rainbow_fg
rainbow_fg_term
rainbow_step
RGB_average
RGB_from_BasicValue
RGB_from_ExtendedValue
RGB_grayscale
RGB_inverted
RGB_monochrome
RGB_to_term_RGB
StyleValue_eq
StyleValue_is_invalid
StyleValue_is_valid
StyleValue_repr
StyleValue_to_str

59 functions were not mentioned in:
    ./test
