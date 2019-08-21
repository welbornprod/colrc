# Documentation {#index}

# Getting Started

**ColrC** is a C library for terminal colors/escape-codes on linux.

It is designed to be easy to use. Calculations and allocations are done for you
when it comes to the escape codes needed to colorize text. You are responsible
for the text strings themselves (the words you want to colorize).

## Including
You must include colr.h and compile colr.c along with your
program.
```c
#include "colr.h"

int main(void) {
    char* s = Colr_str("Hello from ColrC!", fore("blueviolet"), back(WHITE));
    if (!s) return EXIT_FAILURE;

    printf("%s\n", s);
    free(s);
    return EXIT_SUCCESS;
}
```

There are plenty of examples in the [documentation](examples.html), and
[on this page](#example-usage).

## Compiling

ColrC uses a couple glibc features, which may not be compatible with your system.
Most linux distros are compatible.

The colr.h header defines `_GNU_SOURCE` if it's not already defined (see `man feature_test_macros`).

*Be sure to include **libm** (the math library) when compiling*:
```bash
gcc -std=c11 -c myprogram.c colr.c -o myexecutable -lm
```

\anchor files
## Files

The only two files that are needed to use ColrC are colr.h and colr.c.

Name   | Description
:----- | :---------------------------------------------
colr.h | The interface to ColrC.
colr.c | Where ColrC is implemented. This must be compiled/linked with your program.

\anchor example-usage
## Example Usage

You use colr(), colr_join(), and Colr(), along with fore(), back(), and style()
to build colorized strings. There are some print-related functions, for quick
building/printing of colorized strings (colr_puts() and colr_print()).

\includesrc{examples/simple_example.c}

\anchor example-files
### Example Files

For all examples, check the [documentation](examples.html).
Here is a table of the most common usage examples:

Name           | Example
:------------- | :-----------------
\ref colr      | \ref colr_example.c
\ref colr_join | \ref colr_join_example.c
\ref Colr      | \ref Colr_example.c
\ref fore      | \ref fore_example.c
\ref back      | \ref back_example.c
\ref style     | \ref style_example.c

All of these examples can be built with the `examples` target:
```bash
make examples
```

You can then run the executables in `./examples` manually, with the make
target (`make runexamples`), or with the example runner:
```bash
./examples/run_example.sh [NAME_PATTERN...]
```

There is also a "snippet runner" that can build and run
arbitrary C code snippets, but is useful for building and running all example
code snippets found in the ColrC source code itself:
```bash
./tools/snippet.py --examples
```

To see the source-based examples in the terminal you can run:
```bash
./tools/snippet.py --listexamples
```

\anchor why
## Why

ColrC is the `C` version of [Colr](https://github.com/welbornprod/colr)
(a python library) and it's less-flexible cousin
[Colr.sh](https://github.com/welbornprod/colr.sh).
The programming styles vary because `C` doesn't allow easy method chaining,
and instead leans towards nested function calls.

There are other terminal color libraries out there, but I'm not fond of the
approach that they take (wrapping file descriptors, and manually concatenating).
At least, in the libraries that I've seen so far.

\anchor future
## Future

In the future there may be a shared library or a python extension based on
ColrC, but for now I'm finishing out the basic features and testing.

