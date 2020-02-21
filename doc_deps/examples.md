# Examples {#examples_dev}
# ColrC Examples

The example programs listed here in the documentation exist to show people
how to do things in ColrC. They are meant to be brief example programs that
showcase a certain ColrC feature. They are automatically compiled and tested
for memory leaks when you run the "everything test". There is a `makefile` in
the `./examples` directory that knows how to compile all of the example
programs by name. Each one can run as a standalone program.

There is a BASH script (`./examples/run_examples.sh`) that will run these
example programs with options for filtering by name, running `memcheck` on them, or
using the binary name as an argument to another program (`gdb`/`kdbg`).

Here are a few of the most common uses for `run_examples.sh`:
```bash
# Run all examples.
./run_examples.sh

# Run all colr_replace* examples.
./run_examples.sh colr_replace

# Run examples through Valgrind's `memcheck`.
./run_examples.sh simple_example -m

# Run examples through `memcheck`, but only show errors/leaks.
./run_examples.sh simple_example -m -q

# Debug an example using KDbg.
./run_examples.sh simple_example -r kdbg

# Send arguments to KDbg for the example program to use.
# This is like calling `kdbg simple_example -a hello`, which debugs `simple_example hello`.
./run_examples.sh simple_example -r kdbg -- -a hello
```

You can also compile/run all examples from the source directory with a `make` target:
```bash
# Compile examples that have changed.
make examples

# Run all examples.
make runexamples
```

All of the main features in ColrC should have an example that showcases their
usage. If you think of any missing examples, please send an issue or pull-request.
