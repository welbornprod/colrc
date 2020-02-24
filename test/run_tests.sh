#!/bin/bash

# Run test executable, with optional wrapper/args.
# This was created to make the Makefile a little easier to understand/write.
# -Christopher Welborn 07-13-2019
appname="ColrC - Test Runner"
appversion="0.0.5"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

colrdir="$(readlink -f "${appdir}/..")"
examplesdir="${colrdir}/examples"
testdir=$appdir # May change.
toolsdir="${colrdir}/tools"
colrexe="${colrdir}/colrc"
testexe="${appdir}/test_colrc"
declare -a is_build_cmd=("bash" "$toolsdir/is_build.sh")

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
yellow="${yellow:-\x1b[1;33m}"
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

function fail_colrc_mode {
    # Print a description, the run-mode, and the command that was used and exit.
    local desc="${1:-<unknown>}" run_mode="${2:-<unknown>}"
    shift 2
    local cmdargs
    declare -a cmdargs=("$@")
    ((${#cmdargs[@]})) || fail "Failed on $desc in $run_mode. No command given!"
    fail "Failed on $desc in $run_mode mode:\n    $(quote_args "${cmdargs[@]}")"
}

function fail_usage {
    # Print a usage failure message, and exit with an error status code.
    print_usage "$@"
    exit 1
}

function format_duration {
    local seconds="${1:-0}"
    ((seconds)) || {
        printf "%b0s%b" "$yellow" "$NC"
        return 1
    }
    local mins=$((seconds / 60))
    ((mins)) || {
        printf "%b%ss%b" "$yellow" "$seconds" "$NC"
        return 0
    }
    local secs=$((seconds - (mins * 60)))
    ((secs)) || {
        printf "%b%sm%b" "$yellow" "$mins" "$NC"
        return 0
    }
    printf "%b%sm%b" "$yellow" "$mins" "$NC"
    printf ":"
    printf "%b%ss%b" "$yellow" "$secs" "$NC"
    return 0
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
        $appscript [-m] [-n] [-q] -A
        $appscript [-m] [-q] [-a] [-x]
        $appscript [-m] [-q] [-C] [-E] [-M] [-S] [-X]
        $appscript [ARGS...]
        $appscript -e exe [ARGS...]
        $appscript -m [ARGS...]

    Options:
        ARGS              : Optional arguments for the test executable.
        -A,--all          : Run every kind of test, even examples.
        -a,--allmodes     : Run colrc commands on the current binary.
                            This is like: -C -E -M
        -C,--colors       : Run colrc colorization commands on the current
                            binary.
        -E,--examplecmds  : Run colrc example commands on the current binary.
        -e exe,--exe exe  : Run another executable with the test executable
                            and ARGS as arguments.
        -h,--help         : Show this message.
        -M,--commands     : Run colrc commands on the current binary.
        -m,--memcheck     : Run the test executable through valgrind.
        -n,--no-anim      : Don't use colr-run animations.
        -q,--quiet        : Only print basic status messages and errors.
        -S,--source       : Run source examples.
        -v,--version      : Show $appname version and exit.
        -X,--examples     : Run example binaries.
        -x,--allexamples  : Run source examples and example binaries.
                            This is like: -S -X

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
    colr_anim "-" make "${targets[@]}" || fail "\nmake target failed: ${targets[*]}"
    popd
}

function quote_args {
    # Wrap each argument in a quote and print it.
    local arg cnt=0
    # Looping to skip certain arguments.
    for arg; do
        ((cnt)) && printf " "
        if ((!cnt)) || [[ "$arg" == "|" ]]; then
            # Don't quote.
            printf "%s" "$arg"
        else
            printf '"%s"' "$arg"
        fi
        let cnt+=1
    done
}

function run_cmd {
    # Run a command, but redirect stdout to /dev/null for --quiet, if needed.
    if ((do_quiet)); then
        "$@" 1>/dev/null
    else
        "$@"
    fi
}

function run_colrc_cmds {
    local colrc_cmd run_mode="${1:-?}"
    [[ "$run_mode" == "?" ]] && run_mode="$("${is_build_cmd[@]}" -n -b)"

    declare -a colrc_cmd=("$colrexe")
    [[ "$run_mode" == "memcheck"* ]] && colrc_cmd=(
        "$toolsdir/valgrind_run.sh"
        "-e" "$colrexe"
    )
    [[ "$run_mode" == "memcheck"*"quiet" ]] && colrc_cmd+=("--quiet")
    [[ "$run_mode" == *memcheck* ]] && colrc_cmd+=("--")

    local cmdarg colrfeedcmd inputarg
    declare -a colrfeedcmd
    declare -a cmdargs=(
        "--stripcodes"
        "--listcodes"
    )
    local cmdmarker="XXX" use_text
    declare -a colrc_inputs=(
        "Testing ${BLUE}${cmdmarker}${NC} in ${GREEN}${run_mode}${NC} mode."
        "-"
    )
    for inputarg in "${colrc_inputs[@]}"; do
        for cmdarg in "${cmdargs[@]}"; do
            if [[ "$inputarg" == "-" ]]; then
                # Use stdin.
                colrfeedcmd=(
                    "$colrexe"
                    "Testing $cmdarg stdin in $run_mode mode."
                    "red"
                    "34"
                    "underline"
                )
                "${colrfeedcmd[@]}" | run_cmd "${colrc_cmd[@]}" "$cmdarg" || \
                    fail_colrc_mode "$cmdarg" "$run_mode" "${colrfeedcmd[@]}" "|" "${colrc_cmd[@]}" "$cmdarg"
            else
                # Use the TEXT option.
                use_text="${inputarg//$cmdmarker/$cmdarg}"
                run_cmd "${colrc_cmd[@]}" "$use_text" "$cmdarg" || \
                    fail_colrc_mode "$cmdarg" "$run_mode" "${colrc_cmd[@]}" "$use_text" "$cmdarg"
            fi
        done
    done
}

function run_colrc_colors {
    local colrc_cmd run_mode="${1:-?}"
    [[ "$run_mode" == "?" ]] && run_mode="$("${is_build_cmd[@]}" -n -b)"

    declare -a colrc_cmd=("$colrexe")
    [[ "$run_mode" == "memcheck"* ]] && colrc_cmd=(
        "$toolsdir/valgrind_run.sh"
        "-e" "$colrexe"
    )
    [[ "$run_mode" == "memcheck"*"quiet" ]] && colrc_cmd+=("--quiet")
    [[ "$run_mode" == *memcheck* ]] && colrc_cmd+=("--")
    declare -a colrc_inputs=(
        "Testing colrc on $run_mode mode."
        "-"
    )
    declare -A colrc_arg_inputs
    colrc_arg_inputs=(
        ["basic colorization"]="red white underline"
        ["back rainbow"]="reset rainbow -s bright"
        ["fore rainbow"]="rainbow -s bright"
        ["--rainbow fore"]="-R -s bright"
        ["--rainbow back"]="-f white -R -s bright"
        ["--rainbow fore color"]="-f black -R -s bright"
        ["--rainbow back color"]="-b white -R -s bright"
    )
    declare -a colrc_args
    local inputarg argdesc
    for inputarg in "${colrc_inputs[@]}"; do
        for argdesc in "${!colrc_arg_inputs[@]}"; do
            colrc_args=("${inputarg}" ${colrc_arg_inputs[$argdesc]})
            if [[ "$inputarg" == "-" ]]; then
                echo -n "Testing colrc stdin on $run_mode mode: $argdesc" | \
                    run_cmd "${colrc_cmd[@]}" "${colrc_args[@]}" || \
                        fail_colrc_mode "stdin basic colorization" "$run_mode" "${colrc_cmd[@]}" "${colrc_args[@]}"
            else
                colrc_args[0]="${colrc_args[0]}: $argdesc"
                run_cmd "${colrc_cmd[@]}" "${colrc_args[@]}" || \
                    fail_colrc_mode "basic colorization" "$run_mode" "${colrc_cmd[@]}" "${colrc_args[@]}"
            fi
        done
    done
}

function run_colrc_examples {
    local colrc_cmd run_mode="${1:-?}"
    [[ "$run_mode" == "?" ]] && run_mode="$("${is_build_cmd[@]}" -n -b)"

    declare -a colrc_cmd=("$colrexe")
    [[ "$run_mode" == "memcheck"* ]] && colrc_cmd=(
        "$toolsdir/valgrind_run.sh"
        "-e" "$colrexe"
    )
    [[ "$run_mode" == "memcheck"*"quiet" ]] && colrc_cmd+=("--quiet")
    [[ "$run_mode" == *memcheck* ]] && colrc_cmd+=("--")

    declare -A exargs=(
        ["basic-colors"]="--basic"
        ["256-colors"]="--256"
        ["known-names"]="--names"
        ["rainbow"]="--rainbowize"
        ["rgb-colors"]="--rgb"
    )
    local argdesc
    for argdesc in "${!exargs[@]}"; do
        run_cmd "${colrc_cmd[@]}" "${exargs[$argdesc]}" || \
            fail_colrc_mode "$argdesc" "$run_mode" "${colrc_cmd[@]}" "${exargs[$argdesc]}"
    done
}

function run_colrc_modes {
    local run_mode="${1:-?}"
    [[ "$run_mode" == "?" ]] && run_mode="$("${is_build_cmd[@]}" -n -b)"
    # Colorization
    run_colrc_colors "$run_mode" || return 1
    # Examples
    run_colrc_examples "$run_mode" || return 1
    # Commands
    run_colrc_cmds "$run_mode" || return 1
}

function run_examples {
    # Clean all of the examples in ../examples, rebuild them, and run them.
    pushd "$examplesdir" || fail "Unable to cd into examples directory: $examplesdir"
    local cleantarget="clean"
    ((do_quiet)) && cleantarget="cleanquiet"
    make "$cleantarget" all
    colr_anim "Running example code..." \
        ./run_examples.sh "$@" || fail "\nExamples failed."
    popd
}

function run_everything {
    # Run every single unit test, example, source-example, and anything else
    # thay may show a failure, and run them through memcheck if possible.
    local rebuild_colr="" rebuild_tests=""
    local start_time
    start_time="$(date +"%s")"

    [[ -e "$colrexe" ]] && {
        rebuild_colr="release"
        is_debug_exe "$colrexe" && rebuild_colr="debug"
    }
    [[ -e "$testexe" ]] && {
        rebuild_tests="release"
        is_debug_exe "$testexe" && rebuild_tests="debug"
    }
    local memcheck_target="memcheck"
    declare -a exampleargs=("--memcheck")
    ((do_quiet)) && {
        memcheck_target="memcheckquiet"
        exampleargs+=("--quiet")
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
    echo_status "Trying to run (sanitize)" "$colrexe"
    run_colrc_modes "sanitize"

    echo_status "Trying to build/run (sanitize)" "$testexe"
    make_in "$testdir" clean sanitize testquiet

    echo_status "Running examples"
    run_examples "${exampleargs[@]}"

    echo_status "Running source examples"
    run_source_examples "${exampleargs[@]}"

    echo_status "Building test coverage"
    make_in "$testdir" clean coverage

    echo_status "Trying to build (release)" "$colrexe"
    make_in "$colrdir" clean release

    echo_status "Trying to run memcheck on (release)" "$colrexe"
    run_colrc_modes "memcheck-quiet"

    echo_status "Trying to build/run (release)" "$testexe"
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
    fi

    local colrc_build_name test_build_name

    colrc_build_name="$("${is_build_cmd[@]}" --color --status "$colrexe")"
    test_build_name="$("${is_build_cmd[@]}" --color --status "$testexe")"
    local duration
    duration=$(($(date "+%s") - start_time))
    printf "\n%sSuccess%s, the binaries are: %s, %s\n(%s)\n" \
        "$GREEN" "$NC" \
        "$colrc_build_name" "$test_build_name" \
        "$(format_duration "$duration")" 1>&2
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

do_cmds_mode=0
do_colrc_examples_mode=0
do_colors_mode=0
do_examples_mode=0
do_source_mode=0

hash colr-run &>/dev/null || no_colr=1

for arg; do
    case "$arg" in
        "-A" | "--all")
            do_all=1
            ;;
        "-Aq" | "-qA")
            # Convenience short-hand for '-A -q' or '-q -A'
            do_all=1
            do_quiet=1
            ;;
        "-a" | "--allmodes")
            do_colors_mode=1
            do_colrc_examples_mode=1
            do_cmds_mode=1
            ;;
        "-C" | "--colors")
            do_colors_mode=1
            ;;
        "-E" | "--examplecmds")
            do_colrc_examples_mode=1
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
        "-M" | "--commands")
            do_cmds_mode=1
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
        "-S" | "--source")
            do_source_mode=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        "-X" | "--examples")
            do_examples_mode=1
            ;;
        "-x" | "--allexamples")
            do_source_mode=1
            do_examples_mode=1
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

