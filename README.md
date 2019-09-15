# ColrC

For full documentation see [docs/index.html](docs/index.html)

## Getting Started
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

## Files

The only two files that are needed to use ColrC are colr.h and colr.c.

Name   | Description
:----- | :---------------------------------------------
colr.h | The interface to ColrC.
colr.c | Where ColrC is implemented. This must be compiled/linked with your program.

## Example Usage

You use colr_cat(), colr_join(), and Colr(), along with fore(), back(), and style()
to build colorized strings. There are some print-related functions, for quick
building/printing of colorized strings (colr_puts() and colr_print()).

```c
#include "colr.h"

int main(int argc, char** argv) {
    // Print-related macros, using Colr() to build colorized text:
    puts("\nColrC supports ");
    char* joined = colr_join(
        ", ",
        Colr("basic", fore(WHITE)),
        Colr("extended (256)", fore(ext(155))),
        Colr("rgb", fore(rgb(155, 25, 195))),
        Colr("hex", fore(hex("#ff00bb"))),
        Colr("extended hex", fore(ext_hex("#ff00bb"))),
        Colr("color names", fore("dodgerblue"), back("aliceblue")),
        Colr("and styles.", style(BRIGHT))
    );
    printf("%s\n", joined);
    free(joined);

    colr_puts(
        "Strings and ",
        Colr("colors", fore(LIGHTBLUE)),
        " can be mixed in any order."
    );

    // Create a string, using colr_cat(), instead of colr_puts() or colr_print().
    char* mystr = colr_cat(Colr("Don't want to print this.", style(UNDERLINE)));
    printf("\nNow I do: %s\n", mystr);
    free(mystr);

    // Create a ColorText, on the heap, for use with colr_cat(), colr_print(),
    // or colr_puts().
    ColorText* ctext = NULL;
    if (argc == 1) {
        ctext = Colr("<nothing>", fore(RED));
    } else {
        ctext = Colr(argv[1], fore(GREEN));
    }
    char* userstr = colr_cat("Argument: ", ctext);
    puts(userstr);
    // colr_cat() already called ColorText_free(ctext).
    free(userstr);
}
```


### Example Files

For all examples, check the [documentation](examples.html).
Here is a table of the most common usage examples:

Name           | Example
:------------- | :-----------------
colr_cat  | [colr_cat_example.c](examples/colr_cat_example.c)
colr_join | [colr_join_example.c](examples/colr_join_example.c)
Colr      | [Colr_example.c](examples/Colr_example.c)
fore      | [fore_example.c](examples/fore_example.c)
back      | [back_example.c](examples/back_example.c)
style     | [style_example.c](examples/style_example.c)

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

## Why

ColrC is the `C` version of [Colr](https://github.com/welbornprod/colr)
(a python library) and it's less-flexible cousin
[Colr.sh](https://github.com/welbornprod/colr.sh).
The programming styles vary because `C` doesn't allow easy method chaining,
and instead leans towards nested function calls.

There are other terminal color libraries out there, but I'm not fond of the
approach that they take (wrapping file descriptors, and manually concatenating).
At least, in the libraries that I've seen so far.

## Future

In the future there may be a shared library or a python extension based on
ColrC, but for now I'm finishing out the basic features and testing.

