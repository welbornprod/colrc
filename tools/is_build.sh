#!/bin/bash

# Looks at an executable to determine if it was built with debug, address
#  sanitizer (libasan), or neither of those (release mode).
# -Christopher Welborn 08-09-2019
appname="is_build"
appversion="0.0.3"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"

# This should be the only variable that needs to change for other projects.
binaryname=$(find . -maxdepth 1 -type f -executable '!' -name "*.*" | head -n 1)
if [[ -z "$binaryname" ]]; then
    binaryname="<none>"
    binary=""
else
    binary=$(realpath "$binaryname")
fi

# Some colors.
BLUE="${BLUE:-\x1b[1;34m}"
GREEN="${GREEN:-\x1b[1;32m}"
RED="${RED:-\x1b[1;31m}"
YELLOW="${YELLOW:-\x1b[1;33m}"
blue="${blue:-\x1b[1;34m}"
green="${green:-\x1b[1;32m}"
red="${red:-\x1b[1;31m}"
yellow="${yellow:-\x1b[0;33m}"
NC="${NC:-\x1b[0m}"

function current_build {
    local exepath=$1
    [[ -e "$exepath" ]] || fail "Executable doesn't exist: $exepath"
    local filetype libs symbols
    filetype="$(file "$exepath")" || fail "Can't get file type for: $exepath"
    local names namesfmt
    declare -a names
    local is_release=1
    [[ "$filetype" == *debug* ]] && {
        names+=("debug")
        is_release=0
    }
    libs="$(ldd "$exepath" 2>/dev/null)"
    [[ "$libs" == *libasan* ]] && {
        names+=("sanitize")
        is_release=0
    }
    symbols="$(nm "$exepath" 2>/dev/null)"
    [[ "$symbols" == *__gcov* ]] && {
        names+=("coverage")
        is_release=0
    }
    ((is_release)) && names+=("release")
    { [[ "$filetype" == *stripped* ]] && [[ "$filetype" != *"not stripped"* ]]; } && {
        names+=("stripped")
    }
    printf -v namesfmt "%s " "${names[@]}"
    namesfmt="${namesfmt% *}"
    printf "%s\n" "$namesfmt"
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

function print_build {
    local exepath=$1 buildstr=$2 msg=$3
    [[ -n "$exepath" ]] || fail "No exepath given to print_build()!"
    [[ -n "$buildstr" ]] || buildstr="$(current_build "$exepath")"
    { [[ -n "$exepath" ]] && [[ -n "$buildstr" ]]; } || fail "No exepath/buildstr given to print_build()!"
    local exename="${exepath##*/}"
    ((no_colr)) && {
        # Plain:
        if ((do_binary)); then
            printf "%s: %s" "$exename" "$buildstr"
        else
            printf "%s" "$buildstr"
        fi
        [[ -n "$msg" ]] && printf " - %b" "$msg"
        printf "\n"
        return 0
    }
    # Colorized:
    if ((do_binary)); then
        printf "%s%s%s: %s%s%s" "$BLUE" "$exename" "$NC" "$yellow" "$buildstr" "$NC"
    else
        printf "%s%s%s" "$yellow" "$buildstr" "$NC"
    fi
    [[ -n "$msg" ]] && {
        printf " - %s%b%s" "$RED" "$msg" "$NC"
    }
    printf "\n"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Checks an executable's file type and the libraries linked into it,
    and prints the kind of build that made it. You can also pass a build name
    to see if the executable was built with that build target.

    Usage:
        $appscript [-h | -v]
        $appscript [-b] [-n] [-o] [-s] FILE [BUILD...]

    Options:
        BUILD         : One or more build names to check against the executable.
                        Can be a mix of: debug release sanitize
        FILE          : Executable to check.
                        If no args are given, the first executable in the
                        CWD is used.
        -b,--build    : Only show the build string, not the binary name.
        -h,--help     : Show this message.
        -n,--nocolor  : Don't use colors.
        -o,--or       : Match against any of the build names.
        -s,--status   : Print status messages when checking, otherwise this
                        script is silent.
        -v,--version  : Show $appname version and exit.

    When checking against build names, the exit code will be 0 for a match,
    and non-zero for a non-match. All build names must match unless --or is
    used.
    "
}


declare -a nonflags
do_or=0
do_binary=1
do_status=0
no_colr=0
[[ -t 1 ]] || no_colr=1

for arg; do
    case "$arg" in
        "-b" | "--build")
            do_binary=0
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-n" | "--nocolor")
            no_colr=1
            ;;
        "-o" | "--or")
            do_or=1
            ;;
        "-s" | "--status")
            do_status=1
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

{ (( $# > 0 )) ||  [[ -x "$binary" ]]; } ||  fail "No executable given, none found either."

no_args=0
if ((${#nonflags[@]})); then
    binary="${nonflags[0]}"
    nonflags=("${nonflags[@]:1}")
else
    nonflags[0]=$binary
    no_args=1
fi
{ [[ -n "$binary" ]] && ((${#nonflags[@]})); } || {
    no_args=1
}

[[ -e "$binary" ]] || fail "Executable doesn't exist: $binary"
binaryname="${binary##*/}"

if ((no_args)); then
    # Just print the detected build type.
    print_build "$binary"
else
    # Check the build against the user's arguments.
    current="$(current_build "$binary")"
    for buildname in "${nonflags[@]}"; do
        if [[ "$current" == *${buildname}* ]]; then
            # Got a match, return successful in --or mode.
            ((do_or)) && {
                ((do_status)) && print_build "$binary" "$current"
                exit 0
            }
        else
            # No match. This is a failure without --or.
            ((do_or)) || {
                ((do_status)) && print_build "$binary" "$current" "No match for '$buildname'."
                exit 1
            }
        fi
    done
    # In --or mode, none of the patterns matched, so this is a failure.
    ((do_or)) && {
        ((do_status)) && print_build "$binary" "$current" "No matches for '${nonflags[*]}'."
        exit 1
    }
    # Without --or, all of the patterns matched, so this is a success.
fi

((do_status)) && ((!no_args)) && print_build "$binary" "$current"
exit 0

