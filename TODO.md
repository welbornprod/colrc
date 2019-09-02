# ColrC Todo List

## Features
[ ] Allow "rainbow" as an official color name for `ColorText`.
[ ] Examples for the "helper" functions, like stripping codes.

## Missing Features From Colr.py

Some features from Colr.py will never be found in ColrC. Colr depends on
method chaining to implement a lot of features. I'd like to at least be able
to match the colr-tool features, which I think is possible.

[ ] --spread, for rainbow spread (same rgb code for several chars).

## Tests
[ ] Should have almost %100 coverage.
    It will never be %100 percent though, unless I figure out a way to mock
    an `ioctl()` call for the `colr_win_size` stuff.
    [ ] Coverage for `_colr` and friends using a `ColorResult`.
    [ ] Tests for `ColorArgs_from_str`.
    [ ] Tests for `ColorType_to_str`.
    [ ] Tests for '*_example' functions.
    [ ] Tests for `rainbow*` functions.
    [ ] Tests for `BasicValue`.
        [ ] Tests for `BasicValue_is_invalid`.
        [ ] Coverage for `BasicValue_repr`.
        [ ] Tests for `BasicValue_to_str`.
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
    [ ] Tests for `TermSize_repr`.
