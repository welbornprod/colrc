#!/bin/bash

# Wrapper around replacestr.py to make multiple renames/refactors easier.
# -Christopher Welborn 08-08-2019
appname="refactor"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"

hash replacestr &>dev/null || {
    printf "\nMissing \`replacestr\` (replacestr.py).\n" 1>&2
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
        $appscript -h | -v
        $appscript TARGET REPL
        $appscript -s str repl TARGET...
    Options:
        REPL          : Replacement for the thing.
        TARGET        : Thing to replace.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}

(( $# > 0 )) || fail_usage "No arguments!"

declare -a targets replacements
do_str_repl=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-s" | "--substring")
            do_str_repl=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            fail_usage "Unknown flag argument: $arg"
            ;;
        *)
            if ((do_str_repl)) || ((${#targets[@]} == 0)); then
                targets+=("$arg")
            elif ((${#replacements[@]} == 0)); then
                replacements+=("$arg")
            elif ((${#targets[@]} > ${#replacements[@]})); then
                replacements+=("$arg")
            else
                targets+=("$arg")
            fi
    esac
done

if ((do_str_repl)); then
    str_target="${targets[0]}"
    str_repl="${targets[1]}"
    targets=("${targets[@]:2}")
    [[ -n "$str_target" ]] || fail "No target string for substring replacement."
    [[ -n "$str_repl" ]] || fail "No replacement string for substring replacement."
    ((${#targets[@]})) || fail "No targets for substring replacement."
    # Run replacestr for substring replacement of targets.
    for target in "${targets[@]}"; do
        replacement="${target/$str_target/$str_repl}"
        replacements+=("$replacement")
    done
fi

((${#targets[@]} == ${#replacements[@]})) || fail "Need matching replacement for every target."
# Run replacestr for all target/repl pairs.
let i=0
for target in "${targets[@]}"; do
    replacement="${replacements[$i]}"
    replacestr -R -e '.c,.h' "$target" "$replacement"
    let i+=1
done


