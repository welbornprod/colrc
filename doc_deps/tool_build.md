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
ColrC doesn't have an install script at the moment. There's not much to install
though. It's just one executable file.

You can symlink or the `colrc` executable to a directory that's in your `$PATH`:
```bash
ln -s "$PWD"/colrc ~/.local/bin/colrc
```

Or, since `colrc` doesn't have any real dependencies itself, you could just
copy it to `/usr/bin`:
```bash
sudo cp colrc /usr/bin/colrc
```
