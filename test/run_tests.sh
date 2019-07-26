#!/bin/bash

# Run test executable, with optional wrapper/args.
# This was created to make the Makefile a little easier to understand/write.
# -Christopher Welborn 07-13-2019
appname="ColrC - Test Runner"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

declare -a binaries=($(find "$appdir" -maxdepth 1 -executable -type f -name "test_*" ! -name "*.*"))
((${#binaries[@]})) || {
    printf "\nNo test binary found in: %s\n" "$appdir" 1>&2
    exit 1
}
default_binary="${binaries[0]}"
binary_name="${default_binary##*/}"
[[ "$binary_name" == test_* ]] || {
    printf "\nExecutable does not look like a test executable: %s\n" "$default_binary" 1>&2
    exit 1
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
        $appscript [-h | -v]
        $appscript [ARGS...]
        $appscript -e exe [ARGS...]
        $appscript -m [ARGS...]

    Options:
        ARGS              : Optional arguments for the test executable.
        -e exe,--exe exe  : Run another executable with the test executable
                            and ARGS as arguments.
        -h,--help         : Show this message.
        -m,--memcheck     : Run the test executable through valgrind.
        -v,--version      : Show $appname version and exit.

    Test executable: $default_binary
    "
}

declare -a userargs
in_exe=0
exe_wrapper=""
do_memcheck=0

for arg; do
    case "$arg" in
        "-e" | "--exe")
            if [[ -z "$exe_wrapper" ]]; then
                in_exe=1
            else
                flagargs+=("$arg")
            fi
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-m" | "--memcheck")
            do_memcheck=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            userargs+=("$arg")
            ;;
        *)
            if ((in_exe)); then
                exe_wrapper=$arg
                in_exe=0
            else
                userargs+=("$arg")
            fi
    esac
done

declare -a cmd
if ((do_memcheck)); then
    cmd=(
        "valgrind"
    "--tool=memcheck"
    "--leak-check=full"
    "--track-origins=yes"
    "$default_binary"
    "${userargs[@]}"
    )
elif [[ -n "$exe_wrapper" ]]; then
    cmd=("$exe_wrapper" "$default_binary" "${userargs[@]}")
else
    cmd=("$default_binary" "${userargs[@]}")
fi

printf "\nRunning: %s\n" "${cmd[*]}"
"${cmd[@]}"
exit
