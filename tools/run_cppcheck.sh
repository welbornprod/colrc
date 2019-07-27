#!/bin/bash

# Runs cppcheck with some defaults.
# -Christopher Welborn 07-27-2019
appname="ColrC - cppcheck runner"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
colrdir="$appdir/.."
declare -a colr_files=(
    "$colrdir/colr.c"
    "$colrdir/colr.h"
    "$colrdir/colr_tool.c"
    "$colrdir/colr_tool.h"
)
declare -a cppcheck_cmd=(
    "cppcheck"
    "--std=c11"
    "--enable=all"
    "--force"
    "--inline-suppr"
    # These are not needed at all:
    # "-I" "/usr/include"
    # "-I" "/usr/lib/gcc/x86_64-linux-gnu/7/include"
)

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
        $appscript [FILE...] [-- ARGS...]

    Options:
        ARGS          : Extra arguments for cppcheck.
        FILE          : Zero or more files to check.
                        If none are given, then the colr tool source is used.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a user_files
declare -a extra_args

in_args=0

for arg; do
    case "$arg" in
        "--")
            in_args=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            if ((in_args)); then
                extra_args+=("$arg")
            else
                fail_usage "Unknown flag argument: $arg"
            fi
            ;;
        *)
            if ((in_args)); then
                extra_args+=("$arg")
            else
                user_files+=("$arg")
            fi
    esac
done

((${#user_files[@]})) || user_files=("${colr_files[@]}")

"${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}"

exit
