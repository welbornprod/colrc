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

Functions not mentioned in Tests (collected with `cfuncs colr.c ./test`):
    ArgType_eq
    ColorText_free_args
    ColorType_eq
    ColorValue_eq
    _colr_free
    _colr_is_last_arg
    _colr_join_array_length
    _colr_join_arrayn_size
    _colr_join_size
    colr_printf_handler
    colr_printf_info
    colr_printf_register
    _colr_ptr_length
    colr_set_locale
    ExtendedValue_eq
    ExtendedValue_from_BasicValue
    ExtendedValue_from_hex
    ExtendedValue_from_hex_default
    ExtendedValue_from_RGB
    ExtendedValue_is_invalid
    ExtendedValue_is_valid
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
