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

function merge_patterns {
    # Print all arguments wrapped in parens, joined by '|'.
    local wrapped
    IFS=$'\n' declare -a wrapped=($(printf "(%s)|\n" "$@"))
    printf "%s" "${wrapped::-1}"
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
        $appscript [-a | -s] PATTERN...

    Options:
        PATTERN       : Only run executables matching these regex/text patterns.
        -a,--all      : Run all examples, including the source code examples.
        -h,--help     : Show this message.
        -s,--source   : Use examples found in the source code.
                        This is a shortcut for:
                            \`../tools/snippet.py -x [PATTERN...]\`
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

declare -a patterns
do_all=0
do_source=0

for arg; do
    case "$arg" in
        "-a" | "--all")
            do_all=1
            do_source=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-s" | "--source")
            do_source=1
            do_all=0
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            fail_usage "Unknown flag argument: $arg"
            ;;
        *)
            patterns+=("$arg")
    esac
done

((do_source)) && {
    ../tools/snippet.py --examples "$(merge_patterns "${patterns[@]}")"
    ((do_all)) || exit $?
}

let count=0
let errs=0
declare -a matched_patterns unmatched_patterns
for binaryname in "${binaries[@]}"; do
    if ((${#patterns[@]})); then
        for pattern in "${patterns[@]}"; do
            if pattern_matches "$pattern" "$binaryname"; then
                run_exe "$binaryname" || let errs+=1
                [[ "${matched_patterns[*]}" == *"$pattern"* ]] || matched_patterns+=("$pattern")
                let count+=1
                continue
            fi
        done
    else
        run_exe "$binaryname" || let errs+=1
        let count+=1
    fi
done
for pattern in "${patterns[@]}"; do
    [[ "${matched_patterns[*]}" == *"$pattern"* ]] || unmatched_patterns+=("$pattern")
done

if ((count)); then
    plural="examples"
    ((count == 1)) && plural="example"
    printf "\nRan %s %s" "$count" "$plural"
    if ((${#patterns[@]})); then
        printf " matching: %s\n" "${matched_patterns[*]}"
        ((${#unmatched_patterns[@]})) && printf "No matches for: %s\n" "${unmatched_patterns[*]}"
    else
        printf "\n"
    fi
else
    if ((${#patterns[@]})); then
        echo_err "\nNo examples matching: ${patterns[*]}"
    else
        echo_err "\nNo executables found!"
    fi
fi
exit $errs
