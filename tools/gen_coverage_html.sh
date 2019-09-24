#!/bin/bash

# Uses lcov to generate an html report for a previously compiled binary.
# -Christopher Welborn 06-29-2019
appname="Colr Coverage Generator"
appversion="0.0.3"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
# appdir="${apppath%/*}"
declare -A script_deps=(
    ["lcov"]="lcov"
    ["genhtml"]="lcov"
    ["ggcov"]="ggcov"
)

for script_dep in "${!script_deps[@]}"; do
    hash "$script_dep" &>/dev/null || {
        printf "\nMissing \`%s\` command.\n" "$script_dep" 1>&2
        printf "Install the \`%s\` package with your package manager.\n" "${script_deps[$script_dep]}" 1>&2
        exit 1
    }
done

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

function generate_coverage {
    make_coverage
    printf "\nRunning executable for coverage...\n"
    $exe_path "${flagargs[@]}" "${nonflags[@]}" 1>/dev/null || fail "Could not run the executable: $exe_path"
    move_cov_files "$cov_dir"

    generate_lcov
}

function generate_html {
    local title="Coverage"
    [[ "$exe_path" == test* ]] && title="Test Coverage"
    generate_coverage
    printf "\nGenerating html report...\n"
    declare -a info_files=("$cov_dir"/*.info)
    ((${#info_files[@]})) || fail "No .info files were generated."
    declare -a genhtml_args=(
        --output-directory "$html_dir"
        --function-coverage
        --frames
        --title "ColrC - $title"
        --css-file "$css_file"
    )
    ((do_branch)) && genhtml_args+=("--branch-coverage")
    genhtml "${genhtml_args[@]}" "$lcov_name"
}

function generate_lcov {
    printf "\nGenerating lcov report info in: %s\n" "$cov_dir"
    lcov \
        --capture --directory "$cov_dir" --output-file "$lcov_name" \
        --rc lcov_branch_coverage=1 || fail "Failed to generate lcov info."
}

function make_coverage {
    local gcnofile
    declare -a gcnofiles
    while read -r gcnofile; do
        gcnofiles+=("$gcnofile")
    done < <(find "$input_dir" -maxdepth 1 -type f -name "*.gcno")

    ((${#gcnofiles[@]})) && {
        printf "\nCoverage-enabled executable was already compiled.\n"
        return 0
    }
    printf "\nCompiling executable to produce .gcno files...\n"
    make coveragecompile
}

function move_cov_files {
    printf "\nMoving coverage files to: %s\n" "$cov_dir"
    process_cov_files "mv" "$cov_dir"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript -c | -s | -V
        $appscript [-b] [-l] EXE COVERAGE_DIR [ARGS...]

    Options:
        ARGS          : Extra arguments for the executable when running.
        EXE           : Executable to run to generate the coverage files.
        COVERAGE_DIR  : Directory for output coverage files.
        -b,--branch   : Include branch info in html output.
        -c,--clean    : Remove coverage files from the current directory.
        -h,--help     : Show this message.
        -l,--lcov     : Only generate lcov info, not html.
        -s,--summary  : View a summary of a previously generated report.
        -V,--view     : View previously generated reports in a browser.
        -v,--version  : Show $appname version and exit.
    "
}

function process_cov_files {
    local cmd=$1
    shift
    declare -a cmdargs=("$@")
    printf "\nProcessing coverage files in: %s\n" "$input_dir"
    printf "    Command: %s <file> %s\n" "$cmd" "${cmdargs[*]}"
    local have_files=0 covext covfile missing=0
    declare -a exts=(".gcno" ".gcda" ".gcov")
    for covext in "${exts[@]}"; do
        while read -r covfile; do
            have_files=1
            "$cmd" "$covfile" "${cmdargs[@]}" && printf "    %s\n" "$covfile"
        done < <(find "$input_dir" -maxdepth 1 -type f -name "*$covext")
        ((have_files)) || {
            let missing+=1
            printf "    Missing coverage type: *%s\n" "$covext"
        }
    done
    ((missing == ${#exts[@]})) && {
        echo_err "No coverage files found: ${exts[*]}"
        return 1
    }
    return 0
}

function remove_cov_files {
    printf "\nRemoving coverage files:\n"
    process_cov_files "rm"
    local errs=0
    (($#)) && {
        local arg
        for arg; do
            remove_ext_files "$arg" || let errs+=1
        done
    }
    return $errs
}

function remove_ext_files {
    local fileext=$1 objfile have_files=0
    [[ -n "$fileext" ]] || fail "No file extension given to remove_ext_files!"
    while read -r objfile; do
        have_files=1
        rm "$objfile" && printf "    %s\n" "$objfile"
    done < <(find "$cov_dir" -maxdepth 1 -type f -name "$fileext")
    ((have_files)) || {
        printf "    Missing file ext: %s\n" "$fileext"
        return 1
    }
    return 0
}

function view_html {
    local index_file="${html_dir}/index.html"
    [[ -e "$index_file" ]] || fail "HTML index file doesn't exist: $index_file"
    xdg-open "$index_file" &>/dev/null
}

function view_summary {
    [[ -e "$lcov_name" ]] || fail "Coverage info file not found: $lcov_name"
    lcov --rc lcov_branch_coverage=1 --summary "$lcov_name"
}

(( $# > 1 )) || fail_usage "No arguments!"

declare -a flagargs nonflags
exe_path=""
cov_dir=""
do_branch=0
do_clean=0
do_lcov=0
do_summary=0
do_view=0

for arg; do
    case "$arg" in
        "-b" | "--branch")
            do_branch=1
            ;;
        "-c" | "--clean")
            do_clean=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-l" | "--lcov")
            do_lcov=1
            ;;
        "-s" | "--summary")
            do_summary=1
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
exe_path="$(readlink -f "$exe_path")"
input_dir="${exe_path%/*}"
exe_name="${exe_path##*/}"
lcov_name="${cov_dir}/${exe_name}.info"
html_dir="${cov_dir}/html"
css_file="${cov_dir}/gcov-colrc.css"

((do_view || do_summary)) && {
    # Ensure the html directory exists for these commands.
    [[ -d $html_dir ]] || {
        ((do_view)) && { fail "Html report directory doesn't exist: $html_dir"; }
        mkdir "$html_dir" || fail "Can't create coverage html dir: $html_dir"
        printf "Created %s.\n" "$html_dir"
    }
}
if ((do_clean)); then
    remove_cov_files "*.o"
elif ((do_lcov)); then
    generate_coverage
elif ((do_view)); then
    view_html
elif ((do_summary)); then
    view_summary
else
    generate_html
fi
exit
