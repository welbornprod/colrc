#!/bin/bash

# Wrapper around replacestr.py to make multiple renames/refactors easier.
# -Christopher Welborn 08-08-2019
appname="refactor"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

py_cmd="$(bash "$appdir/find_python.sh" -f 3.5)" || fail "No python executable found."
replace_str_file="${appdir}/replacestr.py"
declare -a replace_str_cmd=("$py_cmd" "$replace_str_file")
replace_str_display="$py_cmd $(realpath --relative-to="$appdir" "$replace_str_file")"
[[ -e "$replace_str_file" ]] || {
    printf "\nMissing \`replacestr\` (replacestr.py): %s\n" "$replace_str_file" 1>&2
    exit 1
}

function add_nonflag_arg {
    # Modify the global targets/replacements arrays, based on current state
    # and the current "mode" of operation.
    local arg=$1
    [[ -n "$arg" ]] || fail "Missing argument for add_nonflag_arg()!"
    [[ "$arg" =~ $escape_pat ]] && {
        debug "Sending escaped arg: $arg"
    }
    if ((do_str_repl)) || ((${#targets[@]} == 0)); then
        targets+=("$arg")
        debug "Adding first target: $arg"
    elif ((${#replacements[@]} == 0)); then
        replacements+=("$arg")
        debug "Adding first replacement: $arg"
    elif ((${#targets[@]} > ${#replacements[@]})); then
        replacements+=("$arg")
        debug "Adding next replacement: $arg"
    else
        targets+=("$arg")
        debug "Adding next target: $arg"
    fi
}

function debug {
    ((debug_mode)) || return 0
    # shellcheck disable=SC2059
    # ..i know shellcheck, it's a wrapper. I can do this.
    printf "$@" 1>&2
    printf "\n" 1>&2
}

function debugn {
    ((debug_mode)) || return 0
    # shellcheck disable=SC2059
    # ..i know shellcheck, it's a wrapper. I can do this.
    printf "$@" 1>&2
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
        REPL            : Replacement for the thing.
        TARGET          : Thing to replace.
        -D,--debug      : Show some more info while running.
        -h,--help       : Show this message.
        -s,--substring  : Replace \`str\` with \`repl\` in all targets.
        -v,--version    : Show $appname version and exit.

    Using: \`$replace_str_display\`
    "
}

(( $# > 0 )) || fail_usage "No arguments!"

declare -a targets replacements
do_str_repl=0
debug_mode=0
escape_pat='\\-.+'

for arg; do
    case "$arg" in
        "-D" | "--debug")
            debug_mode=1
            ;;
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
            add_nonflag_arg "$arg"
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
((debug_mode)) && replace_str_cmd+=("--debug")
# Run replacestr for all target/repl pairs.
let i=0
for target in "${targets[@]}"; do
    replacement="${replacements[$i]}"
    "${replace_str_cmd[@]}" -R -e '.c,.h' "$target" "$replacement"
    let i+=1
done


