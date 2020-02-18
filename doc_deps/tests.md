# Tests {#tests}
# ColrC Testing

ColrC uses [snow](https://github.com/mortie/snow) for testing. There are
several test targets in the `makefile` that do diferent things. Some of them
are for quick sanity-checking, some use compiler protections, and some use
Valgrind. There is also a test runner (`run_tests.sh`) that provides an easy
way to run tests through a wrapper program like `valgrind` or `kdbg`/`gdb`.

## Basic Test:
If you want to run them you will have to download/clone the source and
build/run them:
```bash
# The default `test` target uses `-fsanitize` options, which can be slow:
make test
```

This will build all of the tests using the latest `colr.c` and run them.

## Memcheck Test:
You can also run the tests through `valgrind` with the `testmemcheck` target:
```bash
# Removes the `-fsanitize` options, to let `valgrind` do it's thing:
make testmemcheck
```

## Quick Testing

During development, I usually use the `testfast` target for small changes,
followed by a `testfull` to use the address sanitizer and other protection
features.
```bash
make testfast

# After I've sorted out the "easy" failures:
make testfull

# And finally, before pushing changes, the "everything test".
# This is important because it ensures that all examples will compile cleanly
# and there are no leaks:
make testeverything
```

## Test Everything
The 'everything test' builds the colr tool and unit tests, both debug and
release mode (some bugs only show up in release mode), and runs them through
`valgrind` and `-fsanitize` (`libasan`).

The examples are built and ran through `valgrind`, including the examples found
in the source code (see `snippet.py --examples`). This ensures that all
example code is correct/runnable.

The coverage target is built (with the html report).

Finally, the binaries may be rebuilt if they are in a different state than
when the process started (switch back to debug build for development).

If any of those things fail, the process is stopped and there
is probably a bug worth fixing. Errors are always reported, but the
noise from all of those steps can be silenced with `--quiet`.

Each of these steps has found one or more bugs in the code or documentation
while developing ColrC. I don't mind running this before pushing my changes.

If you'd like to run every possible compile target, with tests and memcheck,
including the example code and source-file examples (the 'everything test'):
```bash
make testeverything
```

## Test Tool

The `./test/run_tests.sh` script can run the `snow`-based tests, run `memcheck`
on the examples, and run the `colrc` tool through `memcheck`. The "everything test"
is implemented with this tool. Run `./test/run_test.sh -h` to see options for it.
