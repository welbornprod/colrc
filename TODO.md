# ColrC Todo List

## Warnings
[ ] Look at using a printf modifier only, instead of a whole new specifier.
    It may help with warnings (%cs instead of %R?)

## Lints
[ ] unreadVariable for sprintf() in ColorArgs_list_repr().

## Features
[ ] colr_str_replace_re_all(), replace *ALL* occurrences of a regex pattern with
    a string, ColorArg, ColorResult, or ColorText.

## Tests
Should have %100 coverage.
[ ] Tests for `rainbow*` functions.
[ ] Tests for RGB (more tests, covering all functions)

Functions not mentioned in Tests:
    Collected with: cfuncs -m colr.h ./test -e '(str_replace)|(_colr)|(printf)|(free)|(join_array)|(locale)'
    * All of the exclude patterns are somehow covered under another macro/function.
    * See tests and test coverage to confirm.
_rainbow
rainbow_bg
rainbow_bg_term
rainbow_fg
rainbow_fg_term
rainbow_step

6 functions were not mentioned in:
    ./test
