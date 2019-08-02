# ColrC Todo List

## Features

[ ] Allow "rainbow" as an official color name for `ColorText`.
[ ] Allow user args for `freq` and `offset` in `rainbowize()`.

## Missing Features From Colr.py
[ ] --stripcodes, remove all escape codes from a string (already have `str_strip_codes()`).
[ ] --listcodes, list escape codes found in text.
[ ] --translate, show terminal, hex, and rgb values for any color given.
[ ] --names, the entire colr.name_data database of known color names.

## Tests

[ ] Tests for `str_to_wide`, `wide_to_str`.
[ ] Tests for `str_is_codes`.
[ ] Tests for `_colr()`, `_colr_join()`.
[ ] Tests for `rainbow*` functions.
[ ] Tests for ArgType (small, but still needed for %100).
[ ] Tests for ExtendedValue (not fully covered).
[ ] Tests for StyleValue (not fully covered).
[ ] Tests for `ColorText_from_values` with empty ColorArg values.
[ ] Tests for ColorText with justification (with ColorJustify member set).
    [ ] Tests for `ColorText_length` with justification.
    [ ] Tests for `ColorText_set_just`.
    [ ] Tests for `ColorText_to_str` with justification.
[ ] Tests for RGB (more tests, covering all functions)

## Optimizations

[ ] The rainbow code needs to be optimized, possibly converting to wide-char on the fly.
