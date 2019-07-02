#!/bin/bash

# ...
# -Christopher Welborn 07-02-2019
appname="run_examples"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
declare -a binaries=($(find "$appdir" -maxdepth 1 -executable -type f ! -name "*.*"))

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

function pattern_matches {
    local pat=$1 s=$2
    # No pattern? Everything matches.
    [[ -n "$pat" ]] || return 0
    # No string? Everything matches.
    [[ -n "$s" ]] || return 0

    [[ "$s" =~ $pat ]] || return 1
    return 0
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript PATTERN

    Options:
        PATTERN       : Only run executables matching this regex/text pattern.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}

function run_exe {
    local exename=$1
    [[ -n "$exename" ]] || fail "No executable given to \`run_exe\`!"
    [[ -x "$exename" ]] || fail "Not an executable: $exename"
    printf "\nRunning %s...\n" "$exename"
    "$exename"
}

((${#binaries})) || fail "No binaries built yet. Run \`make\`."

declare -a nonflags
pattern=""

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
            nonflags+=("$arg")
    esac
done

((${#nonflags[@]})) &&  pattern="${nonflags[0]}"

let count=0
let errs=0
for binaryname in "${binaries[@]}"; do
    if pattern_matches "$pattern" "$binaryname"; then
        run_exe "$binaryname" || let errs+=1
        let count+=1
    fi
done

if ((count)); then
    plural="examples"
    ((count == 1)) && plural="example"
    printf "\nRan %s %s" "$count" "$plural"
    if [[ -n "$pattern" ]]; then
        printf " matching: %s\n" "$pattern"
    else
        printf "\n"
    fi
else
    if [[ -n "$pattern" ]]; then
        echo_err "\nNo examples matching: $pattern"
    else
        echo_err "\nNo executables found!"
    fi
fi
exit $errs
