#!/bin/bash

# Runs cppcheck with some defaults.
# -Christopher Welborn 07-27-2019
appname="ColrC - cppcheck runner"
appversion="0.1.0"
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
    "--error-exitcode=1"
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
        $appscript [-f pat] [-e pat] [-t] [-- ARGS...]
        $appscript [-f pat] [-e pat] [FILE...] [-- ARGS...]
        $appscript [-E] [-t] [-- ARGS...]
        $appscript [-E] [FILE...] [-- ARGS...]

    Options:
        ARGS                  : Extra arguments for cppcheck.
        FILE                  : Zero or more files to check.
                                If none are given, the colr tool source is used.
        -E,--errors           : Add (error)|(warn) to the --filter pattern.
        -e pat,--exclude pat  : Exclude lines that match this pattern.
                                These work in the order they are given.
        -f pat,--filter pat   : Only show lines that match this pattern.
                                These work in the order they are given.
        -h,--help             : Show this message.
        -t,--test             : Run on test files.
        -v,--version          : Show $appname version and exit.
    "
}


declare -a user_files
declare -a extra_args

in_args=0
in_filter_arg=0
in_exclude_arg=0
do_test=0
exclude_pat=""
filter_pat=""
do_errors=0
filter_first=1

for arg; do
    case "$arg" in
        "--")
            in_args=1
            ;;
        "-E" | "--errors")
            do_errors=1
            ;;
        "-e" | "--exclude")
            in_exclude_arg=1
            in_filter_arg=0
            [[ -z "$filter_pat" ]] && filter_first=0
            ;;
        "-f" | "--filter")
            in_filter_arg=1
            in_exclude_arg=0
            [[ -z "$exclude_pat" ]] && filter_first=1
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
            elif ((in_exclude_arg)); then
                exclude_pat=$arg
                in_exclude_arg=0
            elif ((in_filter_arg)); then
                filter_pat=$arg
                in_filter_arg=0
            else
                fail_usage "Unknown flag argument: $arg"
            fi
            ;;
        *)
            if ((in_args)); then
                extra_args+=("$arg")
            elif ((in_exclude_arg)); then
                exclude_pat=$arg
                in_exclude_arg=0
            elif ((in_filter_arg)); then
                filter_pat=$arg
                in_filter_arg=0
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
if ((do_errors)); then
    if [[ -n "$filter_pat" ]]; then
        if [[ "$filter_pat" == \(*\) ]]; then
            filter_pat="$filter_pat|(error)|(warn)"
        else
            filter_pat="($filter_pat)|(error)|(warn)"
        fi
    else
        filter_pat="(error)|(warn)"
    fi
fi
if [[ -n "$exclude_pat" ]] && [[ -n "$filter_pat" ]]; then
    if ((filter_first)); then
        # shellcheck disable=SC1001
        # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
        "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E "$filter_pat" | \grep -E -v "$exclude_pat"
    else
        # shellcheck disable=SC1001
        # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
        "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E -v "$exclude_pat" | \grep -E "$filter_pat"
    fi
elif [[ -n "$exclude_pat" ]]; then
    # shellcheck disable=SC1001
    # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E -v "$exclude_pat"
elif [[ -n "$filter_pat" ]]; then
    # shellcheck disable=SC1001
    # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E "$filter_pat"
else
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1
fi
exit
