# Documentation {#index}

![ColrC](https://welbornprod.com/colrc/colrc-pixels.png)

# Getting Started

**ColrC** <small>(<i><b>kuh</b>·lr·see</i>, feels like heresy)</small>
is a C library for terminal colors/escape-codes on linux.

There is also a command-line tool ([colr tool](tool.html)) based on **ColrC**.

It is designed to be flexible and easy to use. Colors can be specified using
defined names (`RED`, `BLUE`, etc.), 256-colors (`ext(36)`),
RGB colors (`rgb(0, 0, 55)`), hex colors (`hex(s)`, `hex("#ff0000")`), or known names (`"aliceblue"`).
These colors can be used with `fore()` and `back()` to set the foreground/background
colors (`fore(RED)`, `back(WHITE)`).
Styles are specified with their defined names (`style(BRIGHT)`).

Strings can be joined, replaced, colorized, and justified using a few
functions/macros. `fore()`, `back()`, and `style()` are mostly optional
and position doesn't matter.

Ownership in **ColrC** is easy to remember. Strings (`char*`) are yours,
everything else belongs to **ColrC**. If you create a **ColrC** object with one
of the `Colr*` macros to use inside of the `colr*` macros (notice the casing),
it will be released. The resulting strings that are returned from the `colr*`
macros will not be released. You must `free()` those.

If you use `colr_print` or `colr_puts` you won't have to manage the resulting
string either.


## Including
You must include colr.h and compile colr.c along with your program.
```c
#include "colr.h"

int main(void) {
    // Simple usage:
    char* s = colr("Hello from ColrC!", fore("blueviolet"), back(WHITE));
    if (!s) return EXIT_FAILURE;
    puts(s);
    // Or just:
    colr_puts(Colr("Hello again!", fore(rgb(255, 0, 0)), back("#ffff00")));

    // Fancier functions:
    char* s2 = colr_replace(
        s,
        "Hello",
        Colr_join(
            " ",
            Colr_cat(
                Colr("Good", fore(rgb(0, 0, 255)), back(RESET)),
                Colr("bye", fore(CYAN), style(BRIGHT))
            ),
            "and",
            Colr("good luck", style(UNDERLINE))
        )
    );
    free(s);
    if (!s2) return EXIT_FAILURE;
    puts(s2);
    free(s2);

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

\anchor Library
## Library

You can also create a shared library (`libcolr.so`) for your system. Clone the
repo and run the make target:
```bash
make libdebug

# Or, build it with no debug info and optimizations turned on:
make librelease
```

If you link the library (and `libm`), you will only need to include the header (`colr.h`):
```bash
gcc -std=c11 -c myprogram.c -o myexecutable -lm -lcolr
```

There are several `make` targets to help you build and install the library.
The installer is interactive, and will let you choose where to install the library based on GCC's
library search path. It will not overwrite existing files without confirmation:
```bash
# Build libcolr with optimizations and copy it to GCC's lib path (you select):
make libinstall

# Build libcolr with optimizations and symlink it to GCC's lib path:
make liblink

# Build libcolr with debug info, and install it:
make libinstalldebug

# Build libcolr with debug info, and symlink/install it:
make liblinkdebug
```
\anchor example-usage
## Example Usage

For a full listing see [the docs](globals_defs.html), but here are
the main features in ColrC:

Name          | Purpose
:------------ | :------------------------------------------------------
colr          | Generates a colorized string.
Colr          | Generates a colorized ColorText.
colr_cat      | Concatenates strings and ColrC objects into a string.
Colr_cat      | Concatenates strings and ColrC objects into a ColorResult.
colr_join     | Generates a string by joining strings/ColrC-objects by another string/ColrC-object.
Colr_join     | Generates a ColorResult by joining strings/ColrC-objects by another string/ColrC-object.

When an allocated ColorArg/ColorText/ColorResult is used inside of a Colr/colr
call it is automatically released.
Strings produced by a Colr/colr call are managed by the user (you must `free()` them).

I've included an example that showcases some of these:

\includesrc{examples/simple_example.c}

\anchor example-files
### Example Files

For all examples, check the [documentation](examples.html).
Here is a table of the most common usage examples:

Name                  | Example
:-------------------- | :-----------------------------
\ref Colr             | \ref Colr_example.c
\ref colr_cat         | \ref colr_cat_example.c
\ref colr_join        | \ref colr_join_example.c
\ref colr_replace     | \ref colr_replace_example.c
\ref colr_replace_re  | \ref colr_replace_re_example.c
\ref fore             | \ref fore_example.c
\ref back             | \ref back_example.c
\ref style            | \ref style_example.c

There are [examples](examples.html) for all of the main features in ColrC,
and [tools](tools.html) (like the snippet runner) you can play with if you clone
the repo.

\anchor why
## Why

ColrC is the `C` version of [Colr](https://github.com/welbornprod/colr)
(a python library) and it's less-flexible cousin
[Colr.sh](https://github.com/welbornprod/colr.sh).
The programming styles vary because `C` doesn't allow easy method chaining,
and instead leans towards nested function calls.

This is an attempt to create a flexible and easy version for `C`.

\anchor future
## Future

In the future there may be a shared library or a python extension based on
ColrC, but for now I'm finishing out the basic features and testing.

