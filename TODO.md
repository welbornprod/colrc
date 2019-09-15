# ColrC Todo List

## Features
[ ] colr_str_replace_re(), replace a regex pattern with a string, ColorArg,
    ColorResult, or ColorText.

## Missing Features From Colr.py
Some features from Colr.py will never be found in ColrC. Colr depends on
method chaining to implement a lot of features. I'd like to at least be able
to match the colr-tool features, which I think is possible.

[ ] --spread, for rainbow spread (same rgb code for several chars).

## Tests
[ ] Should have %100 coverage.
    [ ] Tests for '*_example' functions.
    [ ] Tests for `rainbow*` functions.
    [ ] Tests for ExtendedValue (not fully covered).
        [ ] Coverage for `ExtendedValue_repr`.
        [ ] Tests for `ExtendedValue_from_hex_default`.
        [ ] Tests for `ExtendedValue_is_invalid`.
        [ ] Tests for `ExtendedValue_to_str`.
    [ ] Tests for StyleValue (not fully covered).
    [ ] Tests for ColorText with justification (with ColorJustify member set).
        [ ] Tests for `ColorText_length` with justification.
        [ ] Tests for `ColorText_set_just`.
        [ ] Tests for `ColorText_to_str` with justification.
    [ ] Tests for RGB (more tests, covering all functions)
        [ ] Tests for `RGB_from_hex_default`.
        [ ] Tests for `RGB_to_hex`.
        [ ] Tests for `RGB_to_str`.
        [ ] Tests for `RGB_repr`.
