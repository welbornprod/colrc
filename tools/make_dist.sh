#!/bin/bash

# Creates a .tar.gz file for users of ColrC.
# -Christopher Welborn 12-29-2019
appname="ColrC - Dist Maker"
appversion="0.0.2"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
# Files in the package will be relative to this directory.
src_dir="$(readlink -f "$appdir/..")"
get_ver_script="$appdir/get_version.sh"
[[ -e "$get_ver_script" ]] || {
    printf "Missing dependency script: %s\n" "$get_ver_script" 1>&2
    exit 1
}
if ! colrc_version="$(bash "$get_ver_script")"; then
    printf "Unable to get ColrC version!\n" 1>&2
    exit 1
fi
colrc_src_pkg_name="colrc-$colrc_version.tar.gz"
colrc_src_pkg_name_latest="colrc-latest.tar.gz"

colrc_deb_pkg_name="colr"
colrc_deb_name_latest="${colrc_deb_pkg_name}_latest"
libcolr_deb_name_latest="libcolr-dev_latest"
colrc_lib_name="libcolr"

# Defaults (shouldn't really be used. Be explicit.)
default_dir="$(readlink -f "$appdir/../dist")"
declare -a default_files=(
    "$(readlink -f "$appdir/../colr.h")"
    "$(readlink -f "$appdir/../colr.c")"
)

declare -a default_file_names
for _filepath in "${default_files[@]}"; do
    default_file_names+=("${_filepath##*/}")
done

declare -a tar_cmd=(
    "tar"
    "--create"
    "--auto-compress"
    "--file"
)

python_exe="$(bash "$appdir/find_python.sh")" || {
    printf "\nUnable to determine python executable!\n" 1>&2
    exit 1
}

function echo_err {
    # Echo to stderr.
    echo -e "$@" 1>&2
}

function ensure_dir {
    local dirpath=$1
    [[ -n "$dirpath" ]] || fail "No directory path provided to ensure_dir()!"
    [[ -d "$dirpath" ]] || {
        printf "Creating dist dir: %s\n" "$dirpath"
        mkdir -p "$dirpath" || fail "Failed to create dist dir: $dirpath"
    }
}

function ensure_files {
    local filepath
    for filepath; do
        [[ -e "$filepath" ]] || fail "Missing file: $filepath"
    done
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


function make_deb_pkg {
    local deb_name=$1
    if [[ "$deb_name" != "$colrc_deb_pkg_name" ]] && [[ "$deb_name" != "$colrc_lib_name" ]]; then
        fail "Unknown deb name: $deb_name"
    fi
    local deb_arch
    deb_arch="$(dpkg --print-architecture)" || fail "Unable to determine architecture."
    [[ -n "$deb_name" ]] || fail "No package name provided to gen_debian_struct()!"

    printf "\nCreating deb package for: %s\n" "${deb_name}_${deb_arch}"
    local deb_pkg deb_latest
    if [[ "$deb_name" == "$colrc_deb_pkg_name" ]]; then
        deb_latest="${user_dir}/${colrc_deb_name_latest}_${deb_arch}.deb"
        deb_pkg="$("$python_exe" "$appdir/make_deb.py" -q -d "$user_dir")" || fail "Failed to create colr package!"
    elif [[ "$deb_name" == "$colrc_lib_name" ]]; then
        deb_latest="${user_dir}/${libcolr_deb_name_latest}_${deb_arch}.deb"
        deb_pkg="$("$python_exe" "$appdir/make_deb.py" -q --lib -d "$user_dir")" || fail "Failed to create libcolr package!"
    else
        fail "Unknown deb name, '$deb_name'. Cancelling."
    fi

    [[ -e "$deb_pkg" ]] || fail "No debian package built!"
    printf "\nDebian package built: %s\n" "$deb_pkg"
    cp "$deb_pkg" "$deb_latest" || fail "Failed to copy latest package: $deb_latest"
    printf "Latest Debian package updated: %s\n\n" "$deb_latest"
}


function make_src_pkg {
    local pkg_path pkg_path_latest plural
    if ((${#user_files[@]} == 0)); then
        printf "Using default files: %s\n" "${default_files[*]}" 1>&2
        user_files=("${default_file_names[@]}")
    else
        printf "Using user files: %s\n" "${user_files[*]}" 1>&2
    fi
    ensure_files "${user_files[@]}"
    ensure_dir "$user_dir"
    pkg_path="$user_dir/$colrc_src_pkg_name"
    pkg_path_latest="$user_dir/$colrc_src_pkg_name_latest"

    [[ -e "$pkg_path" ]] && {
        printf "Overwriting previous package: %s\n" "$pkg_path" 1>&2
    }
    cd "$src_dir" || fail "Cannot cd to: $src_dir"
    "${tar_cmd[@]}" "$pkg_path" "${user_files[@]}" || fail "Failed to create package: $pkg_path"
    cp "$pkg_path" "$pkg_path_latest" || fail "Failed to copy package: $pkg_path_latest"
    plural="files"
    ((${#user_files[@]} == 1)) && plural="file"
    printf "\nCreated dist package (%s %s):\n    %s\n" "${#user_files[@]}" "$plural" "$pkg_path"
    printf "Latest package updated: %s\n" "$pkg_path_latest"

}

function mkdir_or_fail {
    mkdir "$@" || fail "Unable to create directory: $*"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-d dir] [FILES...]
        $appscript [-d dir] -c | -D | -L
        $appscript [-d dir] -S [FILES...]

    Options:
        FILES             : Files to include in the source package.
                            Default: ${default_file_names[*]}
        -c,--control      : Show debian control content.
        -D,--deb          : Only make the colrc debian package.
        -d dir,--dir dir  : Destination directory.
                            Default: $default_dir
        -h,--help         : Show this message.
        -L,--libdeb       : Only make the libcolr debian package.
        -S,--source       : Only make the source package.
        -v,--version      : Show $appname version and exit.
    "
}


function set_user_dir {
    if [[ -z "$user_dir" ]]; then
        printf "Using default dir.: %s\n" "$default_dir" 1>&2
        user_dir=$default_dir
    else
        printf "Using user dir.: %s\n" "$user_dir" 1>&2
    fi
}


declare -a user_files
in_dir_arg=0
user_dir=""
do_deb=1
do_deb_lib=1
do_control=0
do_src=1

for arg; do
    case "$arg" in
        "-c" | "--control")
            do_control=1
            do_deb=0
            do_src=0
            do_deb_lib=0
            ;;
        "-D" | "--deb")
            do_deb=1
            do_src=0
            do_deb_lib=0
            do_control=0
            ;;
        "-d" | "--dir")
            in_dir_arg=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-L" | "--libdeb")
            do_deb_lib=1
            do_src=0
            do_deb=0
            do_control=0
            ;;
        "-S" | "--source")
            do_src=1
            do_deb=0
            do_deb_lib=0
            do_control=0
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
                [[ -n "$user_dir" ]] && fail "Only one -d can be used."
                in_dir_arg=0
                user_dir=$arg
                continue
            }
            user_files+=("$arg")
    esac
done

if ((do_control)); then
    printf "%s\n" "$(gen_debian_control colrc)"
    # Nothing else to do.
    exit
fi

set_user_dir
if ((do_deb)); then
    make_deb_pkg colr
fi
if ((do_deb_lib)); then
    make_deb_pkg libcolr
fi
if ((do_src)); then
    make_src_pkg
fi
exit 0
