# ColrC Todo List

## Warnings
[ ] Look at using a printf modifier only, instead of a whole new specifier.
    It may help with warnings (%cs instead of %R?)

## Features
[ ] rgb_safe() macro that will downgrade to ext() when RGB is definitely not
    supported.

## Lints
[ ] unreadVariable for sprintf() in ColorArgs_list_repr().
    See notes in ColorArgs_array_repr() about checking the return value.

## Tests
Should have %100 coverage.
[ ] Small edge cases for TYPE_NONE, TYPE_INVALID, etc. Run: `make coverage coverageview`
