#!/bin/bash

# Run valgrind cachegrind, callgrind, or memcheck on this project.
# -Christopher Welborn 06-09-2019
appname="Valgrind Runner"
appversion="0.0.3"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"

declare -A script_deps=(["valgrind"]="valgrind")
for script_dep in "${!script_deps[@]}"; do
    hash "$script_dep" &>/dev/null || {
        printf "\nMissing \`%s\` command.\n" "$script_dep" 1>&2
        printf "Install the \`%s\` package with your package manager.\n" "${script_deps[$script_dep]}" 1>&2
        exit 1
    }
done

# This should be the only variable that needs to change for other projects.
binaryname=$(find . -maxdepth 1 -type f -executable '!' -name "*.*" | head -n 1)
if [[ -z "$binaryname" ]]; then
    binaryname="<none>"
    binary=""
else
    binary=$(realpath "$binaryname")
fi
default_tool="memcheck"
cachegrind_file="cachegrind.out.${binaryname##*/}"
callgrind_file="callgrind.out.${binaryname##*/}"

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
        $appscript [-e exe] [-q] [TOOL] [ARG...] -- [EXE_ARGS...]

    Options:
        ARG              : One or more extra arguments for valgrind.
        EXE_ARGS         : Arguments for $binaryname while running the test.
        TOOL             : Tool to use. Must be the first non-flag argument if
                           not using the --tool option.
                           Can be one of: cachegrind, callgrind, memcheck
                           Default: $default_tool
        -e exe,--exe exe : Executable to run.
                           Default: $binaryname
        -h,--help        : Show this message.
        -q,--quiet       : Run valgrind in quiet mode.
        -V,--version     : Show $appname version and exit.
    "
}

declare -a nonflags
declare -a flagargs
declare -a exeargs
in_exe_args=0
in_binary_arg=0
do_quiet=0

for arg; do
    case "$arg" in
        "--")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            in_exe_args=1
            ;;
        "-e" | "--exe")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            in_binary_arg=1
            ;;
        "-h" | "--help")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            print_usage ""
            exit 0
            ;;
        "-q" | "--quiet")
            ((in_exe_args)) && {
                exeargs+=("$arg")
                continue
            }
            do_quiet=1
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
            ((in_binary_arg)) && {
                binaryname=$arg
                binary=$(realpath "$binaryname")
                in_binary_arg=0
                continue
            }
            nonflags+=("$arg")
            ;;
    esac
done
[[ -n "$binary" ]] || fail "No binary found, and none given either."
[[ -e "$binary" ]] || fail "The binary does not exist: $binary"

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
        nonflags+=(
            "--callgrind-out-file=$callgrind_file"
            "--dump-instr=yes"
            "--collect-jumps=yes"
        )
        ;;
    "memcheck" | "mem")
        toolname="memcheck"
        [[ "${nonflags[*]}" =~ leak-check ]] || nonflags+=("--leak-check=full")
        [[ "${nonflags[*]}" =~ show-leak-kinds ]] || nonflags+=("--show-leak-kinds=all")
        ;;
esac

declare -a cmd=("valgrind" "--error-exitcode=1")
((do_quiet)) && cmd+=("--quiet")
cmd+=("${flagargs[@]}")
cmd+=("--tool=$toolname")
cmd+=("${nonflags[@]}")
cmd+=("$binary")
cmd+=("${exeargs[@]}")

((do_quiet)) || echo_err "Running: ${cmd[*]}"
"${cmd[@]}"

exit
