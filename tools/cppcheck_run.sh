#!/bin/bash

# Runs cppcheck with some defaults.
# -Christopher Welborn 07-27-2019
appname="ColrC - cppcheck runner"
appversion="0.1.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

colr_dir="$appdir/.."
test_dir="$colr_dir/test"
xml_dir="$colr_dir/cppcheck_report"
xml_file="$xml_dir/cppcheck-report.xml"
html_file="$xml_dir/index.html"
suppress_file="${appdir}/cppcheck.suppress.txt"
test_xml_dir="$test_dir/cppcheck_report"
test_xml_file="$test_xml_dir/cppcheck-report.xml"
test_html_file="$test_xml_dir/index.html"
test_suppress_file="${appdir}/cppcheck.suppress.test.txt"

# Command to list cppcheck errors with descriptions.
declare -a cppcheck_error_list=("python3" "$appdir/cppcheck_errors.py")

declare -A script_deps=(["cppcheck"]="cppcheck")
for script_dep in "${!script_deps[@]}"; do
    hash "$script_dep" &>/dev/null || {
        printf "\nMissing \`%s\` command.\n" "$script_dep" 1>&2
        printf "Install the \`%s\` package with your package manager.\n" "${script_deps[$script_dep]}" 1>&2
        exit 1
    }
done
shopt -s nullglob

declare -a colr_files=("$colr_dir"/colr.c "$colr_dir"/colr.h)
declare -a colr_tool_files=("$colr_dir"/colr_tool.c "$colr_dir"/colr_tool.h)
declare -a test_files=("$test_dir"/test_*.c "$test_dir"/test_*.h)

declare -a cppcheck_cmd=(
    "cppcheck"
    "--std=c11"
    "--enable=all"
    "--force"
    "--inline-suppr"
    "--error-exitcode=1"
    "-DDEBUG"
    "-DCOLR_DEBUG"
    "-DIS_C11"
    "-D__GNUC__"
    "-Ddbug=printf"
)

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

function list_errors {
    # List all error names/descs from cppcheck.
    "${cppcheck_error_list[@]}" "$@"
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript -l [-f pat]
        $appscript [-f pat] [-e pat] [-t | FILE...] [-- ARGS...]
        $appscript [-E] [-t | FILE...] [-- ARGS...]
        $appscript [-t | FILE...] (-r | -x) [-- ARGS...]
        $appscript [-t] -V

    Options:
        ARGS                  : Extra arguments for cppcheck.
        FILE                  : Zero or more files to check.
                                If none are given, the colr tool source is used.
        -E,--errors           : Add (error)|(warn) to the --filter pattern.
        -e pat,--exclude pat  : Exclude lines that match this pattern.
                                These work in the order they are given.
        -f pat,--filter pat   : Only show lines that match this pattern.
                                These work in the order they are given.
        -h,--help             : Show this message.
        -l,--errorlist        : List available error names/descs using:
                                ${cppcheck_error_list[*]}
        -r,--report           : Generate HTML report.
        -t,--test             : Run on test files.
        -V,--view             : View a previously generated HTML report.
        -v,--version          : Show $appname version and exit.
        -x,--xml              : Generate xml report in: $xml_dir
    "
}

function view_html {
    local index_file=$html_file
    ((do_test)) && index_file=$test_html_file
    [[ -e "$index_file" ]] || fail "HTML index file doesn't exist: $index_file"
    xdg-open "$index_file" &>/dev/null
}

declare -a user_files
declare -a extra_args

in_args=0
in_filter_arg=0
in_exclude_arg=0
do_errors=0
do_listerrors=0
do_report=0
do_test=0
do_view=0
do_xml=0
exclude_pat=""
filter_pat=""
filter_first=1

for arg; do
    case "$arg" in
        "--")
            in_args=1
            ;;
        "-E" | "--errors")
            do_errors=1
            ;;
        "-e" | "--exclude")
            in_exclude_arg=1
            in_filter_arg=0
            [[ -z "$filter_pat" ]] && filter_first=0
            ;;
        "-f" | "--filter")
            in_filter_arg=1
            in_exclude_arg=0
            [[ -z "$exclude_pat" ]] && filter_first=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-l" | "--errorlist")
            do_listerrors=1
            ;;
        "-r" | "--report")
            do_report=1
            ;;
        "-t" | "--test")
            do_test=1
            ;;
        "-V" | "--view")
            do_view=1
            ;;
        "-v" | "--version")
            echo -e "$appname v. $appversion\n"
            exit 0
            ;;
        "-x" | "--xml")
            do_xml=1
            ;;
        -*)
            if ((in_args)); then
                extra_args+=("$arg")
            elif ((in_exclude_arg)); then
                exclude_pat=$arg
                in_exclude_arg=0
            elif ((in_filter_arg)); then
                filter_pat=$arg
                in_filter_arg=0
            else
                fail_usage "Unknown flag argument: $arg"
            fi
            ;;
        *)
            if ((in_args)); then
                extra_args+=("$arg")
            elif ((in_exclude_arg)); then
                exclude_pat=$arg
                in_exclude_arg=0
            elif ((in_filter_arg)); then
                filter_pat=$arg
                in_filter_arg=0
            else
                user_files+=("$arg")
            fi
    esac
