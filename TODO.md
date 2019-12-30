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
[ ] Small edge cases for TYPE_NONE, TYPE_INVALID, etc. Run: `make coverage coverageview`
