#!/bin/bash

# Look for unused stuff in ColrC.
# -Christopher Welborn 08-29-2019
appname="unused"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
declare -a cppcheck_cmd=("bash" "${appdir}/cppcheck_run.sh")

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

function get_unused_names {
    # Filter names provided from cppcheck, to see if they are really unused.
    local name grepline namepat
    local has_h=0 has_c=0 has_test=0 has_example=0 has_tool=0
    while read -r name; do
        commentpat="${name}\\(\\)"
        namepat="${name}(\\(| =|,? \\\\)"
        has_h=0
        has_c=0
        has_test=0
        has_example=0
        has_tool=0
        while read -r grepline; do
            [[ "$grepline" =~ $commentpat ]] && continue;
            [[ "$grepline" =~ test_.+\.c ]] && { let has_test+=1; continue; }
            [[ "$grepline" =~ colr_tool\. ]] && { let has_tool+=1; continue; }
            [[ "$grepline" =~ _example\.c ]] && { let has_example+=1; continue; }
            [[ "$grepline" =~ colr\.c ]] && { let has_c+=1; continue; }
            [[ "$grepline" =~ colr\.h ]] && { let has_h+=1; continue; }
        done < <(grep --color=never -E -r "$namepat" --include="*.c" --include="*.h" ./)
        # If it's in the colr tool, it's used.
        ((!do_all && has_tool)) && continue;
        { ((has_c > 1)) || (( has_h > 1)); } && {
            ((do_all)) || continue;
        }
        printf "%-30s : colr.c: %2d colr.h: %2d colr_tool: %2d tests: %2d examples: %2d " \
            "$name" "$has_c" "$has_h" "$has_tool" "$has_test" "$has_example"
        ((has_test || has_example || has_tool)) || {
            printf "!\n"
            continue
        }
        ((has_test || has_tool)) || {
            printf "*\n"
            continue
        }
        ((has_test)) || {
            printf "?\n"
            continue
        }
        ((has_tool)) || {
            printf "%s\n" "-"
            continue
        }
        printf "\n"
    done < <(get_unused_names_cppcheck)
}
function get_unused_names_cppcheck {
    # Print all unused names reported by cppcheck.
    local line name
    while read -r line; do
        name="${line#*\'}"
        name="${name%%\'*}"
        printf "%s\n" "$name"
    done < <("${cppcheck_cmd[@]}" | grep 'is never used')
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-a]
    Options:
        -a,--all      : Show counts for all \"unused\" names reported by
                        cppcheck.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a nonflags
do_all=0

for arg; do
    case "$arg" in
        "-a" | "--all")
            do_all=1
            ;;
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

get_unused_names
