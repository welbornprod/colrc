#!/bin/bash

# ...
# -Christopher Welborn 07-02-2019
appname="run_examples"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

declare -a binaries=($(find "$appdir" -maxdepth 1 -executable -type f ! -name "*.*"))

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

function is_in {
    # Returns a 0 exit status if $1 matches any of the other args.
    local needle=$1
    shift
    local haystack
    declare -a haystack=("$@")
    # Return false/1 if the haystack is empty.
    ((${#haystack[@]})) || return 1
    local possible
    for possible in "${haystack[@]}"; do
        [[ "$needle" == "$possible" ]] && return 0
    done
    return 1
}


function merge_patterns {
    # Print all arguments wrapped in parens, joined by '|'.
    local wrapped
    IFS=$'\n' declare -a wrapped=($(printf "(%s)|\n" "$@"))
    printf "%s" "${wrapped::-1}"
}

function pattern_matches {
    local pat=$1 s=$2
    # No pattern? Everything matches.
    [[ -n "$pat" ]] || return 0
    # No string? Everything matches.
    [[ -n "$s" ]] || return 0

    [[ "$s" =~ $pat ]] || return 1
    return 0
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-a | -s] [-m] [PATTERN...]
        $appscript [-a | -s] [-r exe] [PATTERN...] [-- ARGS...]

    Options:
        ARGS              : Arguments for the executable mentioned with --run.
        PATTERN           : Only run executables matching these regex/text patterns.
        -a,--all          : Run all examples, including the source code examples.
        -h,--help         : Show this message.
        -m,--memcheck     : Run through \`valgrind --tool=memcheck\`.
        -r exe,--run exe  : Use example binary as an argument for another executable,
                            like \`gdb\` or \`kdbg\`.
        -s,--source       : Use examples found in the source code.
                            This is a shortcut for:
                                \`../tools/snippet.py -x [PATTERN...]\`
        -v,--version      : Show $appname version and exit.
    "
}

function run_exe {
    local exename=$1
    shift
    local wrapper_cmd
    declare -a wrapper_cmd=("$@")

    [[ -n "$exename" ]] || fail "No executable given to \`run_exe\`!"
    [[ -x "$exename" ]] || fail "Not an executable: $exename"
    if ((${#wrapper_cmd[@]})); then
        printf "\nRunning %s %s...\n" "${wrapper_cmd[*]}" "$exename"
        "${wrapper_cmd[@]}" "$exename"
    else
        printf "\nRunning %s...\n" "$exename"
        "$exename"
    fi
}

((${#binaries})) || fail "No binaries built yet. Run \`make\`."

declare -a patterns exe_args
do_all=0
do_memcheck=0
do_source=0
in_exe_arg=0
in_exe_args=0
wrapper=""

for arg; do
    case "$arg" in
        "--")
            in_exe_args=1
            ;;
        "-a" | "--all")
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                do_all=1
                do_source=1
            fi
            ;;
        "-h" | "--help")
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                print_usage ""
                exit 0
            fi
            ;;
        "-m" | "--memcheck")
            do_memcheck=1
            ;;
        "-r" | "--run")
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                in_exe_arg=1
            fi
            ;;
        "-s" | "--source")
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                do_source=1
                do_all=0
            fi
            ;;
        "-v" | "--version")
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                echo -e "$appname v. $appversion\n"
            fi
            exit 0
            ;;
        -*)
            if ((in_exe_args)); then
                exe_args+=("$arg")
            else
                fail_usage "Unknown flag argument: $arg"
            fi
            ;;
        *)
            if ((in_exe_arg)); then
                wrapper=$arg
                in_exe_arg=0
            elif ((in_exe_args)); then
                exe_args+=("$arg")
            else
                patterns+=("$arg")
            fi
    esac
done
((do_memcheck)) && {
    wrapper="valgrind"
    exe_args=("--show-leak-kinds=all" "--track-origins=yes")
}
((do_source)) && {
    declare -a snippet_args=("--examples" "$(merge_patterns "${patterns[@]}")")
    [[ -n "$wrapper" ]] && {
        if ((do_memcheck)); then
            snippet_args+=("--memcheck")
        else
            snippet_args+=("--run" "$wrapper")
        fi
    }
    ../tools/snippet.py "${snippet_args[@]}"
    ((do_all)) || exit $?
}

let count=0
let errs=0
declare -a matched_patterns unmatched_patterns
for binaryname in "${binaries[@]}"; do
    run_it=0
    if ((${#patterns[@]})); then
        for pattern in "${patterns[@]}"; do
            if pattern_matches "$pattern" "$binaryname"; then
                [[ "${matched_patterns[*]}" == *"$pattern"* ]] || matched_patterns+=("$pattern")
                run_it=1
                break
            fi
        done
    else
        run_it=1
    fi
    ((run_it)) && {
        run_exe "$binaryname"  "$wrapper" "${exe_args[@]}" || let errs+=1
        let count+=1
    }
done
# Collect unmatched patterns, for reporting.
for pattern in "${patterns[@]}"; do
    is_in "$pattern" "${matched_patterns[@]}" || unmatched_patterns+=("$pattern")
done

if ((count)); then
    plural="examples"
    ((count == 1)) && plural="example"
    printf "\nRan %s %s" "$count" "$plural"
    if ((${#patterns[@]})); then
        printf " matching: %s\n" "${matched_patterns[*]}"
        ((${#unmatched_patterns[@]})) && printf "No matches for: %s\n" "${unmatched_patterns[*]}"
    else
        printf "\n"
    fi
else
    if ((${#patterns[@]})); then
        echo_err "\nNo examples matching: ${patterns[*]}"
    else
        echo_err "\nNo executables found!"
    fi
fi
exit $errs
