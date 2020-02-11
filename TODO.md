# ColrC Todo List

## Warnings
[ ] Look at using a printf modifier only, instead of a whole new specifier.
    It may help with warnings (%cs instead of %R?)
[ ] Look at replacing `alloca` with a variable-length array in fore/back_str_static.
    They aren't used in Colr anywhere. It's an experiment with stack-based Colr
    functions.

## Lints
[ ] unreadVariable for sprintf() in ColorArgs_list_repr().
    See notes in ColorArgs_list_repr() about checking the return value.

## Features
[ ] colr_str_replace_re_all(), replace *ALL* occurrences of a regex pattern with
    a string, ColorArg, ColorResult, or ColorText.

## Tests
Should have %100 coverage.
[ ] Small edge cases for TYPE_NONE, TYPE_INVALID, etc. Run: `make coverage coverageview`
