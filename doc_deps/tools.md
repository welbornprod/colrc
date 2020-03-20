# Tools {#tools}
# ColrC Tools

There are several scripts/tools in the `./tools` directory that aid in development.
Some of them were created specifically for `make` targets, and some are used
for inspecting the state of ColrC. All of them can be used as standalone
commands, and all of them support the `-h`/`--help` options.

- \toollink{examples/run_examples.sh, examples/run_examples.sh}
    - Run examples, and run `memcheck` on the examples.
- \toollink{test/run_tests.sh, test/run_tests.sh}
    - Run tests, `memcheck` examples and the `colrc` tool.
- \toollink{clean.sh, tools/clean.sh}
    - Implements the `clean` make targets.
- \toollink{cppcheck_errors.py, tools/cppcheck_errors.py}
    - Lists all possible `cppcheck` errors/warnings, with filtering options.
- \toollink{cppcheck_run.py, tools/cppcheck_run.py}
    - Run `cppcheck`, generate HTML reports for ColrC.
- \toollink{find_python.sh, tools/find_python.sh}
    - Locate and report a specific python executable by version.
- \toollink{gen_coverage_html.sh, tools/gen_coverage_html.sh}
    - Uses `lcov` to generate an HTML coverage report for ColrC.
- \toollink{gen_latex_pdf.sh, tools/gen_latex_pdf.sh}
    - Generates the PDF manual from Doxygen's LaTeX output.
- \toollink{get_version.sh, tools/get_version.sh}
    - Report the current ColrC version (based on the source files).
- \toollink{install.sh, tools/install.sh}
    - Installs and uninstalls the `colrc` executable.
- \toollink{is_build.sh, tools/is_build.sh}
    - Determines the current build type for `colrc` and `test_colrc`
    (`debug`, `release`, `sanitize`).
- \toollink{make_dist.sh, tools/make_dist.sh}
    - Creates a small downloadable package for users of ColrC.
- \toollink{make_help_fmter.py, tools/make_help_fmter.py}
    - Colorizes and formats output for the `make help` target.
- \toollink{refactor.sh, tools/refactor.sh}
    - Basic refactoring tool, with preview of changes to be made.
- \toollink{replacestr.py, tools/replacestr.py}
    - Replaces strings in files, with options to preview the changes.
    `refactor.sh` is implemented with this.
- \toollink{snippet.py, tools/snippet.py}
    - Compile and run arbitrary C code, ColrC source examples, ColrC snippets
    (snippets of C that use ColrC features), with options for running code through
    `memcheck`, `gdb`/`kdbg`, or user-specified tools.
- \toollink{undoxy_markdown.py, tools/undoxy_markdown.py}
    - Generates a GitHub-friendly README from `index.md` for ColrC.
- \toollink{unused.py, tools/unused.py}
    - Display unused and untested functions/macros in the ColrC source.
- \toollink{valgrind_run.sh, tools/valgrind_run.sh}
    - Runs `colrc` or the tests through `cachegrind`, `callgrind`, or `memcheck`.

If you would like to see the acceptable options or usage strings for these commands,
run `<command> -h`.
