#!/bin/bash

# ...Removes binary and object files, preparing for a fresh compile.
# -Christopher Welborn 06-29-2019
appname="clean"
appversion="0.1.0"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="$(readlink -f "${apppath%/*}/..")"

default_docs_dir="${appdir}/docs"
default_html_dir="${default_docs_dir}/html"
default_man_dir="${default_docs_dir}/man"

# Some color constants.
red='\e[0;31m'
cyan='\e[0;36m'
green='\e[0;32m'
# No Color, normal/reset.
NC='\e[0m'

function clean_compiled {
    printf_status "Removing compiled files:\n"
    if [[ -n "$binary" ]]; then
        if [[ -e "$binary" ]]; then
            safe_rm "$binary" || echo_err "Cannot remove binary: $binary"
            clean_msg 1 "$binary"
        elif [[ "$binary" == "-" ]]; then
            # Binary is ignored on purpose.
            :
        else
            clean_msg 0 "$binary"
        fi
    else
        printf_status "No binary specified for removal.\n"
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
        safe_rm "$objfile" || {
            echo_err "Cannot remove object: $objfile"
            continue
        }
        clean_msg 1 "$objfile"
    done
}

function clean_dir {
    local dirpath=$1
    if [[ -n "$dirpath" ]] && [[ -e "$dirpath" ]]; then
        ((do_confirm)) && printf "Cleaning dir: %s\n" "$dirpath"
        safe_rm -r "${dirpath:?}"/* || fail "Can't remove dir: $dirpath"
    else
        printf_status "Directory already clean:\n    %s\n" "$dirpath"
    fi
    clean_msg 1 "$dirpath"
}

function clean_dirs {
    local arg count errs=0
    declare -a args
    args=("$@")
    for arg in "${args[@]}"; do
        count="$(count_filepaths "$arg"/*)"
        ((count < 1)) && {
            clean_msg 0 "$arg"
            continue
        }
        clean_dir "$arg" || let errs+=1
    done
    return $errs
}

function clean_docs {
    local html_dir=$1 man_dir=$2
    if [[ -n "$html_dir" ]] && [[ -e "$html_dir" ]]; then
        ((do_confirm)) && printf "Cleaning dir: %s\n" "$html_dir"
        safe_rm -r "$html_dir" || fail "Can't remove HTML docs dir: $html_dir"
        printf_status "HTML docs cleaned:\n    %s\n" "$html_dir/*"
    else
        printf_status "HTML docs already clean:\n    %s\n" "$html_dir/*"
    fi
    if [[ -n "$man_dir" ]] && [[ -e "$man_dir" ]]; then
        safe_rm -r "$man_dir" || fail "Can't remove Man docs dir: $man_dir"
        printf_status "Man docs cleaned:\n    %s\n" "$man_dir/*"
    else
        printf_status "Man docs already clean:\n    %s\n" "$man_dir/*"
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
            ((do_confirm)) && printf "Cleaning dir: %s\n" "$filepath"
            safe_rm -r "$filepath" || let errs+=1
            cleaneddirs+=("$filepath")
        elif [[ -n "$filepath" ]] && [[ -e "$filepath" ]]; then
            ((do_confirm)) && printf "Cleaning file: %s\n" "$filepath"
            safe_rm "$filepath" || let errs+=1
            cleaned+=("$filepath")
        else
            missing+=("$filepath")
        fi
    done
    ((${#cleaned[@]})) && {
        printf_status "%s files cleaned:\n" "$desc"
        printf_status "    %s\n" "${cleaned[@]}"
    }
    ((${#cleaneddirs[@]})) && {
        printf_status "%s dirs cleaned:\n" "$desc"
        printf_status "    %s\n" "${cleaneddirs[@]}"
    }
    ((${#missing[@]})) && {
        printf_status "%s already cleaned:\n" "$desc"
        printf_status "    %s\n" "${missing[@]}"
    }
    return $errs
}

function clean_msg {
    # Print a message about whether a file has been "cleaned".
    local wascleaned=$1 filepath=$2
    if [[ "$wascleaned" =~ (1)|(yes)|(true) ]]; then
        printf_status "             Cleaned: %s\n" "$filepath"
    else
        printf_status "     Already cleaned: %s\n" "$filepath"
    fi
}

function confirm {
    # Get input from the user for a yes/no question.
    local msg="${1:-Confirm?}"
    echo -en "${cyan}${msg}${NC} (${green}y${NC}/${red}N${NC}): "
    local answer
    read -r answer
    case "$answer" in
        y*|Y* )
            return 0
            ;;
        * )
            return 1
            ;;
    esac
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

function count_filepaths {
    # Count all arguments that are valid file paths.
    local arg
    declare -a args
    args=("$@")
    local pathcnt=0
    for arg in "${args[@]}"; do
        [[ -e "$arg" ]] && let pathcnt+=1
    done
    printf "%s" "$pathcnt"
}

function print_confirm_filepaths {
    # Print all arguments that are valid file paths.
    # This is for the `safe_rm` command when confirmations are enabled.
    local arg
    declare -a args
    args=("$@")
    ((${#args})) || {
        echo_err "No filepaths provided to \`print_confirm_filepaths\`!"
        return 1
    }
    for arg in "${args[@]}"; do
        [[ -e "$arg" ]] || continue
        printf "    %s\n" "$arg"
    done
    return 0
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-c] BINARY
        $appscript [-c] -D DIR...
        $appscript [-c] -d [DOC_DIR] [HTML_DIR] [MAN_DIR]
        $appscript [-c] -m DESC FILE...

    Options:
        BINARY        : Optional executable to delete.
        DESC          : Description of files that are being manually removed.
        DIR           : Arbitrary directory to remove files in.
        DOC_DIR       : Directory for Doxygen docs.
                        Default: $default_docs_dir
        HTML_DIR      : Directory for Doxygen html docs.
                        Default: $default_html_dir
        MAN_DIR       : Directory for Doxygen man docs.
                        Default: $default_man_dir
        FILE          : One or more files to remove. Can be anything.
                        This is used with --manual.
        -c,--confirm  : Interactively confirm every \`rm\` command.
        -d,--docs     : Clean the docs dir.
                        This will not clean the binary or object files.
        -h,--help     : Show this message.
        -m,--manual   : Just \`rm\` all arguments, without a custom message.
        -q,--quiet    : Only print errors.
        -v,--version  : Show $appname version and exit.
    "
}

function printf_status {
    ((do_quiet)) && return 0
    # shellcheck disable=SC2059
    # ...I know shellcheck. It's a wrapper, I meant to do it.
    printf "$@"
}

function safe_rm {
    ((do_confirm)) && {
        local pathcnt msg
        pathcnt="$(count_filepaths "$@")"
        msg="$*"
        ((pathcnt == 0)) && {
            printf "Nothing to remove: rm %s\n" "$*"
            return 1
        }
        ((pathcnt > 1)) && {
            printf "\nThese files are about to be removed:\n"
            print_confirm_filepaths "$@"
            msg="[$pathcnt paths...]"
        }
        confirm "rm $msg" || return 1
    }
    rm "$@"
}

declare -a objfiles
binary=""
do_confirm=0
do_dir=0
do_docs=0
do_manual=0
do_quiet=0
doc_dir=$default_docs_dir
doc_html_dir=$default_html_dir
doc_man_dir=$default_man_dir
for arg; do
    case "$arg" in
        "-c" | "--confirm")
            do_confirm=1
            ;;
        "-D" | "--dir")
            do_dir=1
            do_docs=0
            do_manual=0
            ;;
        "-d" | "--docs")
            do_dir=0
            do_docs=1
            do_manual=0
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-m" | "--manual")
            do_manual=1
            do_dir=0
            do_docs=0
            ;;
        "-q" | "--quiet")
            do_quiet=1
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
            if ((do_dir || do_docs || do_manual)); then
                objfiles+=("$arg")
            elif [[ -z "$binary" ]]; then
                binary="$arg"
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
elif ((do_dir)); then
    ((${#objfiles[@]})) || fail "Need description and file paths. Got: nothing"
    clean_dirs "${objfiles[@]}"
else
    clean_compiled
fi
exit
