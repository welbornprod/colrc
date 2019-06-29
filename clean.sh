#!/bin/bash

# ...Removes binary and object files, preparing for a fresh compile.
# -Christopher Welborn 06-29-2019
appname="clean"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"


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
        $appscript BINARY

    Options:
        BINARY        : Optional executable to delete.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a objfiles
binary=""

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            fail_usage "Unknown flag argument: $arg"
            ;;
        *)
            if [[ -z "$binary" ]]; then
                binary="$arg"
            else
                objfiles+=("$arg")
            fi
    esac
done

[[ -n "$binary" ]] && {
    if [[ -e "$binary" ]]; then
        rm "$binary" || echo_err "Cannot remove binary: $binary"
        printf "        Cleaned: %s\n" "$binary"
    else
        printf "Already cleaned: %s\n" "$binary"
    fi
}
if ((${#objfiles} == 0)); then
    # Default, delete all object files in CWD.
    while read -r objfile; do
        objfiles+=("$objfile")
    done < <(find . -maxdepth 1 -type f -name "*.o")
fi
((${#objfiles})) || {
    objfiles=($(find . -maxdepth 1 -type f -name "*.c"))
    for objfile in "${objfiles[@]}"; do
        objfile="${objfile%.*}.o"
        objfile="${objfile##*/}"
        echo_err "Already cleaned: $objfile"
    done
    exit 0
}

for objfile in "${objfiles[@]}"; do
    rm "$objfile" || {
        echo_err "Cannot remove object: $objfile"
        continue
    }
    printf "        Cleaned: %s\n" "$objfile"
done

exit