done

((do_listerrors)) && {
    list_errors "$filter_pat"
    exit
}
((do_view)) && {
    # Just view the cppcheck report, if it exists.
    view_html
    exit
}

((${#user_files[@]})) || {
    ((${#colr_files[@]})) || fail "No colr source files found!"
    if ((do_test)); then
        ((${#test_files[@]})) || fail "No test source files found!"
        user_files=("${colr_files[@]}" "${test_files[@]}")
    else
        ((${#colr_tool_files[@]})) || fail "No colr tool source files found!"
        user_files=("${colr_files[@]}" "${colr_tool_files[@]}")
    fi
}

if ((do_test)); then
    [[ -e "$test_suppress_file" ]] && cppcheck_cmd+=("--suppressions-list=$test_suppress_file")
else
    [[ -e "$suppress_file" ]] && cppcheck_cmd+=("--suppressions-list=$suppress_file")
fi

declare -a report_cmd=(
    "cppcheck-htmlreport"
    "--file=$xml_file"
    "--report-dir=$xml_dir"
    "--source-dir=$colr_dir"
    "--title=ColrC - CppCheck Report"
)

((do_test)) && report_cmd=(
    "cppcheck-htmlreport"
    "--file" "$test_xml_file"
    "--report-dir" "$test_xml_dir"
    "--source-dir" "$test_dir"
    "--title" "ColrC - Test - CppCheck Report"
)

((do_report)) && {
    # Generate a report.
    cppcheck_cmd+=("--quiet" "--xml")
    use_xml_dir=$xml_dir
    use_xml_file=$xml_file
    ((do_test)) && {
        use_xml_dir=$test_xml_dir
        use_xml_file=$test_xml_file
    }
    mkdir -p "$use_xml_dir" || fail "Failed to create report directory."
    printf "\nGenerating cppcheck report for:\n"
    for user_file in "${user_files[@]}"; do
        printf "    %s\n" "$(readlink -f "$user_file")"
    done
    echo "${cppcheck_cmd[@]}" "${extra_args[@]}"
    printf "\n"
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 1>"$use_xml_file" 2>&1
    [[ -e "$use_xml_file" ]] || fail "No xml file to generate report with: $use_xml_file"
    printf "\nGenerating cppcheck html...\n"
    echo "${report_cmd[@]}"
    "${report_cmd[@]}" || fail "Failed to generate HTML report."
    exit 0
}

# Regular run, with optional errors, filtering, exlcuding, or xml to stdout.
if ((do_errors)); then
    ((do_xml)) && fail "--errors cannot be used with --xml."
    ((do_report)) && fail "--errors cannot be used with --report."
    if [[ -n "$filter_pat" ]]; then
        if [[ "$filter_pat" == \(*\) ]]; then
            filter_pat="$filter_pat|(error)|(warn)"
        else
            filter_pat="($filter_pat)|(error)|(warn)"
        fi
    else
        filter_pat="(error)|(warn)"
    fi
fi
if ((do_xml)); then
    cppcheck_cmd+=("--quiet" "--xml")
    [[ -z "$exclude_pat" ]] || fail "--exclude cannot be used with --xml."
    [[ -z "$filter_pat" ]] || fail "--filter cannot be used with --xml."
fi
if [[ -n "$exclude_pat" ]] && [[ -n "$filter_pat" ]]; then
    if ((filter_first)); then
        # shellcheck disable=SC1001
        # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
        "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E "$filter_pat" | \grep -E -v "$exclude_pat"
    else
        # shellcheck disable=SC1001
        # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
        "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E -v "$exclude_pat" | \grep -E "$filter_pat"
    fi
elif [[ -n "$exclude_pat" ]]; then
    # shellcheck disable=SC1001
    # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E -v "$exclude_pat"
elif [[ -n "$filter_pat" ]]; then
    # shellcheck disable=SC1001
    # ..i know shellcheck, it's a regular 'g'. I need the actual grep command.
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1 | \grep -E "$filter_pat"
else
    "${cppcheck_cmd[@]}" "${extra_args[@]}" "${user_files[@]}" 2>&1
fi
exit
