# Downloads {#downloads}

# Downloadable Files

Here are a couple downloadable packages from **ColrC**.

## PDF
This documentation is available in a PDF:
- [ColrC-manual.pdf](https://welbornprod.com/colrc/ColrC-manual.pdf)

## Source Files
The **ColrC** header and source file can be downloaded if you don't want to
clone the github repo:
- [Source Package](https://welbornprod.com/colrc/dist/colrc-latest.tar.gz)

## Debian Packages
There is a basic debian package for the standalone [colrc](https://welbornprod.com/colrc/tool.html)
executable, and a shared library (`libcolor`). You can build these yourself
by cloning the repo and running `make distdeb`:
- [colrc](https://welbornprod.com/colrc/dist/colr_latest_amd64.deb) - Installs the `colrc` tool.
- [libcolr](https://welbornprod.com/colrc/dist/libcolr-dev_latest_amd64.deb) - Installs `libcolr.so`, `colr.h`, and `man` pages.

### Installing Debian Packages
Download or build the package, and then install it with `dpkg`:
```bash
# colr
sudo dpkg -i colr_latest_amd64.deb

# libcolr
sudo dpkg -i libcolr-dev_latest_amd64.deb
```

### Uninstalling Debian Packages
You can uninstall the packages using `dpkg -r`, `apt-get`, or `apt`.
This will remove all of the installed files:
```bash
# colr
sudo apt remove colr

# libcolr
sudo apt remove libcolr-dev
```

