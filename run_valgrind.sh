#!/bin/bash

# Run valgrind cachegrind, callgrind, or memcheck on this project.
# -Christopher Welborn 06-09-2019
appname="Valgrind Runner"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
# This should be the only variable that needs to change for other projects.
binaryname="colr"
binary="${appdir}/${binaryname}"

default_tool="memcheck"
cachegrind_file="${binary}.cachegrind"
callgrind_file="${binary}.callgrind"

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
        $appscript [-a] [ARG...] [TOOL] -- [EXE_ARGS...]

    Options:
        ARG           : One or more extra arguments for valgrind.
        EXE_ARGS      : Arguments for $binaryname while running the test.
        TOOL          : Tool to use.
                        Default: $default_tool
        -a,--all      : Shortcut to --show-leak-kinds=all flag.
                        This implies TOOL=memcheck.
        -h,--help     : Show this message.
        -V,--version  : Show $appname version and exit.
    "
}

declare -a nonflags
declare -a flagargs
declare -a exeargs
in_exe_args=0

for arg; do
    case "$arg" in
        "--")
            in_exe_args=1
            ;;
        "-a" | "--all")
            nonflags[0]="memcheck"
            flagargs+=("--show-leak-kinds=all")
            ;;
        "-h" | "--help")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            print_usage ""
            exit 0
            ;;
        "-V" | "--version")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            flagargs+=("$arg")
            ;;
        *)
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            nonflags+=("$arg")
            ;;
    esac
done
[[ -e "$binary" ]] || fail "The binary hasn't been built yet: $binary"

toolname="${nonflags[0]}"
nonflags=("${nonflags[@]:1}")

[[ -z "$toolname" ]] && toolname="$default_tool"
toolname="${toolname,,}"
case "$toolname" in
    "cachegrind" | "cache")
        toolname="cachegrind"
        nonflags+=("--cachegrind-out-file=$cachegrind_file")
        ;;
    "callgrind" | "call")
        toolname="callgrind"
        nonflags+=("--callgrind-out-file=$callgrind_file")
        ;;
    "memcheck" | "mem")
        toolname="memcheck"
        [[ "${nonflags[*]}" =~ leak-check ]] || nonflags+=("--leak-check=full")
        ;;
esac

declare -a cmd=("valgrind")
cmd+=("${flagargs[@]}")
cmd+=("--tool=$toolname")
cmd+=("${nonflags[@]}")
cmd+=("$binary")
cmd+=("${exeargs[@]}")
echo_err "Running: ${cmd[*]}"
"${cmd[@]}"

exit
