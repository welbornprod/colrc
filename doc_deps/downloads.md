# Downloads {#downloads}

# Downloadable Files

Here are a couple downloadable packages from **ColrC**. Version-specific files
are available for download at the [ColrC repo](https://github.com/welbornprod/colrc/tree/dev/dist).

\anchor pdf
## PDF
This documentation is available in a PDF:
- [ColrC-manual.pdf](https://welbornprod.com/colrc/ColrC-manual.pdf)

\anchor source-files
## Source Files
The **ColrC** header and source file can be downloaded if you don't want to
clone the github repo:
- [Source Package](https://welbornprod.com/colrc/dist/colrc-latest.tar.gz)

\anchor debian-packages
## Debian Packages
There is a basic debian package for the standalone [colrc](https://welbornprod.com/colrc/tool.html)
executable, and a shared library (`libcolor`). You can build these yourself
by cloning the repo and running `make distdeb`:
- [colr](https://welbornprod.com/colrc/dist/colr_latest_amd64.deb) - Installs the `colrc` tool.
- [libcolr](https://welbornprod.com/colrc/dist/libcolr-dev_latest_amd64.deb) - Installs `libcolr.so`, `colr.h`, and `man` pages.

\anchor installing-debian-packages
### Installing Debian Packages
Download or build the package, and then install it with `dpkg`:
```bash
# colr
sudo dpkg -i colr_latest_amd64.deb

# libcolr
sudo dpkg -i libcolr-dev_latest_amd64.deb
```

\anchor uninstalling-debian-packages
### Uninstalling Debian Packages
You can uninstall the packages using `dpkg -r`, `apt-get`, or `apt`.
This will remove all of the installed files:
```bash
# colr
sudo apt remove colr

# libcolr
sudo apt remove libcolr-dev
```

\anchor debian-source-packages
## Debian Source Packages
There are basic debian source packages for `colr` and `libcolr-dev`. They are
not the same as the [source package](#source-files) at the top of this page.
They include only the minimum build dependencies to compile ColrC (colrc or libcolr)
and generate a debian package. You can build these yourself by cloning the
repo and running `make distdebfull`:
- [colr](https://welbornprod.com/colrc/dist/colr_latest.tar.gz) - Source package for the `colrc` tool.
- [libcolr](https://welbornprod.com/colrc/dist/libcolr-dev_latest.tar.gz) - Source package for `libcolr.so`, `colr.h`, and `man` pages.

\anchor building-debian-source-packages
### Building Debian Source Packages
Once you've downloaded the source package for either `colr` or `libcolr-dev`,
you can use `debmake` to generate a debian package for them:
```bash
# colr
debmake -a colr_latest.tar.gz
# ...follow instructions:
cd colr-latest
dpkg-buildpackage

# libcolr: The binary spec (-b) is required.
debmake -b 'libcolr:lib' -a libcolr-dev_latest.tar.gz
# ...follow instructions:
cd libcolr-dev-latest
dpkg-buildpackage
```
