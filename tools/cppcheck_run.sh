#!/bin/bash

# Runs cppcheck with some defaults.
# -Christopher Welborn 07-27-2019
appname="ColrC - cppcheck runner"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
colrdir="$appdir/.."
testdir="$colrdir/test"
shopt -s nullglob

declare -a colr_files=("$colrdir"/colr.c "$colrdir"/colr.h)
declare -a colr_tool_files=("$colrdir"/colr_tool.c "$colrdir"/colr_tool.h)
declare -a test_files=("$testdir"/test_*.c "$testdir"/test_*.h)

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
        $appscript [-t] [-- ARGS...]
        $appscript [FILE...] [-- ARGS...]

    Options:
        ARGS          : Extra arguments for cppcheck.
        FILE          : Zero or more files to check.
                        If none are given, then the colr tool source is used.
        -h,--help     : Show this message.
        -t,--test     : Run on test files.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a user_files
declare -a extra_args

in_args=0
do_test=0

for arg; do
    case "$arg" in
        "--")
            in_args=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-t" | "--test")
            do_test=1
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

((${#user_files[@]})) || {
    ((${#colr_files[@]})) || fail "No colr source files found!"
    if ((do_test)); then
        ((${#test_files[@]})) || fail "No test source files found!"
        user_files=("${colr_files[@]}" "${test_files[@]}")
    else
        ((${#colr_tool_files[@]})) || fail "No colr tool source files found!"
        user_files=("${colr_files[@]}" "${colr_tool_files[@]}")
    fi
}
"${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}"

exit
