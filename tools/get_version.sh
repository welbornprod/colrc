#!/bin/bash

# Used as the FILE_VERSION_FILTER for Doxygen in Doxyfile_common.
# It simply prints the current ColrC version based on the version
# found in colr.h
# -Christopher Welborn 08-11-2019
appname="ColrC - Version"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"
colrh_file="${appdir}/../colr.h"
[[ -e "$colrh_file" ]] || {
    printf "\nCan't find colr.h!" 1>&2
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

function get_colrc_version {
    local versionstr
    versionstr="$(awk '/COLR_VERSION/ { print $3; nextfile }' "$colrh_file")"
    if ((do_projnum)); then
        printf "%s\n" "$versionstr"
    else
        printf "%s\n" "${versionstr//\"/}"
    fi
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-p]

    Options:
        -h,--help        : Show this message.
        -p,--projectnum  : Print 'PROJECT_NUMBER=<version>' for Doxygen.
        -v,--version     : Show $appname version and exit.
    "
}


declare -a nonflags
do_projnum=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-p" | "--projectnum")
            do_projnum=1
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

((do_projnum)) && printf "PROJECT_NUMBER="
get_colrc_version
exit
