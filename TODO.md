# ColrC Todo List

## Features
[ ] Allow "rainbow" as an official color name for `ColorText`.

## Missing Features From Colr.py
[ ] --listcodes, list escape codes found in text.
[ ] --translate, show terminal, hex, and rgb values for any color given.
[ ] --spread, for rainbow spread (same rgb code for several chars).

## Tests
[ ] Tests for colr("test", NULL, NULL, ..).
    This involves using something besides NULL to mark the end of the argument
    list. See: test_colr.c ("handles NULL")
[ ] Tests for `rainbow*` functions.
[ ] Tests for ExtendedValue (not fully covered).
[ ] Tests for StyleValue (not fully covered).
[ ] Tests for `ColorText_from_values` with empty ColorArg values.
[ ] Tests for ColorText with justification (with ColorJustify member set).
    [ ] Tests for `ColorText_length` with justification.
    [ ] Tests for `ColorText_set_just`.
    [ ] Tests for `ColorText_to_str` with justification.
[ ] Tests for RGB (more tests, covering all functions)
