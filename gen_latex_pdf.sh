#!/bin/bash

# Generates a PDF from Doxygen's LaTeX output.
# -Christopher Welborn 07-08-2019
appname="gen_latex_pdf"
appversion="0.0.1"
apppath="$(readlink -f "${BASH_SOURCE[0]}")"
appscript="${apppath##*/}"
appdir="${apppath%/*}"

colr_anim_run="$appdir/colr_anim_run.py"
latex_dir="${appdir}/docs/latex"
doxy_tex="${latex_dir}/refman.tex"
ref_pdf="${latex_dir}/refman.pdf"
doc_pdf="${appdir}/docs/ColrC-manual.pdf"
doxy_config="${appdir}/Doxyfile_latex"

shopt -s nullglob


function clean_ref_pdf {
    # These files must be removed before generating the reference pdf.
    local exts
    declare -a exts=(
        .ps .dvi .aux .toc .idx .ind .ilg .log .out .brf .blg .bbl
    )
    local ext files
    declare -a files
    for ext in "${exts[@]}"; do
        files=("$latex_dir"/*"$ext")
        ((${#files[@]})) || continue
        rm "$latex_dir"/*"$ext"
    done
    [[ -e "${latex_dir}/refman.pdf" ]] && rm "${latex_dir}/refman.pdf"
}

function clean_ref_tex {
    # Remove Doxygen-generated latex files.
    local exts
    declare -a exts=(.tex .md5 .pdf .sty)
    local ext files
    declare -a files
    for ext in "${exts[@]}"; do
        files=("$latex_dir"/*"$ext")
        ((${#files[@]})) || continue
        rm "$latex_dir"/*"$ext"
    done

    [[ -e "${latex_dir}/Makefile" ]] && {
        rm "${latex_dir}/Makefile"
    }
}

function clean_pdf {
    # Removes the ColrC-manual pdf.
    [[ -e "$doc_pdf" ]] && rm "$doc_pdf"
}

function colr_anim {
    # Run a command through colr_anim_run.py, with a custom message.
    local msg=$1
    shift
    if [[ "$msg" == "-" ]]; then
        "$colr_anim_run" -- "$@"
    else
        "$colr_anim_run" -a -e -m "$msg" -- "$@"
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

function gen_ref_pdf {
    # Generate the reference pdf based on Doxygen's latex output.
    # Must have these packages installed:
    #      doxygen-latex, texlive-lang-cyrillic, and texlive-fonts-extra
    [[ -e "$doxy_tex" ]] || {
        printf "\nDoxygen latex docs must be built first...\n" 1>&2
        doxygen "$doxy_config"
    }
    needs_built_from "$ref_pdf" "$doxy_tex" || {
        ((do_ref)) && {
            printf "Reference pdf up to date: %s\n" "$ref_pdf"
            printf "                Based on: %s\n" "$doxy_tex"
        }
        return 0
    }

    # log files and indexes need to be removed before running this.
    clean_ref_pdf

    cd "$latex_dir" || fail "Unable to cd into latex dir: $latex_dir"

    declare -a pdf_args=("-halt-on-error" "-shell-escape")

    colr_anim "Generating ref pdf (1)" pdflatex "${pdf_args[@]}" refman || fail "pdflatex failed!"
    colr_anim "Generating index (1)" makeindex refman.idx || fail "makeindex failed!"
    colr_anim "Generating ref pdf (2)" pdflatex "${pdf_args[@]}" refman || fail "pdflatex failed!"
    local latex_count=3
    while grep -E -s 'Rerun (LaTeX|to get cross-references right)' refman.log &>/dev/null && ((latex_count < 11))
      do
        colr_anim "Rerunning ref pdf ($latex_count)" pdflatex "${pdf_args[@]}" refman || fail "pdflatex re-run failed!"
        let latex_count+=1
      done
    colr_anim "Generating index (2)" makeindex refman.idx || fail "makeindex failed!"
    colr_anim "Generating ref pdf ($latex_count)" pdflatex "${pdf_args[@]}" refman || fail "pdflatex failed!"

    cd "$appdir" || return 1
}

function gen_pdf {
    # This simply copies the reference pdf to the docs directory with a new name.
    needs_built_from "$doc_pdf" "$ref_pdf" || {
        printf "ColrC manual up to date: %s\n" "$doc_pdf"
        printf "               Based on: %s\n" "$ref_pdf"
        return 0
    }
    gen_ref_pdf || fail "Reference pdf generation failed!"
    [[ -e "$ref_pdf" ]] || fail "Reference pdf not found: $ref_pdf"
    cp "$ref_pdf" "$doc_pdf" || fail "Failed to create ColrC pdf: $doc_pdf"

}

function graceful {
    # Like `fail()`, but happier.
    echo -e "$@"
    exit 0
}

function needs_built_from {
    # Returns a 0 exit status if file1 is newer than file2, or if either
    # file is missing.
    # This is used to cause file2 to be freshly built when needed.
    local pdf="$1" ref="$2"
    if [[ -e "$pdf" ]] && [[ -e "$ref" ]]; then
        # Both exist, check the times.
        [[ "$ref" -nt "$pdf" ]] && return 0
    elif [[ ! -e "$pdf" ]]; then
        # File 1 is missing, build it.
        return 0
    elif [[ ! -e "$ref" ]]; then
        # File 2, the reference is missing.
        return 1
    fi
}

function print_usage {
    # Show usage reason if first arg is available.
    [[ -n "$1" ]] && echo_err "\n$1\n"

    echo "$appname v. $appversion

    Usage:
        $appscript -h | -v
        $appscript [-C] [-c]
        $appscript [-r]

    Options:
        -C,--cleandoxy  : Clean Doxygen-generated files.
        -c,--clean      : Clean reference pdf files and the main pdf file.
        -h,--help       : Show this message.
        -r,--reference  : Generate the reference pdf only.
        -v,--version    : Show $appname version and exit.
    "
}

declare -a nonflags
do_ref=0
do_clean_doxy=0
do_clean_pdf=0

for arg; do
    case "$arg" in
        "-Cc" | "-cC" | "-CC" | "-cc")
            do_clean_doxy=1
            do_clean_pdf=1
            ;;
        "-C" | "--cleandoxy")
            do_clean_doxy=1
            ;;
        "-c" | "--clean")
            do_clean_pdf=1
            ;;
        "-h" | "--help")
            print_usage ""
            exit 0
            ;;
        "-r" | "--reference")
            do_ref=1
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

[[ -e "$doxy_config" ]] || fail "Can't find the Doxygen config file: $doxy_config"

if ((do_clean_pdf)); then
    clean_pdf
    clean_ref_pdf
    ((do_clean_doxy)) && clean_ref_tex
elif ((do_clean_doxy)); then
    ((do_clean_pdf)) && {
        clean_pdf
        clean_ref_pdf
    }
    clean_ref_tex
elif ((do_ref)); then
    gen_ref_pdf
else
    gen_pdf
fi
exit
