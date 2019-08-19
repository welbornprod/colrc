# ColrC Todo List

## Features
[ ] Allow "rainbow" as an official color name for `ColorText`.

## Missing Features From Colr.py

Some features from Colr.py will never be found in ColrC. Colr depends on
method chaining to implement a lot of features. I'd like to at least be able
to match the colr-tool features, which I think is possible.

[ ] --translate, show terminal, hex, and rgb values for any color given.
        Most of the translation functions are done, as far as converting from
        one color value to another. I need the front-end, displaying the
        transformations. Not sure what to do about BasicValues yet.
[ ] --spread, for rainbow spread (same rgb code for several chars).
[ ] --auto-disable, check for `isatty(fileno(stdout))`, and disable colors.
        The disabling part is weird, because I'm trying to stay away from global
        variables in ColrC. However, colr-tool can use a global variable, and may
        not even need one to implement this. It just turns itself into a basic
        `cat`/`echo` program (`cat` without a text arg, `echo` with the text arg).

## Tests
[ ] Tests for `colr_str_code_cnt`, `colr_str_code_len`, and `colr_str_get_codes`.
[ ] Tests for `rainbow*` functions.
[ ] Tests for ExtendedValue (not fully covered).
[ ] Tests for StyleValue (not fully covered).
[ ] Tests for `ColorText_from_values` with empty ColorArg values.
[ ] Tests for ColorText with justification (with ColorJustify member set).
    [ ] Tests for `ColorText_length` with justification.
    [ ] Tests for `ColorText_set_just`.
    [ ] Tests for `ColorText_to_str` with justification.
[ ] Tests for RGB (more tests, covering all functions)
