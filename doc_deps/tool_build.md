# Tool Building {#tool_building}

# Build

To use the ColrC tool you will have to build it first. A `makefile` is provided,
so the actual building only takes one command. Make sure you have all of the
[system dependencies](dependencies.html) first.

Clone the repo, if you haven't already:
```bash
git clone https://github.com/welbornprod/colrc.git
```

Make sure you're in the ColrC project directory:
```bash
cd colrc
```

Finally, run the `make` target:
```bash
make release
```

The build process doesn't take very long, and when it's done there will be a
`colrc` executable in the project directory.

# Install
Installing is just copying or symlinking the executable somewhere in `$PATH`.
There is a `make` target that will let you choose an install path, and do the
rest for you:
```bash
make install

# Install as a symlink instead of a copy:
make installlink
```

By default, it will ask for confirmation before installing or overwriting
anything.

# Uninstall
If `colrc` was installed somewhere in `$PATH`, you can simply run the install
script with `--uninstall`, or just:
```bash
make uninstall
```
