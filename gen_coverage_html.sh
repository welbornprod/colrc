#!/bin/bash

# Uses lcov to generate an html report for a previously compiled binary.
# -Christopher Welborn 06-29-2019
appname="Colr Coverage Generator"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"

shopt -s nullglob

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


function generate_html() {
    printf "\nRunning executable for coverage...\n"
    $exe_path "${flagargs[@]}" "${nonflags[@]}" 1>/dev/null

    printf "\nMoving coverage files...\n"
    local have_files=0 covext covfile
    for covext in .gcno .gcda .gcov; do
        while read -r covfile; do
            have_files=1
            mv "$covfile" "$cov_dir" && printf "    %s\n" "$covfile"
        done < <(find "$input_dir" -maxdepth 1 -type f -name "*$covext")
        ((have_files)) || {
            printf "No coverage files: *%s\n" "$$covext"
            break;
        }
    done

    printf "\nGenerating lcov report info...\n"
    lcov --capture --directory "$cov_dir" --output-file "$lcov_name" || fail "Failed to generate lcov info."
    declare -a info_files=("$cov_dir"/*.info)
    ((${#info_files[@]})) || fail "No .info files were generated."

    printf "\nGenerating html report...\n"
    genhtml --output-directory "$html_dir" "${info_files[@]}"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript EXE COVERAGE_DIR [ARGS...]

    Options:
        ARGS          : Extra arguments for the executable when running.
        EXE           : Executable to run to generate the coverage files.
        COVERAGE_DIR  : Directory for output coverage files.
        -h,--help     : Show this message.
        -v,--version  : Show $appname version and exit.
    "
}

function view_html() {
    local index_file="${html_dir}/index.html"
    [[ -e "$index_file" ]] || fail "HTML index file doesn't exist: $index_file"
    google-chrome "$index_file" &>/dev/null
}

(( $# > 1 )) || fail_usage "No arguments!"

declare -a flagargs nonflags
exe_path=""
cov_dir=""
do_view=0

for arg; do
    case "$arg" in
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        "-V" | "--view")
            do_view=1
            ;;
        -*)
            flagargs+=("$arg")
            ;;
        *)
            if [[ -z "$exe_path" ]]; then
                exe_path=$arg
            elif [[ -z "$cov_dir" ]]; then
                cov_dir=$arg
            else
                nonflags+=("$arg")
            fi
    esac
done
[[ -e "$exe_path" ]] || fail "Executable doesn't exist: $exe_path"
[[ -n "$cov_dir" ]] || fail "Missing coverage directory argument!"
[[ -d "$cov_dir" ]] || {
    ((do_view)) && { fail "Coverage directory doesn't exist: $cov_dir"; }
    mkdir "$cov_dir" || fail "Can't create coverage dir: $cov_dir"
    printf "Created %s.\n" "$cov_dir"
}
input_dir="${exe_path%/*}"
exe_name="${exe_path##*/}"
lcov_name="${cov_dir}/${exe_name}.info"
html_dir="${cov_dir}/html"

[[ -d $html_dir ]] || {
    ((do_view)) && { fail "Html report directory doesn't exist: $html_dir"; }
    mkdir "$html_dir" || fail "Can't create coverage html dir: $html_dir"
    printf "Created %s.\n" "$html_dir"
}
if ((do_view)); then
    view_html
else
    generate_html
fi
