#!/bin/bash

# Run test executable, with optional wrapper/args.
# This was created to make the Makefile a little easier to understand/write.
# -Christopher Welborn 07-13-2019
appname="ColrC - Test Runner"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
colrdir="$(readlink -f "${appdir}/..")"
examplesdir="${colrdir}/examples"
toolsdir="${colrdir}/tools"
colrexe="${colrdir}/colrc"
testexe="${appdir}/test_colrc"

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

# Some colors.
BLUE="${BLUE:-\x1b[1;34m}"
GREEN="${GREEN:-\x1b[1;32m}"
RED="${RED:-\x1b[1;31m}"
NC="${NC:-\x1b[0m}"

function echo_err {
    # Echo to stderr.
    printf "%s" "$RED" 1>&2
    echo -e "$@" 1>&2
    printf "%s" "$NC" 1>&2
}

function echo_status {
    printf "%s" "$BLUE"
    echo -e "$@"
    printf "%s" "$NC"
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

function is_debug_exe {
    # Returns a 0 exit status if the argument is an executable built with
    # debug info.
    local exe=$1
    [[ -n "$exe" ]] || fail "No executable given to is_debug_exe()!"

    if file "$exe" | grep "debug" &>/dev/null; then
        return 0
    fi
    return 1
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript [-h | -v]
        $appscript -A
        $appscript [ARGS...]
        $appscript -e exe [ARGS...]
        $appscript -m [ARGS...]

    Options:
        ARGS              : Optional arguments for the test executable.
        -A,--all          : Run every kind of test, even examples.
        -e exe,--exe exe  : Run another executable with the test executable
                            and ARGS as arguments.
        -h,--help         : Show this message.
        -m,--memcheck     : Run the test executable through valgrind.
        -v,--version      : Show $appname version and exit.

    Test executable: $default_binary
    "
}

function make_colrtool {
    # Clean the colr tool build, rebuild it, and run it.
    local target="${1:-debug}"
    shift
    pushd "$colrdir" || fail "Unable to cd into colr directory: $colrdir"
    make clean "$target" "$@" || fail "\nUnable to build colr tool ($target)!"
    popd
}
function make_tests {
    # Clean the test build, rebuild it, and run it.
    local target="${1:-debug}"
    shift
    pushd "$appdir" || fail "Unable to cd into test directory: $appdir"
    make clean "$target" "$@" || fail "\nCan't even build the tests ($target)!"
    make test || fail "\nUnit tests failed."
    popd
}

function run_examples {
    # Clean all of the examples in ../examples, rebuild them, and run them.
    pushd "$examplesdir" || fail "Unable to cd into examples directory: $examplesdir"
    make clean all
    ./run_examples.sh "$@" || fail "\nExamples failed."
    popd
}

function run_everything {
    # Run every single unit test, example, source-example, and anything else
    # thay may show a failure, and run them through memcheck if possible.
    local rebuild_colr="" rebuild_tests=""
    [[ -e "$colrexe" ]] && {
        rebuild_colr="release"
        is_debug_exe "$colrexe" && rebuild_colr="debug"
    }
    [[ -e "$testexe" ]] && {
        rebuild_tests="release"
        is_debug_exe "$testexe" && rebuild_tests="debug"
    }

    echo_status "\nTrying to build $colrexe in debug mode..."
    COLR_ARGS="TEST red white underline" make_colrtool debug memcheckquiet 1>/dev/null
    echo_status "\nTrying to build $testexe in debug mode..."
    make_tests debug memcheckquiet 1>/dev/null
    echo_status "\nRunning examples..."
    run_examples --memcheck --quiet 1>/dev/null
    echo_status "\nRunning source examples..."
    run_source_examples --memcheck --quiet 1>/dev/null

    echo_status "\nBuilding test coverage..."
    make_tests coverage 1>/dev/null

    echo_status "\nBuilding $colrexe in release mode..."
    COLR_ARGS="TEST red white underline" make_colrtool release memcheckquiet 1>/dev/null
    echo_status "\nBuilding $testexe in release mode..."
    make_tests release memcheckquiet 1>/dev/null

    local do_colr_rebuild=0
    if [[ -n "$rebuild_colr" ]]; then
        if is_debug_exe "$colrexe"; then
             [[ "$rebuild_colr" == "debug" ]] || do_colr_rebuild=1
        else
            [[ "$rebuild_colr" == "release" ]] || do_colr_rebuild=1
        fi
    fi
    if ((do_colr_rebuild)); then
        echo_status "\nRebuilding $colrexe for $rebuild_colr mode..."
        make_colrtool clean "$rebuild_colr" 1>/dev/null || \
            fail "\nTried to rebuild in $rebuild_colr mode, and failed."
    else
        rebuild_colr="release"
    fi
    local do_test_rebuild=0
    if [[ -n "$rebuild_tests" ]]; then
        if is_debug_exe "$testexe"; then
             [[ "$rebuild_tests" == "debug" ]] || do_test_rebuild=1
        else
            [[ "$rebuild_tests" == "release" ]] || do_test_rebuild=1
        fi
    fi
    if ((do_test_rebuild)); then
        echo_status "\nRebuilding $testexe for $rebuild_tests mode..."
        make_tests clean "$rebuild_tests" 1>/dev/null || \
            fail "\nTried to rebuild tests in $rebuild_tests mode, and failed."
    else
        rebuild_tests="release"
    fi


    [[ -n "$rebuild_colr" ]] || rebuild_colr="release"
    [[ -n "$rebuild_tests" ]] || rebuild_tests="release"
    binmode="${colrexe##*/}:$rebuild_colr, ${testexe##*/}:$rebuild_tests"
    printf "\n%sSuccess%s, the binaries are: %s\n" "$GREEN" "$NC" "$binmode"
}

function run_source_examples {
    # Build all of the source examples, and run them.
    pushd "$colrdir"
    python3 "${toolsdir}/snippet.py" --examples "$@" || fail "\nSource examples failed."
    popd
}

declare -a userargs
in_exe=0
exe_wrapper=""
do_all=0
do_memcheck=0

for arg; do
    case "$arg" in
        "-A" | "--all")
            do_all=1
            ;;
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
if ((do_all)); then
    run_everything
    exit
elif ((do_memcheck)); then
    cmd=(
        "valgrind"
        "--tool=memcheck"
        "--leak-check=full"
        "--track-origins=yes"
        "--error-exitcode=1"
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
