# Make {#make}
# ColrC Make Targets

ColrC is built using `make`, and though there are plenty of targets in the
main directory, `./test`, and `./examples`, only a few are needed to make
confident changes to ColrC. Most test targets have a `quiet` version that
only shows failures in the terminal.

The typical workflow looks like this:
```bash
# Start fresh, if needed.
make clean

# Make sure everything compiles.
# This can be skipped if you are just writing tests.
make

# Make sure all tests pass.
make testfast

# Make sure nothing leaks.
# This can be skipped in favor of `make testeverything`, but is faster.
make testfull

# Make sure there are no leaks in ColrC or the many examples.
# This is only needed when you think you're done with your work,
# and you'd like to commit/push your changes.
make testeverything

# Rebuild the documentation if anything has changed.
make docs
```

If one of them fails, start over. If all of them pass, congratulations. You
didn't break anything.

All make targets can be listed with `make help` or `make targets`. I've listed
the main targets here.

# Build

- `make clean`
    - Remove any object files or binaries to force a fresh build.
- `make`
    - Simple running `make` in the source directory will build the `colrc`
    tool in debug mode.
- `make release`
    - Build a non-debug build for the `colrc` tool.

# Test

- `make test`
    - Build and run the tests using the address sanitizer options (slowest build time).
- `make testfast`
    - Build and run the tests in debug mode (fastest build time).
- `make testmemcheck`
    - Build and run the tests in debug mode, through `memcheck`.
- `make testfull`
    - Build and run the test in debug mode, in `memcheck` mode, and in "sanitized" mode.
- `make testeverything`
    - Like `make testfull`, but also runs `memcheck` on all source examples,
    example files, and any examples in the main README.
    It also builds the coverage report.
- `make testcoverage`
    - Build a coverage report for the tests.
- `make testcoverageview`
    - Open the coverage report in a browser.
- `make cppcheckreport`
    - Build a cppcheck report.
- `make cppcheckview`
    - Open the cppcheck report in a browser.

# Document

- `make docshtml`
    - Build the HTML documentation.
    This is faster if you're tweaking the format or looking for mistakes.
- `make docs`
    - Build all documentation (HTML, PDF, GitHub README, etc.)
- `make cleandocs`
    - Remove all generated doc files, to start fresh.

# Examples

- `make examples`
    - Build all examples in `./examples`.
    This is not required, but is useful if you've written a new example and
    you would like to make sure it compiles.
- `make cleanexamples`
    - Remove all example objects/binaries, to start fresh.

# Debian Packages

- `make distdeb`
    - Build debian binary packages for the colr tool and libcolr.
    The packages will be placed in `./dist`, and can be installed using
    `sudo dpkg -i <file>`. Packages can be uninstalled like any other package
    (`sudo apt remove colr` or `libcolr-dev`).
- `make distdebfull`
    - Build debian source & binary packages for the colr tool and libcolr.
    The packages will be placed in `./dist`, and can be used with `debmake`
    to create debian packages.
    They include only the minimum build dependencies to compile ColrC
    (colrc or libcolr) and generate a debian package.
    When running `debmake` on the `libcolr` tarball, you will need to specify
    the binary spec: `debmake -b 'libcolr:lib' libcolr-dev_<version>.tar.gz`
