#!/bin/bash

# ...Removes binary and object files, preparing for a fresh compile.
# -Christopher Welborn 06-29-2019
appname="clean"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}/.."

default_docs_dir="${appdir}/docs"
default_html_dir="${appdir}/${default_docs_dir}/html"
default_man_dir="${appdir}/${default_docs_dir}/man"


function clean_compiled {
    printf "Removing compiled files:\n"
    if [[ -n "$binary" ]]; then
        if [[ -e "$binary" ]]; then
            rm "$binary" || echo_err "Cannot remove binary: $binary"
            clean_msg 1 "$binary"
        elif [[ "$binary" == "-" ]]; then
            # Binary is ignored on purpose.
            :
        else
            clean_msg 0 "$binary"
        fi
    else
        printf "No binary specified for removal.\n"
    fi

    if ((${#objfiles} == 0)); then
        # Default, delete all object files in CWD.
        while read -r objfile; do
            objfiles+=("$objfile")
        done < <(find . -maxdepth 1 -type f -name "*.o")
    fi
    ((${#objfiles})) || {
        objfiles=($(find . -maxdepth 1 -type f -name "*.c"))
        for objfile in "${objfiles[@]}"; do
            objfile="${objfile%.*}.o"
            objfile="${objfile##*/}"
            clean_msg 0 "$objfile"
        done
        exit 0
    }

    for objfile in "${objfiles[@]}"; do
        [[ -e "$objfile" ]] || {
            clean_msg 0 "$objfile"
            continue
        }
        rm "$objfile" || {
            echo_err "Cannot remove object: $objfile"
            continue
        }
        clean_msg 1 "$objfile"
    done
}

function clean_docs {
    local html_dir=$1 man_dir=$2
    if [[ -n "$html_dir" ]] && [[ -e "$html_dir" ]]; then
        rm -r "$html_dir" || fail "Can't remove HTML docs dir: $html_dir"
        printf "HTML docs cleaned:\n    %s\n" "$html_dir/*"
    else
        printf "HTML docs already clean:\n    %s\n" "$html_dir/*"
    fi
    if [[ -n "$man_dir" ]] && [[ -e "$man_dir" ]]; then
        rm -r "$man_dir" || fail "Can't remove Man docs dir: $man_dir"
        printf "Man docs cleaned:\n    %s\n" "$man_dir/*"
    else
        printf "Man docs already clean:\n    %s\n" "$man_dir/*"
    fi
}

function clean_manual {
    # Print a description ($1), and remove all other arguments (rm ${@:1})
    local desc=$1
    [[ -n "$desc" ]] || fail "No arguments provided to clean_manual()!"
    shift
    local files filepath cleaned missing
    declare -a files=("$@")
    ((${#files[@]})) || fail "No file paths provided to clean_manual()!"
    declare -a cleaned missing cleaneddirs
    let errs=0
    for filepath in "${files[@]}"; do
        if [[ -d "$filepath" ]]; then
            rm -r "$filepath" || let errs+=1
            cleaneddirs+=("$filepath")
        elif [[ -n "$filepath" ]] && [[ -e "$filepath" ]]; then
            rm "$filepath" || let errs+=1
            cleaned+=("$filepath")
        else
            missing+=("$filepath")
        fi
    done
    ((${#cleaned[@]})) && {
        printf "%s files cleaned:\n" "$desc"
        printf "    %s\n" "${cleaned[@]}"
    }
    ((${#cleaneddirs[@]})) && {
        printf "%s dirs cleaned:\n" "$desc"
        printf "    %s\n" "${cleaneddirs[@]}"
    }
    ((${#missing[@]})) && {
        printf "%s already cleaned:\n" "$desc"
        printf "    %s\n" "${missing[@]}"
    }
    return $errs
}

function clean_msg {
    # Print a message about whether a file has been "cleaned".
    local wascleaned=$1 filepath=$2
    if [[ "$wascleaned" =~ (1)|(yes)|(true) ]]; then
        printf "             Cleaned: %s\n" "$filepath"
    else
        printf "     Already cleaned: %s\n" "$filepath" 1>&2
    fi
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
        $appscript BINARY
        $appscript -d [DOC_DIR] [HTML_DIR] [MAN_DIR]
        $appscript -m DESC FILE...

    Options:
        BINARY        : Optional executable to delete.
        DESC          : Description of files that are being manually removed.
        DOC_DIR       : Directory for Doxygen docs.
                        Default: $default_docs_dir
        HTML_DIR      : Directory for Doxygen html docs.
                        Default: $default_html_dir
        MAN_DIR       : Directory for Doxygen man docs.
                        Default: $default_man_dir
        FILE          : One or more files to remove. Can be anything.
                        This is used with --manual.
        -d,--docs     : Clean the docs dir.
                        This will not clean the binary or object files.
        -h,--help     : Show this message.
        -m,--manual   : Just \`rm\` all arguments, without a custom message.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a objfiles
binary=""
do_docs=0
do_manual=0
doc_dir=$default_docs_dir
doc_html_dir=$default_html_dir
doc_man_dir=$default_man_dir

for arg; do
    case "$arg" in
        "-d" | "--docs")
            do_docs=1
            do_manual=0
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-m" | "--manual")
            do_manual=1
            do_docs=0
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        -*)
            if [[ "$arg" == "-" ]]; then
                binary="$arg"
            else
                fail_usage "Unknown flag argument: $arg"
            fi
            ;;
        *)
            if ((!do_docs && !do_manual)) && [[ -z "$binary" ]]; then
                binary="$arg"
            else
                objfiles+=("$arg")
            fi
    esac
done

if ((do_manual)); then
    ((${#objfiles[@]})) || fail "Need description and file paths. Got: nothing"
    ((${#objfiles[@]} > 1)) || fail "No file paths provided. Got: ${objfiles[*]}"
    clean_manual "${objfiles[@]}"
elif ((do_docs)); then
    case ${#objfiles[@]} in
        3)
            doc_dir="${objfiles[0]}"
            doc_html_dir="${objfiles[1]}"
            doc_man_dir="${objfiles[2]}"
            ;;
        2)
            doc_dir="${objfiles[0]}"
            doc_html_dir="${objfiles[1]}"
            doc_man_dir="${doc_dir}/man"
            ;;
        1)
            doc_dir="${objfiles[0]}"
            doc_html_dir="${doc_dir}/html"
            doc_man_dir="${doc_dir}/man"
            ;;
        *)
            fail_usage "Too many arguments."
    esac
    clean_docs "$doc_html_dir" "$doc_man_dir"
else
    clean_compiled
fi
exit
