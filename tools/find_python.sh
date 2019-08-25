#!/bin/bash

# Find the default python version/executable.
# -Christopher Welborn 08-24-2019
appname="Python Finder"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"


function debug {
    ((do_debug)) || return 0;
    local msgcolor="${cyan:-\x1b[36m}"
    printf "debug: %s" "$msgcolor" 1>&2
    # shellcheck disable=SC2059
    printf "$@" 1>&2
    printf "%s\n" "${NC:-\x1b[0m}" 1>&2
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

    Prints the first non-2.7 python executable found.

    Usage:
        $appscript -h | -v
        $appscript [-f] [-l] [MIN] [MAX]

    Options:
        MAX           : Maximum python version, in the form: X.Y
                        Default: $py_max_default
        MIN           : Minimum python version, in the form: X.Y
                        Default: $py_min_default
        -f,--full     : Print the full path.
        -h,--help     : Show this message.
        -l,--long     : Use only long-form executables like \`python3.7\` instead
                        of \`python3\`.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a nonflags
do_debug=0
do_full=0
do_long=0
py_min_default="3.0"
py_max_default="4.9"
py_min=""
py_max=""

for arg; do
    case "$arg" in
        "-D" | "--debug")
            do_debug=1
            ;;
        "-f" | "--full")
            do_full=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-l" | "--long")
            do_long=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            fail_usage "Unknown flag argument: $arg"
            ;;
        *)
            if [[ -z "$py_min" ]]; then
                py_min=$arg
            elif [[ -z "$py_max" ]]; then
                py_max=$arg
            else
                nonflags+=("$arg")
            fi
    esac
done
# Use defaults.
[[ -z "$py_min" ]] && py_min=$py_min_default
[[ -z "$py_max" ]] && py_max=$py_max_default
[[ "$py_min" == *.* ]] || py_min="${py_min}.0"
[[ "$py_max" == *.* ]] || py_max="${py_max}.0"

debug "Using min: $py_min, max: $py_max"

minmajor="${py_min%%.*}"
stopmajor=$minmajor
let stopmajor-=1

maxmajor="${py_max%%.*}"
trymajor=$maxmajor
pyminor="${py_max##*.}"
py_name=""
while ((trymajor > stopmajor)); do
    ((do_long)) || {
        # Try just the short-form major version.
        tryexe="python${trymajor}"
        debug "Trying short-form: $tryexe"
        hash "$tryexe" &>/dev/null && {
            py_name=$tryexe
            break;
        }
    }
    while ((pyminor > -1)); do
        tryexe="python${trymajor}.${pyminor}"
        debug "Trying long-form: $tryexe"
        hash "$tryexe" &>/dev/null && {
            py_name=$tryexe
            break
        }
        let pyminor-=1
    done
    let trymajor-=1
    let pyminor=9
    [[ -z "$py_name" ]] || break;
done
[[ -z "$py_name" ]] && {
    # Still no python executable.
    ((do_long)) || {
        debug "Trying default: python"
        hash "python" &>/dev/null && py_name="python"
    }
}
[[ -n "$py_name" ]] || fail "No suitable python executable found."
verstr="$("$py_name" --version 2>&1)" || fail "Cannot run python executable: $py_name"
debug "Version: $verstr"
vernum="${verstr,,}"
vernum="${vernum//python /}"
debug "Number: $vernum"
# Check version string, to be certain the name isn't a lie :/.
vermajor="${vernum%%.*}"
debug "Major: $vermajor"
if ((vermajor < minmajor)) || ((vermajor > maxmajor)); then
    fail "Version isn't compatible: $vernum is not in the range $py_min-$py_max"
fi
verminor="${vernum#*.}"
verminor="${verminor%%.*}"
debug "Minor: $verminor"

debug "Using python exe: $py_name"
py_exe=$py_name
((do_full)) && py_exe="$(readlink -f "$py_name")"
printf "%s\n" "$py_exe"
exit 0
