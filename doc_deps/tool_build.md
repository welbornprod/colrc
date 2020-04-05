# Tool Building {#tool_building}

\anchor tool-build
# Build

To use the ColrC tool you will have to build it first. A `makefile` is provided,
so the actual building only takes one command. Make sure you have all of the
[system dependencies](https://welbornprod.com/colrc/dependencies.html) first.

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

\anchor install-debian-package
# Install Debian Package
You can [download](https://welbornprod.com/colrc/downloads.html#debian-packages)
or [build](https://welbornprod.com/colrc/make.html#make-debian-packages) a debian package
to install the colr tool:
```bash
make distdeb
sudo dpkg -i dist/colr_latest*.deb
```

\anchor uninstall-debian-package
# Uninstall Debian Package
You can uninstall the colr package just like any other debian package:
```bash
sudo apt remove colr
```

\anchor install-manual
# Install Manual
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

\anchor uninstall-manual
# Uninstall Manual
If `colrc` was installed somewhere in `$PATH`, you can simply run the install
script with `--uninstall`, or just:
```bash
make uninstall
```
