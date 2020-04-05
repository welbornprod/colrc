#!/bin/bash

# Creates debian package structures from the ColrC source,
# builds the packages, and copies them into <colrc root>/dist.
# This will also make a copy of the package with 'latest' in it's name.
# -Christopher Welborn 04-04-2020
appname="ColrC Deb Maker"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
colrc_dir="$(readlink -f "$appdir/..")"
get_ver_script="$colrc_dir/tools/get_version.sh"

colrc_version="$(bash "$get_ver_script")"

colr_dir="colr-$colrc_version"
libcolr_dir="libcolr-dev-$colrc_version"

declare -a colr_files=(
    "makefile"
    "colr.h"
    "colr.c"
    "colr.controls.h"
    "colr.controls.c"
    "dbug.h"
)
declare -a colr_tool_files=("${colr_files[@]}")
colr_tool_files+=(
    "colr_tool.h"
    "colr_tool.c"
)

function build_deb {
    # cd into a package directory and build it.
    # Arguments:
    #   $1   : The package directory (./colr-x.x.x, or ./libcolr-dev-x.x.x).
    #   $2.. : Extra arguments for `dpkg-buildpackage`.
    local srcdir=$1 build_status=0
    [[ -n "$srcdir" ]] || fail "No source directory given to build_deb()!"
    [[ -e "$srcdir" ]] || fail "Missing source directory: $srcdir"
    shift
    declare -a build_args=("$@")
    pushd "$srcdir" 1>/dev/null
    printf "Running: dpkg-buildpackage %s\n" "${build_args[*]}"
    dpkg-buildpackage "${build_args[@]}"
    build_status=$?
    popd 1>/dev/null
    return $build_status
}

function build_pkgs {
    # Clean existing package dirs, rebuild them, and run dpkg-buildpackage.
    local failures=0
    clean_dirs

    ((do_pkg_colr)) && {
        printf "Building colr\n"
        create_dir_colr
        build_deb "$appdir/$colr_dir" -b || let failures+=1
    }
    ((do_pkg_libcolr)) && {
        create_dir_libcolr
        printf "Building libcolr...\n"
        build_deb "$appdir/$libcolr_dir" -b || let failures+=1
    }

    ((failures)) && {
        printf "Cleaning up for failures...\n"
        clean_dirs
        return $failures
    }
    local distdir="$appdir/../dist"
    [[ -d "$distdir" ]] || {
        printf "Creating dist dir: %s\n" "$distdir"
        mkdir -p "$distdir" || fail "Unable to create dist directory!"
    }
    distdir="$(readlink -f "$distdir")"
    printf "Copying deb packages to: %s\n" "$distdir"
    local debpath debfile destpath pkgname pkgarch latestname
    while read -r debpath; do
        debfile="${debpath##*/}"
        destpath="$distdir/$debfile"
        if cp "$debpath" "$destpath"; then
            printf "Copied package: %s\n" "$destpath"
            pkgname="${debfile%%_*}"
            pkgarch="${debfile##*_}"
            latestname="${pkgname}_latest_${pkgarch}"
            cp "$destpath" "$distdir/$latestname" && {
                printf "Updated latest package: %s\n" "$latestname"
            }
            rm "$debpath" || printf "Unable to remove original package: %s\n" "$debpath"
        else
            printf "Unable to copy package file: %s\n" "$debpath"
        fi
    done < <(find "$colrc_dir/deb_pkgs" -type f -name "*.deb")
    clean_dirs
    clean_pkg_files
}


function clean_dirs {
    # Delete any leftover package directories.
    local dirpath
    local colrname="colr-[0-9].[0-9].[0-9]"
    local libcolrname="libcolr-dev-[0-9].[0-9].[0-9]"
    while read -r dirpath; do
        printf "Removing package directory: %s\n" "$dirpath"
        rm -r "$dirpath"
    done < <(find "$colrc_dir/deb_pkgs" -type d -name "$colrname" -or -name "$libcolrname" ! -name "*.in")
}

function clean_pkg_files {
    local filepath
    declare -a findargs=(
        "-name" "*.deb"
        "-or"
        "-name" "*.ddeb"
        "-or"
        "-name" "*.dsc"
        "-or"
        "-name" "*.tar.gz"
        "-or"
        "-name" "*.build"
        "-or"
        "-name" "*.buildinfo"
        "-or"
        "-name" "*.changes"
    )
    while read -r filepath; do
        printf "Removing package file: %s\n" "$filepath"
        rm "$filepath"
    done < <(find "$colrc_dir/deb_pkgs" -type f "${findargs[@]}")
}

