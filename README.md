# ColrC

![CI](https://github.com/welbornprod/colrc/workflows/CI/badge.svg?event=push)

For full documentation see [welbornprod.com/colrc](https://welbornprod.com/colrc/index.html)

## Getting Started
**ColrC** <small>(<i><b>kuh</b>·lr·see</i>, feels like heresy)</small>
is a C library for terminal colors/escape-codes on linux.

There is also a command-line tool ([colr tool](https://welbornprod.com/colrc/tool.html)) based on **ColrC**.

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

There are plenty of examples in the [documentation](https://welbornprod.com/colrc/examples.html), and
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
[colr.h](https://welbornprod.com/colrc/colr_8h.html) | The interface to ColrC.
[colr.c](https://welbornprod.com/colrc/colr_8c.html) | Where ColrC is implemented. This must be compiled/linked with your program.

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
## Example Usage

For a full listing see [the docs](https://welbornprod.com/colrc/globals_defs.html), but here are
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

```c
#include "colr.h"

int main(int argc, char** argv) {
    // Print-related macros, using Colr() to build colorized text:
    puts("\nColrC supports ");
    colr_puts(Colr_join(
        ", ",
        Colr("basic", fore(WHITE)),
        Colr("extended (256)", fore(ext(155))),
        Colr("rgb", fore(rgb(155, 25, 195))),
        Colr("hex", fore(hex("#ff00bb"))),
        Colr("extended hex", fore(ext_hex("#ff00bb"))),
        Colr("color names", fore("dodgerblue"), back("aliceblue")),
        Colr("and styles.", style(BRIGHT))
    ));

    colr_puts(
        "Strings and ",
        Colr("colors", fore(LIGHTBLUE)),
        " can be mixed in any order."
    );

    // Create a string, using colr(), instead of colr_puts() or colr_print().
    char* mystr = colr("Don't want to print this.", style(UNDERLINE));
    printf("\nNow I do: %s\n", mystr);
    free(mystr);

    // Concatenate existing strings with ColrC objects.
    // Remember that the colr macros will free ColrC objects, not strings.
    // So I'm going to use the Colr* macros inside of this call (not colr*).
    char* catted = colr_cat(
        "Exhibit: ",
        Colr("b", fore(BLUE)),
        "\nThe ColorText/Colr was released."
    );
    puts(catted);
    free(catted);

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

    // Create a joined string (a "[warning]" label).
    char* warning_label = colr_join(Colr("warning", fore(YELLOW)), "[", "]");
    // Simulate multiple uses of the string.
    for (int i = 1; i < 4; i++) printf("%s This is #%d\n", warning_label, i);
    // Okay, now we're done with the colorized string.
    free(warning_label);

    // Colorize an existing string by replacing a word.
    char* logtext = "[warning] This is an awesome warning.";
    char* colorized = colr_replace(
        logtext,
        "warning",
        Colr("warning", fore(YELLOW))
    );
    // Failed to allocate for new string?
    if (!colorized) return EXIT_FAILURE;
    puts(colorized);
    // You have to free the resulting string.
    free(colorized);

    // Or colorize an existing string by replacing a regex pattern.
    colorized = colr_replace_re(
        logtext,
        "\\[\\w+\\]",
        Colr_join(
            Colr("ok", style(BRIGHT)),
            "(",
            ")"
        ),
        REG_EXTENDED
    );
    if (!colorized) return EXIT_FAILURE;
    puts(colorized);
    free(colorized);

    // Or maybe you want to replace ALL of the occurrences?
    char* logtext2 = "[warning] This is an awesome warning.";
    // There is also a colr_replace_re_all() if you'd rather use a regex pattern.
    char* colorizedall = colr_replace_all(
        logtext2,
        "warning",
        Colr("WARNING", fore(YELLOW))
    );
    // Failed to allocate for new string?
    if (!colorizedall) return EXIT_FAILURE;
    puts(colorizedall);
    // You have to free the resulting string.
    free(colorizedall);

}
```


### Example Files

For all examples, check the [documentation](https://welbornprod.com/colrc/examples.html).
Here is a table of the most common usage examples:

Name                  | Example
:-------------------- | :-----------------------------
Colr             | [Colr_example.c](examples/Colr_example.c)
colr_cat         | [colr_cat_example.c](examples/colr_cat_example.c)
colr_join        | [colr_join_example.c](examples/colr_join_example.c)
colr_replace     | [colr_replace_example.c](examples/colr_replace_example.c)
colr_replace_re  | [colr_replace_re_example.c](examples/colr_replace_re_example.c)
fore             | [fore_example.c](examples/fore_example.c)
back             | [back_example.c](examples/back_example.c)
style            | [style_example.c](examples/style_example.c)

There are [examples](https://welbornprod.com/colrc/examples.html) for all of the main features in ColrC,
and [tools](https://welbornprod.com/colrc/tools.html) (like the snippet runner) you can play with if you clone
the repo.

## Why

ColrC is the `C` version of [Colr](https://github.com/welbornprod/colr)
(a python library) and it's less-flexible cousin
[Colr.sh](https://github.com/welbornprod/colr.sh).
The programming styles vary because `C` doesn't allow easy method chaining,
and instead leans towards nested function calls.

This is an attempt to create a flexible and easy version for `C`.

## Future

In the future there may be a shared library or a python extension based on
ColrC, but for now I'm finishing out the basic features and testing.

