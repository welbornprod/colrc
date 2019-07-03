# Documentation {#index}

# Getting Started
**ColrC** is a C library for terminal colors/escape-codes on linux.

It is designed to be easy to use. Calculations and allocations are done for you
when it comes to the escape codes needed to colorize text. You are responsible
for the text strings themselves (the words you want to colorize).

## Compiling
You must include colr.h and link colr.c in with your
program.
```c
#include "colr.h"

// ..use ColrC functions/macros/etc.
```

ColrC uses a couple glibc features, which may not be compatible with your system.
Most linux distros are compatible.

The colr.h header defines `_GNU_SOURCE` if it's not already defined (see `man feature_test_macros`).

*Be sure to include **libm** (the math library) when compiling*:
```bash
gcc -std=c11 -c myprogram.c colr.c -o myexecutable -lm
```

## Files

The only two files that are needed to use ColrC are colr.h and colr.c.

Name   | Description
:----- | :---------------------------------------------
colr.h | The interface to ColrC.
colr.c | Where ColrC is implemented. This must be compiled/linked with your program.
dbug.h | Some debugging support for the colr tool executable, only enabled when compiled with `-DDEBUG`.

## Example Usage

You use colr(), colr_join(), and Colr(), along with fore(), back(), and style()
to build colorized strings.

\include examples/Colr_example.c

If you find yourself typing the same thing over and over to join by a common
element, just use colr_join():

\include examples/colr_join_example.c

If you prefer to manually insert the escape codes in an arbitrary order,
you can use fore(), back(), and style() directly inside a colr() call:

\include examples/manual_example.c


Finally, if you are going to be making your own Colrs outside of the macros,
you're going to need to know how to `free()` them:

\include examples/Colr_manual_example.c

### Example Files

There are more examples in the documentation:

Name           | Example
:------------- | :-----------------
\ref colr      | \ref colr_example.c
\ref colr_join | \ref colr_join_example.c
\ref Colr      | \ref Colr_example.c
\ref fore      | \ref fore_example.c
\ref back      | \ref back_example.c
\ref style     | \ref style_example.c

## Why

ColrC is the `C` version of [Colr](https://github.com/welbornprod/colr)
(a python library), and it's less-flexible cousin
[Colr.sh](https://github.com/welbornprod/colr.sh).
The programming styles vary because `C` doesn't allow easy method chaining,
and instead leans towards nested function calls.

There are other terminal color libraries out there, but I'm not fond of the
approach that they take (wrapping file descriptors, and manually concatenating).
At least, in the libraries that I've seen so far.


## Future

In the future there may be a shared library or a python extension based on
ColrC, but for now I'm finishing out the basic features and testing.

## Tests

ColrC uses CMocka for testing. In future I hope to have more tests, and more
thorough tests. For now, if you want to run them you will need to install the
CMocka library (`cmocka-dev` on Debian-based distros). From there you can `cd`
into the `test` directory and make/run the tests:

```bash
cd test
make test
```

This will build all of the tests using the latest `colr.c` and run them with
`run_tests.py` (a helper that colorizes the test output).

