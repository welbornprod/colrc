# ColrC

For full documentation see [docs/index.html](docs/index.html)

## Getting Started
**ColrC** is a C library for terminal colors/escape-codes on linux.

It is designed to be easy to use. Calculations and allocations are done for you
when it comes to the escape codes needed to colorize text. You are responsible
for the text strings themselves (the words you want to colorize).

## Including
You must include colr.h and link colr.c in with your
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
dbug.h | Some debugging support for the colr tool executable, only enabled when compiled with `-DDEBUG`.

## Example Usage

You use colr(), colr_join(), and Colr(), along with fore(), back(), and style()
to build colorized strings.

```c
#include "colr.h"

int main(void) {
    /*
        Colr() is for styling one piece of text.
        When combined with the colr() macro it allows you to seperate colors/styles.
    */
    char* colorized = colr(
        Colr("America ", fore(RED)),
        Colr("the ", fore(WHITE)),
        Colr("beautiful", fore(BLUE)),
        ".\n"
    );

    /*
        All of the Colr, fore, back, and style resources were free'd by `colr`.
        You are responsible for the text and the resulting colorized string.
    */
    if (!colorized) return 1;
    printf("%s", colorized);
    free(colorized);

    /*
        There are three justification macros that make it easy to create
        ColorText's with center, left, or right-justified text.
    */
    char* just = colr(
        Colr_center("This is centered.", 80, fore("lightblue")),
        "\n",
        Colr_ljust("This is on the left.", 38, fore(ext_hex("ff2525"))),
        "----",
        Colr_rjust("This is on the right.", 38, fore(ext_rgb(255, 53, 125)))
    );
    if (!colorized) return 1;
    printf("%s\n", just);
    free(just);

    /*
        If you don't need several Colr() calls, there is a shortcut for safely
        creating colorized text using Colr_str().
    */
    char* fast = Colr_str(
        "Hello from ColrC.",
        fore("#2500FF"),
        back(ext_hex("#353535")),
        style(UNDERLINE)
    );
    if (!fast) return 1;
    printf("%s\n", fast);
    free(fast);
}
```


If you find yourself typing the same thing over and over to join by a common
element, just use colr_join():

```c
#include "colr.h"

int main(void) {
    /*
        You can join things by a plain string or a colorized string.

        For the pieces, the order/number of arguments does not matter.
        colr_join() accepts ColorTexts, ColorArgs, and strings (char*).
    */
    char* colorized = colr_join(
        "\n",
        "This is a plain line.",
        Colr("This one is some kind of purple.", fore(rgb(125, 0, 155))),
        Colr("This one is bright.", style(BRIGHT)),
        "Another plain one, why not?"
    );
    if (!colorized) return 1;
    // Prints each colorized piece of text on it's own line:
    printf("%s\n", colorized);
    free(colorized);

    /*
        The joiner can be a ColorText, string, or ColorArg (though ColorArgs
        would be kinda useless).
    */
    char* final = colr_join(
        Colr(" <--> ", fore(ext_hex("#353535")), style(UNDERLINE)),
        "This",
        Colr(" that ", fore(RED)),
        "the other."
    );
    if (!final) return 1;
    // Prints each piece, joined by a colorized " <--> ".
    printf("%s\n", final);
    free(final);

    /*
        Arrays of ColorText, ColorArgs, or strings can be used with
        colr_join_array().
    */
    char* joiner = " [and] ";
    ColorText* words[] = {
        Colr("this", fore(RED)),
        Colr("that", fore(hex("ff3599"))),
        Colr("the other", fore(BLUE), style(UNDERLINE)),
        // The last member must be NULL.
        NULL
    };
    char* s = colr_join_array(joiner, words);
    if (!s) {
        // Couldn't allocate memory for the final string.
        for (size_t i = 0; words[i]; i++) ColorText_free(words[i]);
        return 1;
    }
    printf("%s\n", s);
    free(s);

    // Don't forget to free your ColorTexts/ColorArgs.
    for (size_t i = 0; words[i]; i++) ColorText_free(words[i]);
}
```


If you prefer to manually insert the escape codes in an arbitrary order,
you can use fore(), back(), and style() directly inside a colr() call:

```c
#include "colr.h"

int main(void) {
    // Basic colors:
    char* s = colr(
        fore(RED),
        "This is a test ",
        back(BLUE),
        style("bright"),
        "and only a test.",
        NC,
        "\nHad to reset the colors to get plain again.\n",
        fore(rgb(255, 0, 155)),
        back(ext(255)),
        style("underline"),
        // Careful placement of the NC (CODE_RESET_ALL) before the newline:
        // You said you wanted to do this manually...
        // Colr() takes care of stuff like this fore you.
        "This is the last line.", NC, "\n"
    );
    // CODE_RESET_ALL was appended to the result (even without the NC),
    // so later calls won't be affected by previous styles.
    printf("%s\n", s);

    free(s);
}
```



Finally, if you are going to be making your own Colrs outside of the macros,
you're going to need to know how to `free()` them:

```c
#include "colr.h"

int main(void) {
    /*
        Colr is shorthand for building a ColorText struct.
        If you use them outside of the colr macros, you must free them using
        ColorText_free().
        Again, you must always free the plain text if you allocated for it.
    */
    ColorText* ctext = Colr("I didn't allocate this.", fore(BLUE));
    /*
        If you use the *to_str functions directly,
        you must free the resulting string.
    */
    char* ctext_str = ColorText_to_str(*ctext);
    printf("But I allocated the resulting string: %s\n", ctext_str);
    // No leaks, free the ColorText's resources.
    ColorText_free(ctext);
    // And free the string you created from it.
    free(ctext_str);

    // Another way to do that would be:
    ColorText* piece = Colr("Just another string", back(ext(255)), fore(ext(0)));
    char *text_str = colr(piece);
    printf("%s\n", text_str);
    // The ColorText `piece` is gone. `colr()` called `free()` on it.
    // Still, we need to free the result.
    free(text_str);
}
```


### Example Files

For all examples, check the [documentation](examples.html).
Here is a table of the most common usage examples:

Name           | Example
:------------- | :-----------------
colr      | [colr_example.c](examples/colr_example.c)
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

