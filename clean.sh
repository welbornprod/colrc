#!/bin/bash

# ...Removes binary and object files, preparing for a fresh compile.
# -Christopher Welborn 06-29-2019
appname="clean"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"

default_docs_dir="docs"
default_docs_file="${default_docs_dir}/html/index.html"

function clean_compiled() {
    printf "Removing compiled files:\n"
    if [[ -n "$binary" ]]; then
        if [[ -e "$binary" ]]; then
            rm "$binary" || echo_err "Cannot remove binary: $binary"
            clean_msg 1 "$binary"
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
        rm "$objfile" || {
            echo_err "Cannot remove object: $objfile"
            continue
        }
        clean_msg 1 "$objfile"
    done
}

function clean_docs() {
    local docsdir=$1 docsmainfile=$2

    if [[ -n "$docsdir" ]] && [[ -e "$docsmainfile" ]]; then
        if rm -r "${docsdir:?}"/*; then
            printf "Docs cleaned:\n    %s\n" "$docsdir/*"
        fi
    else
        printf "Docs already clean:\n    %s\n" "$docsdir/*"
    fi
}

function clean_msg() {
    # Print a message about whether a file has been "cleaned".
    local wascleaned=$1 filepath=$2
    if [[ "$wascleaned" =~ (1)|(yes)|(true) ]]; then
        printf "            Cleaned: %s\n" "$filepath"
    else
        printf "    Already cleaned: %s\n" "$filepath" 1>&2
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
        $appscript -d [DOC_DIR] [DOC_INDEX]

    Options:
        BINARY        : Optional executable to delete.
        DOC_DIR       : Directory for Doxygen docs.
                        Default: $default_docs_dir
        DOC_INDEX     : Main index.html file for docs.
                        Default: $default_docs_file
        -d,--docs     : Clean the docs dir.
                        This will not clean the binary or object files.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}


declare -a objfiles
binary=""
do_docs=0
doc_dir=$default_docs_dir
doc_index=$default_docs_file

for arg; do
    case "$arg" in
        "-d" | "--docs")
            do_docs=1
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
            if ((!do_docs)) && [[ -z "$binary" ]]; then
                binary="$arg"
            else
                objfiles+=("$arg")
            fi
    esac
done

if ((do_docs)); then
    case ${#objfiles[@]} in
        2)
            doc_dir="${objfiles[0]}"
            doc_index="${objfiles[1]}"
            ;;
        1)
            doc_dir="${objfiles[0]}"
            doc_index="$doc_dir/html/index.html"
            ;;
        *)
            fail_usage "Too many arguments."
    esac
    clean_docs "$doc_dir" "$doc_index"
else
    clean_compiled
fi
exit