# Just run the binary in the different "modes"
# The '?' just runs the binary in it's current state.
user_mode="?"
((do_memcheck)) && {
    user_mode="memcheck"
    ((do_quiet)) && user_mode="memcheck-quiet"
}

declare -a example_args
((do_quiet)) && example_args+=("--quiet")
((do_memcheck)) && example_args+=("--memcheck")
((do_colors_mode)) && run_colrc_colors "$user_mode"
((do_colrc_examples_mode)) && run_colrc_examples "$user_mode"
((do_cmds_mode)) && run_colrc_cmds "$user_mode"
((do_examples_mode)) && run_examples "${example_args[@]}"
((do_source_mode)) && run_source_examples "${example_args[@]}"

# Exit if we are hand-picking stuff to run.
((
    do_colors_mode ||
    do_colrc_examples_mode ||
    do_cmds_mode ||
    do_source_mode ||
    do_examples_mode
)) && exit

declare -a cmd
if ((do_all)); then
    if ((do_quiet)); then
        run_everything 1>/dev/null
    else
        run_everything
    fi
    exit
elif ((do_memcheck)); then
    declare -a use_args
    if [[ -n "$exe_wrapper" ]]; then
        if [[ "$exe_wrapper" == "kdbg" ]]; then
            for userarg in "${userargs[@]}"; do
                use_args+=("-a" "$userarg")
            done
        else
            use_args=("${userargs[@]}")
        fi
        use_binary=$exe_wrapper
        use_args=("$default_binary" "${use_args[@]}")
    else
        use_binary=$default_binary
        use_args=("${userargs[@]}")
    fi
    cmd=(
        "valgrind"
        "--tool=memcheck"
        "--leak-check=full"
        "--track-origins=yes"
        "--error-exitcode=1"
        "$use_binary"
        "--"
        "${use_args[@]}"
    )
elif [[ -n "$exe_wrapper" ]]; then
    declare -a use_args
    if [[ "$exe_wrapper" == "kdbg" ]]; then
        for userarg in "${userargs[@]}"; do
            use_args+=("-a" "$userarg")
        done
    else
        use_args=("--" "${userargs[@]}")
    fi
    cmd=("$exe_wrapper" "$default_binary" "${use_args[@]}")
else
    ((do_quiet)) && userargs+=("--quiet")
    cmd=("$default_binary" "${userargs[@]}")
fi

printf "\nRunning: %s\n" "${cmd[*]}"
# This environment variable has to be set until I figure out how to let
# libasan know about the register_printf_specifier() call for colr_printf stuff.
ASAN_OPTIONS="check_printf=0" "${cmd[@]}"
exit
