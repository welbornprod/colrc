#!/bin/bash

# Simply copies the last-built colr-tool executable to `colrc-<version>`,
# and creates a symlink in ~/.local/bin if the dir exists, and there's not
# already a `colrc` symlink.
# -Christopher Welborn 02-23-2020
appname="ColrC Tool Copier"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="$(readlink -f "${apppath%/*}")"

colrc_dir="${appdir}/.."
colrc_exe="${colrc_dir}/colrc"
[[ -e "$colrc_dir" ]] || {
    printf "\nMissing ColrC directory: %s\n" "$colrc_dir" 1>&2
    exit 1
}
colrc_ln_dir="$HOME/.local/bin"
colrc_ln="$colrc_ln_dir/colrc"

function cp_new_exe {
    local new_name new_path
    new_name="$(get_new_name)" || return 1
    new_path="$colrc_dir/$new_name"
    cp "$colrc_exe" "$new_path" || fail "Unable to create $new_path!"
    print_label "Copied executable" "$colrc_exe"
    print_label "To" "$new_path"
    ln_colrc "$new_path"
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

function get_new_name {
    # Prints `colrc-<current_version>`
    local vers
    vers="$(bash "$appdir/get_version.sh")" || fail "Unable to determine ColrC version!"
    printf "colrc-%s" "$vers"
}

function ln_colrc {
    local src=$1
    [[ -n "$src" ]] || fail "No path provided to \`ln_colrc\`!"
    [[ -e "$src" ]] || fail "Source path does not exist!: $src"
    [[ -e "$colrc_ln_dir" ]] || fail "Directory does not exist: $colrc_ln_dir"
    [[ -e "$colrc_ln" ]] && {
        if ((do_overwrite)); then
            print_label "Removing old symlink" "$colrc_ln"
            rm "$colrc_ln" || fail "Failed to remove old symlink: $colrc_ln"
        else
            print_label "Symlink already exists" "$colrc_ln" 1>&2
            return 1
        fi
    }
    ln -s "$src" "$colrc_ln" || fail "Failed to create symlink: $colrc_ln"
    print_label "Created symlink" "$src"
    # Don't use `readlink` when printing this.
    print_label "As" "$colrc_ln" "-"
}

function print_label {
    local lbl=$1 path=$2 nolink=$3
    [[ -n "$lbl" ]] || fail "Missing label for \`print_label\`, got nothing!"
    [[ -n "$path" ]] || fail "Missing value for \`print_label\`, got ${lbl:-nothing}!"
    # Fix relative paths, if they exist.
    { [[ -z "$nolink" ]] && [[ -e "$path" ]]; } && path="$(readlink -f "$path")"
    printf "%24s: %s\n" "$lbl" "$path"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-o]

    Options:
        -h,--help       : Show this message.
        -o,--overwrite  : Overwrite existing symlinks.
        -v,--version    : Show $appname version and exit.
    "
}

[[ -e "$colrc_exe" ]] || fail "ColrC executable not found: $colrc_exe"

declare -a nonflags
do_overwrite=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-o" | "--overwrite")
            do_overwrite=1
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

cp_new_exe || exit 1

exit 0