function create_dir_colr {
    # Create the colr_<version> package directory.
    printf "Creating package directory: %s\n" "$colr_dir"
    cp -r "$appdir/colr.in" "$appdir/$colr_dir"
    refresh_colr
}

function create_dir_libcolr {
    # Create the libcolr-dev_<version> package directory.
    printf "Creating package directory: %s\n" "$libcolr_dir"
    cp -r "$appdir/libcolr-dev.in" "$appdir/$libcolr_dir"
    refresh_libcolr
}

function echo_err {
    # Echo to stderr.
    echo -e "$@" 1>&2
}

function fail {
    # Print a message to stderr and exit with an error status code.
    echo_err "$@"
    exit 1
}

function fail_usage {
    # Print a usage failure message, and exit with an error status code.
    print_usage "$@"
    exit 1
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-C | -L]
        $appscript [-c] [-p]
        $appscript -d

    Options:
        -C,--colr       : Create the colr package.
        -c,--clean      : Remove leftover package directories (if they exist).
        -d,--dirs       : Create the directories, but don't build any packages.
        -h,--help       : Show this message.
        -L,--libcolr    : Create the libcolr package.
        -p,--cleanpkgs  : Clean leftover package files (if they exist).
        -v,--version    : Show $appname version and exit.
    "
}

function refresh_colr {
    # Copy files needed to compile the latest colr tool.
    local filename filepath destpath
    for filename in "${colr_tool_files[@]}"; do
        filepath="$colrc_dir/$filename"
        destpath="$appdir/$colr_dir/$filename"
        # No need to link if we already did this.
        [[ -h "$destpath" ]] && continue
        [[ -e "$destpath" ]] && fail "Please remove existing file: $destpath"
        ln -s "$filepath" "$destpath"
    done
    # Build dependencies
    mkdir -p "$appdir/$colr_dir/tools"
    cp "$appdir/../tools/clean.sh" "$appdir/$colr_dir/tools"
}

function refresh_libcolr {
    # Copy files needed to compile the latest libcolr.
    local filename filepath
    for filename in "${colr_files[@]}"; do
        filepath="$colrc_dir/$filename"
        destpath="$appdir/$libcolr_dir/$filename"
        # No need to link if we already did this.
        [[ -h "$destpath" ]] && continue
        [[ -e "$destpath" ]] && fail "Please remove existing file: $destpath"
        ln -s "$filepath" "$destpath"
    done

    # Build dependencies
    mkdir -p "$appdir/$libcolr_dir/tools"
    cp "$appdir/../tools/clean.sh" "$appdir/$libcolr_dir/tools"

    # Copy man pages.
    mkdir -p "$appdir/$libcolr_dir/docs/man"
    cp -r "$colrc_dir/docs/man/man3" "$appdir/$libcolr_dir/docs/man/man3"

    # GZip the man pages.
    gzip "$appdir/$libcolr_dir/docs/man/man3/"*.3
}


declare -a nonflags
do_clean=0
do_clean_pkgs=0
do_dirs_only=0
do_pkg_colr=0
do_pkg_libcolr=0


for arg; do
    case "$arg" in
        "-C" | "--colr")
            do_pkg_colr=1
            ;;
        "-c" | "--clean")
            do_clean=1
            ;;
        "-d" | "--dirs")
            do_dirs_only=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-L" | "--libcolr")
            do_pkg_libcolr=1
            ;;
        "-p" | "--cleanpkgs")
            do_clean_pkgs=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            fail_usage "Unknown flag argument: $arg"
            ;;
        *)
            nonflags+=("$arg")
    esac
done

# Clean only?
((do_clean || do_clean_pkgs)) && {
    ((do_clean)) && clean_dirs
    ((do_clean_pkgs)) && clean_pkg_files
    exit
}

# Create dirs only?
((do_dirs_only)) && {
    create_dir_colr
    create_dir_libcolr
    exit
}

# Build debian packages.
((do_pkg_colr || do_pkg_libcolr)) || {
    # Package wasn't specified, do both.
    do_pkg_colr=1
    do_pkg_libcolr=1
}
build_pkgs
