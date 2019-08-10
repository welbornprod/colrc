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
testdir=$appdir # May change.
toolsdir="${colrdir}/tools"
colrexe="${colrdir}/colrc"
testexe="${appdir}/test_colrc"

declare -a binaries=($(find "$appdir" -maxdepth 1 -executable -type f -name "test_*" ! -name "*.*"))
((${#binaries[@]})) || {
    if [[ "$*" != *-A* ]] && [[ "$*" != *--all* ]]; then
        printf "\nNo test binary found in: %s\n" "$appdir" 1>&2
        exit 1
    fi
}
default_binary="${binaries[0]}"
binary_name="${default_binary##*/}"
[[ "$binary_name" == test_* ]] || {
    if [[ "$*" != *-A* ]] && [[ "$*" != *--all* ]]; then
        printf "\nExecutable does not look like a test executable: %s\n" "$default_binary" 1>&2
        exit 1
    fi
}

# Some colors.
BLUE="${BLUE:-\x1b[1;34m}"
GREEN="${GREEN:-\x1b[1;32m}"
RED="${RED:-\x1b[1;31m}"
blue="${blue:-\x1b[1;34m}"
green="${green:-\x1b[1;32m}"
red="${red:-\x1b[1;31m}"
NC="${NC:-\x1b[0m}"

function colr_anim {
    # Run a command through colr-run, with a custom message.
    ((no_colr)) && {
        shift
        "$@"
        return
    }
    local msg=$1
    shift
    if [[ "$msg" == "-" ]]; then
        colr-run -- "$@" 1>&2
    else
        colr-run -a -e -m "$msg" -- "$@" 1>&2
    fi
}

function echo_err {
    # Echo to stderr.
    printf "%s" "$RED" 1>&2
    echo -e "$@" 1>&2
    printf "%s" "$NC" 1>&2
}

function echo_status {
    local lbl=$1 val
    shift
    declare -a val=("$@")
    if ((${#val[@]} == 0)); then
        printf "\n%s%s%s\n" "$blue" "$lbl" "$NC" 1>&2
    else
        printf "\n%s%s%s: %s%s\n" "$blue" "$lbl" "$green" "${val[*]}" "$NC" 1>&2
    fi
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
        $appscript [-n] [-q] -A
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
        -n,--no-anim      : Don't use colr-run animations.
        -q,--quiet        : Only print basic status messages and errors.
        -v,--version      : Show $appname version and exit.

    Test executable: $default_binary
    "
}

function make_in {
    # Clean the colr tool build, rebuild it, and run it.
    local targets dirpath=$1
    shift
    [[ -e "$dirpath" ]] || fail "Directory doesn't exist: $dirpath"
    declare -a targets=("$@")
    ((${#targets[@]})) || targets=("debug")
    pushd "$dirpath" || fail "Unable to cd into: $dirpath"
    make "${targets[@]}" || fail "\nmake target failed: ${targets[*]}"
    popd
}

function run_colrc_modes {
    local colrc_cmd run_mode=$1
    declare -a colrc_cmd=("$colrexe")
    [[ "$run_mode" == "memcheck"* ]] && colrc_cmd=(
        "$toolsdir/valgrind_run.sh"
        "-e" "$colrexe"
    )
    [[ "$run_mode" == "memcheck-quiet" ]] && colrc_cmd+=("--quiet")
    colrc_cmd+=("--")

    "${colrc_cmd[@]}" "Testing colr in sanitize mode." red white underline || \
        fail "Failed on basic colorization."
    "${colrc_cmd[@]}" "Testing colr in sanitize mode." white rainbow || \
        fail "Failed on back rainbow."
    "${colrc_cmd[@]}" "Testing colr in sanitize mode." rainbow black || \
        fail "Failed on fore rainbow."
    "${colrc_cmd[@]}" --basic || \
        fail "Failed on basic colors example."
    "${colrc_cmd[@]}" --256 || \
        fail "Failed on 256-colors example."
    "${colrc_cmd[@]}" --names || \
        fail "Failed on known names example."
    "${colrc_cmd[@]}" --rainbow || \
        fail "Failed on rainbow colors example."
    "${colrc_cmd[@]}" --rgb || \
        fail "Failed on rgb colors example."
}

function run_examples {
    # Clean all of the examples in ../examples, rebuild them, and run them.
    pushd "$examplesdir" || fail "Unable to cd into examples directory: $examplesdir"
    make clean all
    colr_anim "Running example code..." \
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
    local memcheck_target="memcheck" example_args
    declare -a example_args=("--memcheck")
    ((do_quiet)) && {
        memcheck_target="memcheckquiet"
        example_args+=("--quiet")
    }
    echo_status "Trying to build (debug)" "$colrexe"
    make_in "$colrdir" clean debug

    echo_status "Trying to run memcheck on (debug)" "$colrexe"
    run_colrc_modes "memcheck-quiet"

    echo_status "Trying to build (debug)" "$testexe"
    make_in "$testdir" clean debug

    echo_status "Trying to run memcheck on (debug)" "$testexe"
    make_in "$testdir" "$memcheck_target"

    echo_status "Trying to build (sanitize)" "$colrexe"
    make_in "$colrdir" clean sanitize
    run_colrc_modes "normal"

    echo_status "Trying to build (sanitize)" "$testexe"
    make_in "$testdir" clean sanitize testquiet

    echo_status "Running examples..."
    run_examples "${example_args[@]}"

    echo_status "Running source examples..."
    run_source_examples "${example_args[@]}"

    echo_status "Building test coverage..."
    make_in "$testdir" clean coverage

    echo_status "Trying to build (release)" "$colrexe"
    make_in "$colrdir" clean release

    echo_status "Trying to run memcheck on (release)" "$colrexe"
    run_colrc_modes "memcheck-quiet"

    echo_status "Trying to build (release)" "$testexe"
    make_in "$testdir" clean release

    echo_status "Trying to run memcheck on (release)" "$testexe"
    make_in "$testdir" "$memcheck_target"

    local do_colr_rebuild=0
    if [[ -n "$rebuild_colr" ]]; then
        if is_debug_exe "$colrexe"; then
             [[ "$rebuild_colr" == "debug" ]] || do_colr_rebuild=1
        else
            [[ "$rebuild_colr" == "release" ]] || do_colr_rebuild=1
        fi
    fi
    if ((do_colr_rebuild)); then
        echo_status "Rebuilding ($rebuild_colr)" "$colrexe"
        make_in "$colrdir" clean "$rebuild_colr" || \
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
        echo_status "Rebuilding ($rebuild_tests)" "$testexe"
        make_in "$testdir" clean "$rebuild_tests" || \
            fail "\nTried to rebuild tests in $rebuild_tests mode, and failed."
    else
        rebuild_tests="release"
    fi

    rebuild_colr="release"
    rebuild_tests="release"
    is_debug_exe "$colrexe" && rebuild_colr="debug"
    is_debug_exe "$testexe" && rebuild_tests="debug"
    binmode="${colrexe##*/}:$rebuild_colr, ${testexe##*/}:$rebuild_tests"
    printf "\n%sSuccess%s, the binaries are: %s\n" "$GREEN" "$NC" "$binmode" 1>&2
}

function run_source_examples {
    # Build all of the source examples, and run them.
    pushd "$colrdir"
    colr_anim "Running source examples..." \
        python3 "${toolsdir}/snippet.py" --examples "$@" || fail "\nSource examples failed."
    popd
}

declare -a userargs
in_exe=0
exe_wrapper=""
do_all=0
do_memcheck=0
do_quiet=0
no_colr=0
hash colr-run &>/dev/null || no_colr=1

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
        "-n" | "--no-anim")
            no_colr=1
            ;;
        "-q" | "--quiet")
            do_quiet=1
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

# colr-run animations are only used when in quiet mode.
((do_quiet)) || no_colr=1

declare -a cmd
if ((do_all)); then
    if ((do_quiet)); then
        run_everything 1>/dev/null
    else
        run_everything
    fi
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
    ((do_quiet)) && userargs+=("--quiet")
    cmd=("$default_binary" "${userargs[@]}")
fi

printf "\nRunning: %s\n" "${cmd[*]}"
"${cmd[@]}"
exit
