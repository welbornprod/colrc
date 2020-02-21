# Dependencies {#dependencies}
# ColrC Dependencies

## System

To compile the `colrc` tool, or use the helper tools, you will need a few
system dependencies:

- `gcc` or `clang`
    - You can use `gcc` or `clang` to compile ColrC.
    - `gcc 7.4.0+` or `clang 3.9.0+` is recommended.
- `make`
    - The main build steps are implemented in make files.
    - GNU Make 4.1+ is recommended (other versions may work).
- [`libc`](https://www.gnu.org/software/libc/)
    - The ColrC tests use GNU extensions, and certain ColrC features are enabled
    when compiled with `libc`.
    - ColrC uses `libm` to implement it's "rainbow"-related functions.
    - `libc6-dev 2.27+` is recommended.
- `python3`
    - Several scripts in `./tools` use Python.
    - Python 3.6+ is recommended.
- `bash`
    - Several scripts in `./tools` use BASH-specific features.
    - BASH 4.4+ is recommended.
- `valgrind`
    - Used for it's `memcheck` tool, to test for memory leaks in ColrC code,
    examples, and snippets.
- `cppcheck`
    - Used for extra linting of the ColrC source code.
- `lcov`
    - Used to generate test coverage reports.
- `doxygen`
    - Documentation for ColrC is generated with Doxygen.
    - Doxygen 1.8+ is recommended.
- `doxygen-latex`
    - Extras to generate the PDF manual.
- `texlive-lang-cyrillic`
    - Includes fonts for the PDF manual.
- `texlive-fonts-extra`
    - Includes fonts for the PDF manual.
- `texlive-latex-base`
    - Provides the `pdflatex` command to generate the PDF manual.
- `texlive-binaries`
    - Provides the `makeindex` command to generate the PDF manual.

## Python

There are several helper tools in the ColrC repo. They are responsible for
running tests, generating documentation, running `valgrind`, and other
conveniences.
The python-based tools have their own dependencies:

- [colr](https://pypi.org/project/colr)
    - Provides terminal colors and the `colr-run` tool.
    - This was also the inspiration for ColrC.
- [docopt](https://pypi.org/project/docopt)
    - Provides argument parsing.
- [easysettings](https://pypi.org/project/easysettings)
    - Provides settings/configuration files.
- [fmtblock](https://pypi.org/project/fmtblock)
    - Provides text block formatting.
- [outputcatcher](https://pypi.org/project/outputcatcher)
    - Provides stdout/stderr blocking/catching.
- [printdebug](https://pypi.org/project/printdebug)
    - Provides debug information while running the tools.
- [pygments](https://pypi.org/project/pygments)
    - Provides syntax highlighting for code listed with the tools.

There is a `requirements.txt` in the `./tools` directory for easy installation
of these packages (`pip install -r requirements.txt`).

