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
pkg_name="colrc-$colrc_version.tar.gz"

# Defaults (shouldn't really be used. Be explicit.)
default_dir="$(readlink -f ../dist)"
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

downloads_page_in="$src_dir/doc_deps/downloads.md.in"
downloads_page_out="$src_dir/doc_deps/downloads.md"

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

function fix_downloads_page {
    local latest_pkg=$1
    [[ -n "$latest_pkg" ]] || fail "No package path provided to \`fix_downloads_page\`."
    latest_pkg="${latest_pkg##*/}"
    sed "s/{latest_package}/$latest_pkg/" "$downloads_page_in" > "$downloads_page_out" || {
        fail "Failed to fix downloads page: $downloads_page_in"
    }
    printf "\nFixed downloads page: %s\n" "$downloads_page_out"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-P | -p] [-d dir] [FILES...]

    Options:
        FILES             : Files to include in the package.
                            Default: ${default_file_names[*]}
        -d dir,--dir dir  : Destination directory.
                            Default: $default_dir
        -h,--help         : Show this message.
        -P,--nodlpage     : Don't fix the downloads page.
        -p,--dlpage       : Just fix the downloads page.
        -v,--version      : Show $appname version and exit.
    "
}


declare -a user_files
in_dir_arg=0
user_dir=""
do_dl_page_only=0
no_dl_page=0

for arg; do
    case "$arg" in
        "-d" | "--dir")
            in_dir_arg=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-P" | "--nodlpage")
            do_dl_page_only=0
            no_dl_page=1
            ;;
        "-p" | "--dlpage")
            do_dl_page_only=1
            no_dl_page=0
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

if [[ -z "$user_dir" ]]; then
    printf "Using default dir.: %s\n" "$default_dir" 1>&2
    user_dir=$default_dir
else
    printf "Using user dir.: %s\n" "$user_dir" 1>&2
fi
if ((${#user_files[@]} == 0)); then
    printf "Using default files: %s\n" "${default_files[*]}" 1>&2
    user_files=("${default_file_names[@]}")
else
    printf "Using user files: %s\n" "${user_files[*]}" 1>&2
fi
ensure_files "${user_files[@]}"
ensure_dir "$user_dir"
pkg_path="$user_dir/$pkg_name"
if ((!do_dl_page_only)); then
    [[ -e "$pkg_path" ]] && {
        printf "Overwriting previous package: %s\n" "$pkg_path" 1>&2
    }
    cd "$src_dir" || fail "Cannot cd to: $src_dir"
    "${tar_cmd[@]}" "$pkg_path" "${user_files[@]}" || fail "Failed to create package: $pkg_path"
    plural="files"
    ((${#user_files[@]} == 1)) && plural="file"
    printf "\nCreated dist package (%s %s):\n    %s\n" "${#user_files[@]}" "$plural" "$pkg_path"
fi
((no_dl_page)) || fix_downloads_page "$pkg_path"
exit 0
