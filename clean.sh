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
    local docsdir=$1 docsmainfile=$2

    if [[ -n "$docsdir" ]] && [[ -e "$docsmainfile" ]]; then
        if rm -r "${docsdir:?}"/*; then
            printf "Docs cleaned:\n    %s\n" "$docsdir/*"
        fi
    else
        printf "Docs already clean:\n    %s\n" "$docsdir/*"
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
        $appscript -d [DOC_DIR] [DOC_INDEX]
        $appscript -m DESC FILE...

    Options:
        BINARY        : Optional executable to delete.
        DESC          : Description of files that are being manually removed.
        DOC_DIR       : Directory for Doxygen docs.
                        Default: $default_docs_dir
        DOC_INDEX     : Main index.html file for docs.
                        Default: $default_docs_file
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
doc_index=$default_docs_file

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
