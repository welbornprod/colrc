#!/bin/bash

# Installer for the ColrC tool.
# It will let you choose an installation path, either explicitly or by
# choosing from a directory in your $PATH. It will not overwrite existing files
# unless you tell it to.
#
# There isn't much to install here. It's just one executable file, with a
# fairly unique name.
# -Christopher Welborn 02-23-2020
appname="ColrC Installer"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

colrc_dir="$(readlink -f "$appdir/..")"
installable="$colrc_dir/colrc"
# This is set later, by arguments or path selection (by menu).
install_path=""

# Some color constants.
red='\e[0;31m'
cyan='\e[0;36m'
green='\e[0;32m'
# No Color, normal/reset.
NC='\e[0m'

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

function find_colrc_exe {
    # Find an installed colrc already in $PATH.
    local IFS=$'\n' usepath
    declare -a paths
    paths=($(echo "$PATH" | tr ':' '\n'))
    local path trycolrc
    for path in "${paths[@]}"; do
        trycolrc="$path/colrc"
        [[ -e "$trycolrc" ]] && {
            printf "%s" "$trycolrc"
            return 0
        }
    done
    return 1
}

function has_installable {
    # Ensure that all required files are present.
    [[ -e "$installable" ]] || {
        echo_err "Missing installable file: $installable"
        echo_err "\nIt looks like \`colrc\` hasn't been built yet."
        echo_err "Have you tried \`make release\`?"
        return 1
    }

    return 0
}

function install_colrc {
    # Install the colrc executable.
    local other_colrc
    other_colrc="$(find_colrc_exe)" && {
        printf "\nAnother \`colrc\` was found: %s\n" "$other_colrc"
        local maybe_install_path="${other_colrc%/*}"
        confirm "Install to $maybe_install_path?" && {
            install_path="$maybe_install_path"
            printf "Installing: %s\n        To: %s\n" "$installable" "$install_path"
            ((do_overwrite)) || {
                printf "\nThis will overwrite the previous installation: %s\n" "$other_colrc"
                confirm "Overwrite previous installation" || fail "User cancelled."
                do_overwrite=1
            }
        }
    }

    if [[ -z "$install_path" ]]; then
        # Choose an installation path.
        install_path="$(select_path)" || fail "No path chosen!"
    fi
    # Could've landed here from command-line arguments. Check for existence.
    [[ -d "$install_path" ]] || fail "Installation path does not exist: $install_path"

    ((do_overwrite)) || {
        printf "Installing: %s\n        To: %s\n" "$installable" "$install_path"
        confirm "Is this okay?" || fail "User cancelled."
    }
    local target="$install_path/colrc"

    # Handle existing installations.
    if [[ -e "$target" ]]; then
        ((do_overwrite)) || {
            echo_err "Already installed: $target"
            return 1
        }
        # Need to remove the existing one, in case one is a link.
        rm "$target" || fail "Failed to remove existing: $target"
    fi
    # Symlink or copy?
    if ((do_symlink)); then
        if ln -s "$installable" "$target"; then
            printf "\nSymlinked: %s\n       To: %s\n" "$target" "$installable"
        else
            echo_err "Symlink command failed!"
            return 1
        fi
    else
        if cp "$installable" "$target"; then
            printf "\nCopied: %s\n    To: %s\n" "$installable" "$target"
        else
            echo_err "Copy command failed!"
            return 1
        fi
    fi
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-l] [-o | -u] [DIR]

    Options:
        DIR             : Directory to install \`colrc\`.
                          Default: <chosen interactively>
        -h,--help       : Show this message.
        -l,--link       : Use a symlink instead of copying.
        -o,--overwrite  : Overwrite existing colrc files.
        -u,--uninstall  : Uninstall an existing \`colrc\` executable.
        -v,--version    : Show $appname version and exit.
    "
}

function select_path {
    local IFS=$'\n' usepath
    declare -a paths
    paths=($(echo "$PATH" | tr ':' '\n'))
    PS3="Choose the installation path: "
    select usepath in "${paths[@]}"; do
        case "${#usepath}" in
            0 )
                [[ "$REPLY" =~ ^[Qq]([Uu][Ii][Tt])? ]] && return 1
                echo_err "Invalid choice, press q to quit."
                ;;
            * )
                printf "%s" "$usepath"
                return 0
                ;;
        esac
    done
    return 1
}

function uninstall_colrc {
    # Uninstall the colrc executable from a path in $PATH.
    local foundcolrc
    foundcolrc="$(find_colrc_exe)" || fail "Cannot find \`colrc\` in \$PATH!\nIt's probably not installed."
    confirm "Remove $foundcolrc?" || fail "User cancelled."
    rm "$foundcolrc"
}

has_installable || fail "\nCannot continue."

declare -a nonflags
do_overwrite=0
do_symlink=0
do_uninstall=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-l" | "--link")
            do_symlink=1
            ;;
        "-o" | "--overwrite")
            do_overwrite=1
            ;;
        "-u" | "--uninstall")
            do_uninstall=1
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
            [[ -z "$install_path" ]] && install_path="$arg"
    esac
done
if ((do_uninstall)); then
    uninstall_colrc
else
    install_colrc
fi
