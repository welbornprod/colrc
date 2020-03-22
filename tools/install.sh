#!/bin/bash

# Installer for the ColrC tool and libcolr.
# It will let you choose an installation path, either explicitly or by
# choosing from a directory in your $PATH. It will not overwrite existing files
# unless you tell it to.
#
# There isn't much to install here. It's just one executable file, with a
# fairly unique name.
# -Christopher Welborn 02-23-2020
appname="ColrC Installer"
appversion="0.1.0"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

colrc_dir="$(readlink -f "$appdir/..")"
installable="$colrc_dir/colrc"
installable_lib="$colrc_dir/libcolr.so"
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

function find_libcolr {
    # Find an installed colrc already in $PATH.
    local IFS=$'\n' usepath
    declare -a paths
    local path
    while read -r path; do
        paths+=("$path")
    done < <(get_gcc_lib_dirs)
    local path trylibcolr
    for path in "${paths[@]}"; do
        trylibcolr="$path/libcolr.so"
        [[ -e "$trylibcolr" ]] && {
            printf "%s" "$trylibcolr"
            return 0
        }
    done
    return 1
}

function get_gcc_lib_dirs {
    local use_std="c"
    local reportedpath dirpath
    while read -r reportedpath; do
        dirpath="$(readlink -f "$reportedpath")"
        [[ -e "$dirpath" ]] || continue
        printf "%s\n" "$dirpath"
    done < <(gcc "-x$use_std" -E -v /dev/null -o /dev/null 2>&1 | \
        grep LIBRARY_PATH | \
            cut -d '=' -f2 | \
                tr ':' '\n')
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

function has_installable_lib {
    # Ensure that all required files are present.
    [[ -e "$installable_lib" ]] || {
        echo_err "Missing installable file: $installable_lib"
        echo_err "\nIt looks like \`libcolr.so\` hasn't been built yet."
        echo_err "Have you tried \`make lib\`?"
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

    install_file "$installable" "$target"
}

function install_file {
    local installable=$1 target=$2
    { [[ -n "$installable" ]] && [[ -n "$target" ]]; }  || fail "Missing arguments for install_file()!"
    # Handle existing installations.
    if [[ -e "$target" ]]; then
        ((do_overwrite)) || {
            echo_err "Already installed: $target"
            return 1
        }
        # Need to remove the existing one, in case one is a link.
        remove_file "$target" || fail "Failed to remove existing: $target"
    fi
    # Symlink or copy?
    local cmdtype="Copy" cmdtypeplural="Copied" targetdir="${target%/*}"
    declare -a cmd
    [[ -w "$targetdir" ]] || cmd=("sudo")
    if ((do_symlink)); then
        cmdtype="Symlink"
        cmdtypeplural="Symlinked"
        cmd+=("ln" "-s")
    else
        cmd+=("cp")
    fi
    [[ -w "$targetdir" ]] || {
        printf "No write access for: %s\n" "$targetdir"
        printf "Using: %s %s %s\n" "${cmd[*]}" "$installable" "$target"
    }
    if "${cmd[@]}" "$installable" "$target"; then
        printf "\n%9s: %s\n%9s: %s\n" "$cmdtypeplural" "$installable" "To" "$target"
    else
        echo_err "$cmdtype command failed!"
        return 1
    fi
    return 0
}

function install_libcolr {
    local other_libcolr
    other_libcolr="$(find_libcolr)" && {
        printf "\nAnother \`libcolr\` was found: %s\n" "$other_libcolr"
        local maybe_install_path="${other_libcolr%/*}"
        confirm "Install to $maybe_install_path?" && {
            install_path="$maybe_install_path"
            printf "Installing: %s\n        To: %s\n" "$installable_lib" "$install_path"
            ((do_overwrite)) || {
                printf "\nThis will overwrite the previous installation: %s\n" "$other_libcolr"
                confirm "Overwrite previous installation" || fail "User cancelled."
                do_overwrite=1
            }
        }
    }

    if [[ -z "$install_path" ]]; then
        # Choose an installation path.
        install_path="$(select_path_lib)" || fail "No path chosen!"
    fi
    # Could've landed here from command-line arguments. Check for existence.
    [[ -d "$install_path" ]] || fail "Installation path does not exist: $install_path"

    ((do_overwrite)) || {
        printf "Installing: %s\n        To: %s\n" "$installable_lib" "$install_path"
        confirm "Is this okay?" || fail "User cancelled."
    }
    local target="$install_path/libcolr.so"

    install_file "$installable_lib" "$target"
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

function remove_file {
    local filepath=$1
    { [[ -n "$filepath" ]] && [[ -e "$filepath" ]]; } || fail "Invalid file path for remove_file()!"
    declare -a rm_cmd=("rm")
    [[ -w "${filepath%/*}" ]] || {
        rm_cmd=("sudo" "rm")
        printf "No write access for: %s\n" "${filepath%/*}"
        printf "Using: %s %s\n" "${rm_cmd[*]}" "$filepath"
    }
    "${rm_cmd[@]}" "$filepath"
}

function select_path {
    local IFS=$'\n' usepath
    declare -a paths
    paths=($(echo "$PATH" | tr ':' '\n' | sort -u))
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

function select_path_lib {
    local IFS=$'\n' usepath
    declare -a paths
    local path
    while read -r path; do
        paths+=("$path")
    done < <(get_gcc_lib_dirs | sort -u)
    ((${#paths[@]})) || fail "Cannot find GCC lib paths!"
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
    uninstall_file "$foundcolrc"
}

function uninstall_file {
    local filepath=$1
    confirm "Remove $filepath?" || fail "User cancelled."
    remove_file "$filepath"
}

function uninstall_libcolr {
    # Uninstall libcolor from a path in gcc search paths.
    local foundlibcolr
    foundlibcolr="$(find_libcolr)" || fail "Cannot find \`libcolr\` in GCC's search path!\nIt's probably not installed."
    uninstall_file "$foundlibcolr"
}

declare -a nonflags
do_overwrite=0
do_symlink=0
do_uninstall=0
do_lib=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-L" | "--lib")
            do_lib=1
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

if ((do_lib)); then
    # libcolr
    if ((do_uninstall)); then
        uninstall_libcolr
    else
        has_installable_lib || fail "\nCannot continue."
        install_libcolr
    fi
else
    # colrc
    if ((do_uninstall)); then
        uninstall_colrc
    else
        has_installable || fail "\nCannot continue."
        install_colrc
    fi
fi
