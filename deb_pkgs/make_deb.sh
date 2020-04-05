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
    # Only binary packages are built by default.
    declare -a build_args=("-b")
    local build_type="binary"
    declare -a find_cp_args=("-name" "*.deb")
    ((do_source)) && {
        # If the user wants to, we can build a source package also.
        # The source isn't exactly the same as cloning the repo.
        # Only the minimum build dependencies are included, like the
        # source files and the makefile.
        build_args=("-F")
        build_type="source/binary"
        find_cp_args+=("-or" "-name" "*.tar.gz")
    }
    ((do_pkg_colr)) && {
        printf "\nBuilding colr %s package...\n" "$build_type"
        create_dir_colr
        build_deb "$appdir/$colr_dir" "${build_args[@]}" || let failures+=1
    }
    ((do_pkg_libcolr)) && {
        create_dir_libcolr
        printf "\nBuilding libcolr %s package...\n" "$build_type"
        build_deb "$appdir/$libcolr_dir" "${build_args[@]}" || let failures+=1
    }

    ((failures)) && {
        printf "Cleaning up for failures...\n"
        clean_dirs
        return $failures
    }
    local debpath
    ((no_dist)) && {
        # Skip package copying.
        ((keep_dirs)) || clean_dirs
        printf "\nGenerated files:\n"
        while read -r debpath; do
            printf "    %s\n" "$debpath"
        done < <(find "$colrc_dir/deb_pkgs" -type f "${find_cp_args[@]}")
        return
    }
    # Default dist directory when not specified.
    local distdir="$appdir/../dist"
    # Use the user's dist dir if requested.
    [[ -n "$use_dist_dir" ]] && distdir="$use_dist_dir"
    # Create the directory, if needed.
    [[ -d "$distdir" ]] || {
        printf "Creating dist dir: %s\n" "$distdir"
        mkdir -p "$distdir" || fail "Unable to create dist directory!"
    }
    distdir="$(readlink -f "$distdir")"
    printf "Copying deb packages to: %s\n" "$distdir"
    local debfile destpath pkgname pkgarch latestname fileext
    while read -r debpath; do
        debfile="${debpath##*/}"
        destpath="$distdir/$debfile"
        if cp "$debpath" "$destpath"; then
            printf "Copied package: %s\n" "$destpath"
            ((no_latest)) || {
                # Update latest package file.
                pkgname="${debfile%%_*}"
                if [[ "$debpath" == *.tar.gz ]] || [[ "$debpath" == *.dsc ]]; then
                    fileext="${debfile##*$colrc_version}"
                    latestname="${pkgname}_latest${fileext}"
                else
                    pkgarch="${debfile##*_}"
                    latestname="${pkgname}_latest_${pkgarch}"
                fi
                cp "$destpath" "$distdir/$latestname" && {
                    printf "Updated latest package: %s\n" "$latestname"
                }
            }
            ((keep_files)) || {
                # Remove original (in ./deb_pkgs).
                rm "$debpath" || printf "Unable to remove original package: %s\n" "$debpath"
            }
        else
            printf "Unable to copy package file: %s\n" "$debpath"
        fi
    done < <(find "$colrc_dir/deb_pkgs" -type f "${find_cp_args[@]}")
    ((keep_dirs)) || clean_dirs
    ((keep_files)) || clean_pkg_files
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
        $appscript [-d dir] [-C | -L] [-F] [-K] [-N | -k] [-n]
        $appscript [-c] [-p]
        $appscript -D

    Options:
        -C,--colr         : Create the colr package.
        -c,--clean        : Remove leftover package directories (if they exist).
        -D,--dirs         : Create the directories, but don't build any packages.
        -d dir,--dir dir  : Destination directory for package files.
        -F,--full         : Include source packages.
        -h,--help         : Show this message.
        -K,--keepdirs     : Don't delete package directories that are created.
                            They can be deleted later with \`-c\`.
        -k,--keepfiles    : Don't delete original source/package files that are
                            created. They can be deleted later with \`-p\`.
        -L,--libcolr      : Create the libcolr package.
        -N,--nodist       : Don't update the packages in the dist dir (or -d dir).
                            This enables -k.
        -n,--nolatest     : Don't update the 'latest' packages in the
                            destination directory.
        -p,--cleanpkgs    : Clean leftover package files (if they exist).
        -v,--version      : Show $appname version and exit.

    Normal Operation:
        Create both the colr and libcolr package:
            ./$appscript
        Create both source & binary packages.
            ./$appscript -F
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
        cp "$filepath" "$destpath"
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
        cp "$filepath" "$destpath"
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
do_source=0
keep_dirs=0
keep_files=0
no_dist=0
no_latest=0
in_dir_arg=0
# If set when parsing args, final packages are placed here.
use_dist_dir=""

for arg; do
    case "$arg" in
        "-C" | "--colr")
            do_pkg_colr=1
            ;;
        "-c" | "--clean")
            do_clean=1
            ;;
        "-D" | "--dirs")
            do_dirs_only=1
            ;;
        "-d" | "--dir")
            in_dir_arg=1
            ;;
        "-F" | "--full")
            do_source=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-K" | "--keepdirs")
            keep_dirs=1
            ;;
        "-k" | "--keepfiles")
            keep_files=1
            ;;
        "-L" | "--libcolr")
            do_pkg_libcolr=1
            ;;
        "-N" | "--nodist")
            no_dist=1
            ;;
        "-n" | "--nolatest")
            no_latest=1
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
            ((in_dir_arg)) && {
                use_dist_dir="$arg"
                continue
            }
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
