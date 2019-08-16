#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" run_snippet.py
    Compiles a small ColrC snippet and runs it. This is for small testing
    and debugging purposes.

    Example Usage:
        ./tools/snippet.py 'char* s = colr(fore(BLUE), "Okay."); print(s);'

    To run examples found in the source code:
        ./tools/snippet.py -E

    -Christopher Welborn 07-04-2019
"""

import os
import re
import subprocess
import stat
import sys
import tempfile

try:
    from colr import (
        Colr as C,
        auto_disable as colr_auto_disable,
        docopt,
    )
    from easysettings import load_json_settings
    from printdebug import DebugColrPrinter
    from pygments import highlight
    from pygments.lexers import get_lexer_by_name
    from pygments.formatters import Terminal256Formatter
except ImportError as ex:
    print(f'\nMissing third-party library: {ex.name}', file=sys.stderr)
    print('It is installable with `pip`.', file=sys.stderr)
    sys.exit(1)

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug

pyg_lexer = get_lexer_by_name('c')
pyg_fmter = Terminal256Formatter(bg='dark', style='monokai')

colr_auto_disable()

NAME = 'ColrC - Snippet Runner'
VERSION = '0.2.3'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

CONFIGFILE = os.path.join(SCRIPTDIR, 'snippet.json')
config = load_json_settings(
    CONFIGFILE,
    default={
        'last_snippet': None,
        'last_binary': None,
        'last_c_file': None,
    }
)
TMPPREFIX = NAME.replace(' ', '').lower()
TMPDIR = tempfile.gettempdir()

EDITOR = os.environ.get('EDITOR', None)
if EDITOR:
    EDITOR_DESC = f'set to: {EDITOR}'
else:
    EDITOR = 'vim'
    EDITOR_DESC = f'not set, using: {EDITOR}'

COLR_DIR = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
COLRC_FILE = os.path.join(COLR_DIR, 'colr.c')
COLRH_FILE = os.path.join(COLR_DIR, 'colr.h')
DBUGH_FILE = os.path.join(COLR_DIR, 'dbug.h')
EXAMPLES_SRC = (COLRC_FILE, COLRH_FILE)

os.chdir(COLR_DIR)

MACROS = {
    'print': {
        'define': '#define print(s) printf("%s\\n", s)',
        'desc': 'Wrapper for print("%s\\n", s).',
    },
    'printferr': {
        'define': '#define printferr(...) fprintf(stderr, __VA_ARGS__)',
        'desc': 'Wrapper for fprintf(stderr, ...).',
    },
    'printerr': {
        'define': '#define printerr(s) fprintf(stderr, "%s\\n", s)',
        'desc': 'Wrapper for fprintf(stderr, "%s\\n", s).',
    },
    'print_repr': {
        'define': '#define print_repr(x) printf("%s\\n", colr_repr(x))',
        'desc': 'Wrapper for printf("%s\\n", colr_repr(x)).',
    },
}

USAGE_MACROS = '\n'.join(
    f'{name:>27}  : {MACROS[name]["desc"]}'
    for name in sorted(MACROS)
)

SANITIZE_ARGS = (
    '-fno-omit-frame-pointer',
    '-fstack-protector-strong',
    '-fsanitize=address',
    '-fsanitize=leak',
    '-fsanitize=undefined',
)

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-D] -c
        {SCRIPT} [-D] (-L | -N) [PATTERN]
        {SCRIPT} [-D] [-n] [-q] [-w] -V
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe] -b
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] -x [PATTERN] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [CODE] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-f file...] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-w] (-e [CODE] | -l) [-- ARGS...]
        {SCRIPT} [-D] -E [CODE] [-- ARGS...]
        {SCRIPT} [-D] -E [-f file...] [-- ARGS...]
        {SCRIPT} [-D] -E [-w] (-e [CODE] | -l) [-- ARGS...]

    Options:
        ARGS                 : Extra arguments for the compiler.
        CODE                 : Code to compile. It is auto-wrapped in a main()
                               function if no main() signature is found.
                               "colr.h" and "dbug.h" are included unless include
                               lines for them are found.
                               When used with -e, the editor is started with
                               this as it's content.
                               Default: stdin
        PATTERN              : Only use examples with a leading comment
                               that matches this text/regex pattern.
        -b,--lastbinary      : Re-run the last binary that was compiled.
        -c,--clean           : Clean {TMPDIR} files, even though they will be
                               cleaned when the OS reboots.
        -D,--debug           : Show some debug info while running.
        -E,--preprocessor    : Run through gcc's preprocessor and print the
                               output.
        -e,--editlast        : Edit the last snippet in $EDITOR and run it.
                               $EDITOR is {EDITOR_DESC}
                               If the CODE argument is given, the editor
                               it will replace the contents of the last snippet.
        -f name,--file name  : Read file to get snippet to compile.
        -h,--help            : Show this help message.
        -L,--listexamples    : List example code snippets in the source.
        -l,--last            : Re-run the last snippet.
        -m,--memcheck        : Run the snippet through `valgrind`.
        -N,--listnames       : List example snippet names from the source.
        -n,--name            : Print the resulting binary name, for further
                               testing.
        -q,--quiet           : Don't print any status messages.
        -r exe,--run exe     : Run a program on the compiled binary, like
                               `gdb` or `kdbg`.
        -s,--sanitize        : Use -fsanitize compiler arguments.
        -V,--viewlast        : View the last snippet that was compiled.
        -v,--version         : Show version.
        -w,--wrapped         : Use the "wrapped" version, which is the resulting
                               `.c` file for snippets.
        -x,--examples        : Use source examples as the snippets.

    Predefined Macros:
{USAGE_MACROS}
"""  # noqa (ignore long lines)


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    global status
    if argd['--quiet']:
        status = noop
    if argd['--sanitize']:
        argd['ARGS'].extend(SANITIZE_ARGS)
    if argd['--clean']:
        return clean_tmp()
    elif argd['--listexamples'] or argd['--listnames']:
        pat = try_repat(argd['PATTERN'])
        return list_examples(name_pat=pat, names_only=argd['--listnames'])
    elif argd['--examples']:
        pat = try_repat(argd['PATTERN'])
        return run_examples(
            pat=pat,
            exe=argd['--run'],
            compiler_args=argd['ARGS'],
            memcheck=argd['--memcheck'],
            quiet=argd['--quiet'],
        )
    elif argd['--lastbinary']:
        if not config['last_binary']:
            raise InvalidArg('no "last binary" found.')
        return run_compiled_exe(
            config['last_binary'],
            exe=argd['--run'],
            src_file=config['last_c_file'],
            show_name=argd['--name'],
            memcheck=argd['--memcheck'],
            quiet=argd['--quiet'],
        )
    elif argd['--viewlast']:
        if argd['--wrapped']:
            if not config['last_c_file']:
                raise InvalidArg('no "last file" found.')
            return view_snippet(
                filepath=config['last_c_file'],
                show_name=argd['--name'],
                quiet=argd['--quiet'],
            )
        else:
            if not config['last_snippet']:
                raise InvalidArg('no "last snippet" found.')
            return view_snippet(
                text=config['last_snippet'],
                show_name=argd['--name'],
                quiet=argd['--quiet'],
            )

    if argd['--file']:
        snippets = [
            read_file(s)
            for s in argd['--file']
        ]
    if argd['--editlast']:
        if argd['--wrapped']:
            if not config['last_c_file']:
                raise InvalidArg('no "last file" found.')
            snippet = edit_snippet(filepath=config['last_c_file'])
        else:
            if not (argd['CODE'] or config['last_snippet']):
                raise InvalidArg('no "last snippet" found, no code given.')
            snippet = edit_snippet(text=argd['CODE'] or config['last_snippet'])
        if not snippet:
            raise UserCancelled()
        snippets = [snippet]
    elif argd['--last']:
        if not config['last_snippet']:
            raise InvalidArg('no "last snippet" found.')
        snippets = [Snippet(config['last_snippet'], name='last-snippet')]
    else:
        snippets = [
            Snippet(argd['CODE'], name='cmdline-snippet') or read_stdin()
        ]

    if argd['--preprocessor']:
        return preprocess_snippets(snippets, compiler_args=argd['ARGS'])

    return run_snippets(
        snippets,
        exe=argd['--run'],
        show_name=argd['--name'],
        compiler_args=argd['ARGS'],
        memcheck=argd['--memcheck'],
        quiet=argd['--quiet'],
    )


def clean_objects(filepaths):
    debug('Removing temporary object files...')
    for filepath in filepaths:
        if os.path.exists(filepath):
            os.remove(filepath)


def clean_tmp():
    tmpfiles = [
        os.path.join(TMPDIR, s)
        for s in os.listdir(TMPDIR)
        if s.startswith(TMPPREFIX)
    ]
    tmplen = len(tmpfiles)
    plural = 'file' if tmplen == 1 else 'files'
    debug(f'Found {tmplen} {plural} in {TMPDIR}.')
    for filepath in tmpfiles:
        os.remove(filepath)
        debug(f'Removed: {filepath}', align=True)

    tmplen = len(tmpfiles)
    plural = 'file' if tmplen == 1 else 'files'
    tmpfmt = C(tmplen, 'blue', style='bright')
    status(f'Cleaned {tmpfmt} temporary {plural} in: {TMPDIR}')
    return 0 if tmpfiles else 1


def edited_code_trim(lines):
    """ Remove comment lines, and look for markers like '// cancel'.
        Returns possibly compilable code on success, or None if the code
        was empty or "cancelled".

        Arguments:
            lines  : An iterable of code lines (like a list or file object).

        Returns a list of code lines on success, or None on error/cancellation.
    """
    # Not using a list comprehension, so I can look for "markers".
    trimmed = []
    cancel_markers = set(('//cancel', '/*cancel', '/*\ncancel'))
    in_block_comment = False
    for line in lines:
        stripped = line.lstrip()
        if in_block_comment:
            if '*/' in stripped:
                in_block_comment = False
            debug(line, align=True)
            continue
        elif stripped.startswith('/*'):
            in_block_comment = True
            debug('Starting block comment:')
            debug(line, align=True)
            continue
        # A comment line, look for markers.
        if stripped.startswith('//'):
            if stripped.replace(' ', '').lower() in cancel_markers:
                debug(f'Found cancel marker: {line}')
                return None
            debug('Skipping comment:')
            debug(line, align=True)
            continue
        # Usable code.
        trimmed.append(line)

    # Remove leading blank lines.
    while trimmed and (not trimmed[0].strip()):
        trimmed.pop(0)

    if trimmed and (trimmed[0].lower().startswith('cancel')):
        # Just wrote 'cancel'/'CANCEL' at the top of the file.
        debug(f'Found plain cancel marker: {trimmed[0]!r}')
        return None
    return trimmed or None


def edit_snippet(filepath=None, text=None):
    if not (filepath or text):
        raise EditError('Developer Error: Missing file path or initial text!')
    marker = f'// {NAME} - Snippet Editing'
    header = '\n'.join((
        marker,
        '',
        '/*  If this file is empty, or all lines are commented out with',
        '    single-line comments, the process is cancelled.',
        '    You may also write "cancel" at the top of the file, or insert a',
        '    single/block comment with the first word as "cancel" anywhere in',
        '    the file to cancel compilation.',
        '*/\n',
    ))
    if text:
        fd, filepath = temp_file(extra_prefix='last_snippet')
        os.write(fd, header.encode())
        os.write(fd, text.encode().strip())
        os.close(fd)
        name = 'edited-snippet'
    elif filepath:
        prepend_to_file(filepath, header, cond_first_line=marker)
        name = filepath

    try:
        ret = os.system(' '.join((EDITOR, filepath)))
    except Exception as ex:
        raise EditError(f'Failed to edit last snippet: {ex}')
    else:
        if ret != 0:
            raise EditError(f'Editor ({EDITOR}) returned non-zero!')

    with open(filepath, 'r') as f:
        codelines = edited_code_trim(f)
    if not codelines:
        raise UserCancelled()

    return Snippet(''.join(codelines), name=name)


def find_src_examples():
    """ Look for \\examplecode tags in the source comments,
        and return a dict of {filename: [snippet, ...]}
    """
    # Future-proofing for-loop.
    examples = {
        s: find_src_file_examples(s)
        for s in EXAMPLES_SRC
    }
    return {k: v for k, v in examples.items() if v}


def find_src_file_examples(filepath):
    """ Look for \\examplecode tags in the file comments,
        and return a list of code snippets.
    """
    snippets = []
    current_snippet = None
    with open(filepath, 'r') as f:
        for line in f:
            linestrip = line.lstrip()
            if linestrip.startswith('\\endexamplecode'):
                snippets.append(
                    Snippet(''.join(current_snippet), name=current_snippet[0])
                )
                current_snippet = None
            elif linestrip.startswith('\\examplecode') and ('.c' in linestrip):
                if linestrip.startswith('\\examplecodefor{'):
                    _, _, rest = linestrip.partition('{')
                    rest, _, _ = rest.partition(',')
                    linestrip = f'Example code for {rest.strip()}\n'
                else:
                    # No name for this example.
                    linestrip = 'Example code for an unknown snippet\n'
                current_snippet = [f'// {linestrip}']
                tab_cnt = space_cnt = 0
            elif current_snippet is not None:
                if len(current_snippet) == 1:
                    # First line. Get indent.
                    testline = line
                    while testline.startswith('\t'):
                        tab_cnt += 1
                        testline = testline[1:]
                    testline = line
                    while testline.startswith(' '):
                        space_cnt += 1
                        testline = testline[1:]
                if line.startswith('\t'):
                    trimmed = line[tab_cnt:]
                elif line.startswith('    '):
                    trimmed = line[space_cnt:]
                else:
                    trimmed = line
                current_snippet.append(trimmed)
    return snippets


def format_leader(code):
    """ Format the first line in a code snippet, trying to find it's
        name or identifier.
    """
    firstline = code.split('\n')[0]
    firsttrim = firstline.lstrip()
    if firsttrim.startswith('//'):
        firsttrim = firsttrim.lstrip('/').lstrip()
    else:
        firsttrim = 'Uncommented/unknown snippet'
    if firsttrim.startswith('Example code for '):
        name = firsttrim.replace('Example code for ', '')
        desc = firsttrim.replace(name, '').strip()
        fmted = C(' ').join(
            C(desc, 'blue'),
            C(name, 'blue', style='bright'),
        )
    else:
        fmted = C(firsttrim, 'blue')
    return fmted


def get_gcc_cmd(
        input_files, output_file=None, user_args=None, preprocess=False):
    """ Get the cmd needed to run gcc on a file (without -c or -o). """
    c_files = [s for s in input_files if s.endswith('.c')]
    cmd = ['gcc']
    if preprocess:
        cmd.append('-E')
    elif c_files:
        cmd.append('-c')
    cmd.extend(input_files)
    if output_file:
        cmd.extend(('-o', output_file))
    cmd.append(f'-iquote{COLR_DIR}')
    cmd.extend(get_make_flags())
    cmd.extend(user_args or [])
    return cmd


def get_make_flags(user_args=None):
    """ Get gcc flags from a `make` dry run, and return them. """
    ignore_flags = {'-c', '-o'}
    flags = set()
    for line in iter_make_output(user_args=user_args):
        if (not line.strip()) or (not line.startswith('gcc')):
            continue
        flags.update(
            arg
            for arg in line.split()
            if arg.startswith('-') and (arg not in ignore_flags)
        )
    debug('Flags from make: {}'.format(' '.join(flags)))
    return flags


def get_obj_files():
    return [
        s
        for s in os.listdir('.')
        if s.startswith(TMPPREFIX) and s.endswith('.o')
    ]


def highlight_snippet(s):
    """ Highlight a C code snippet using pygments. """
    return highlight(s, pyg_lexer, pyg_fmter).strip()


def iter_make_output(user_args=None):
    """ Run `make [user_args...] -B -n` and yield the decoded output lines.
        Possibly raises subprocess.CalledProcessError.
        Arguments:
            user_args  : Extra arguments for make, or None.
    """
    make_cmd = ['make']
    make_cmd.extend(user_args or [])
    make_cmd.extend(['-B', '-n'])
    yield from iter_output(make_cmd)


def iter_output(cmd, stderr=subprocess.PIPE):
    """ Run `cmd [user_args...]` and yield the decoded output lines.
        Possibly raises subprocess.CalledProcessError.
        Arguments:
            cmd        : Popen command to run.
    """
    if isinstance(cmd, str):
        cmds = [cmd]
    else:
        cmds = list(cmd)
    debug('Running: {}'.format(' '.join(cmds)))
    yield from (
        subprocess.check_output(
            cmds,
            stderr=stderr
        ).decode().strip().splitlines()
    )


def list_examples(name_pat=None, names_only=False):
    """ List all example snippets found in the source. """
    snippetinfo = find_src_examples()
    if not snippetinfo:
        print_err('No example snippets found.')
        return 1

    length = 0
    skipped = 0
    for filepath in sorted(snippetinfo):
        snippets = snippetinfo[filepath]
        length += len(snippets)
        printed_file = False
        for snippet in snippets:
            if (
                    (name_pat is not None) and
                    (name_pat.search(snippet.name) is None)):
                debug(f'Skipping non-matching snippet: {snippet.name}')
                skipped += 1
                continue
            if not printed_file:
                printed_file = True
                print(C(f'\n{filepath}', 'cyan', style='underline')(':'))
            if names_only:
                print(C(f'    {snippet.name}', 'blue'))
            else:
                print(C(snippet))

    if (name_pat is not None) and (skipped == length):
        print(C(': ').join(
            C('No snippets matching', 'red'),
            C(name_pat.pattern, 'blue')
        ))
        return 1
    lenfmt = C(length, 'yellow', style='bright')
    found = length - skipped
    foundfmt = C(found, 'blue', style='bright')
    plural = 'snippet' if found == 1 else 'snippets'
    msg = f'Found {foundfmt} {plural}.'
    if found != length:
        msg = f'{msg} Total: {lenfmt}'
    if skipped:
        skipfmt = C(skipped, 'blue', style='bright')
        msg = f'{msg}, Skipped: {skipfmt}'
    print(f'\n{msg}')
    return 0 if length else 1


def no_output_str(filepath, src_file=None):
    """ Used as output (a message about no output) when a compiled example
        has none.
    """
    if src_file:
        srcpath = src_file
    elif filepath.endswith('.binary'):
        srcpath = ''.join((os.path.splitext(filepath)[0], '.c'))
    else:
        srcpath = f'{filepath}.c'
    msg = C('\n').join(
        C(': ').join(
            C('   Source', 'cyan'),
            C(f'{srcpath}', 'blue'),
        ),
        C(' ').join(
            C('no output from', 'dimgrey'),
            C(filepath, 'lightblue'),
        ).join('           <', '>'),
    )
    return f'{msg}\n'


def prepend_to_file(filepath, s, cond_first_line=None):
    """ Prepend a string to a file's contents.
        If `cond_first_line` is set, and the first line in the file matches,
        then nothing is prepended.

        Returns True if the file was modified, otherwise False.
    """
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except EnvironmentError as ex:
        raise EditError(
            f'Failed to prepend header to file: {filepath}\nError: {ex}'
        )

    if (not cond_first_line) or (cond_first_line not in lines[0]):
        lines.insert(0, s)
    else:
        # No need to re-write the file, with nothing changed.
        return False
    try:
        with open(filepath, 'w') as f:
            f.writelines(lines)
    except EnvironmentError as ex:
        EditError(
            f'Failed to write header to file: {filepath}\nError: {ex}'
        )
    return True


def print_err(*args, **kwargs):
    """ A wrapper for print() that uses stderr by default.
        Colorizes messages, unless a Colr itself is passed in.
    """
    if kwargs.get('file', None) is None:
        kwargs['file'] = sys.stderr

    # Use color if the file is a tty.
    if kwargs['file'].isatty():
        # Keep any Colr args passed, convert strs into Colrs.
        msg = kwargs.get('sep', ' ').join(
            str(a) if isinstance(a, C) else str(C(a, 'red'))
            for a in args
        )
    else:
        # The file is not a tty anyway, no escape codes.
        msg = kwargs.get('sep', ' ').join(
            str(a.stripped() if isinstance(a, C) else a)
            for a in args
        )

    print(msg, **kwargs)


def read_file(filepath):
    if not filepath:
        return None
    with open(filepath, 'r') as f:
        return Snippet(f.read(), name=filepath)


def read_stdin():
    """ Return data from stdin, with a message printed if stdin is a tty. """
    if sys.stdin.isatty() and sys.stdout.isatty():
        print('\nReading from stdin until end of file (Ctrl + D)...\n')
    return Snippet(sys.stdin.read(), name='stdin')


def run_compile_cmd(filepath, args):
    """ Use subprocess to run a gcc command. """
    try:
        proc = subprocess.Popen(args)
    except subprocess.CalledProcessError:
        raise CompileError(filepath)
    return proc.wait()


def run_compiled_exe(
        filepath, exe=None, src_file=None, show_name=False,
        memcheck=False, quiet=False):
    """ Run an executable (the compiled snippet). """
    if not filepath.startswith(TMPDIR):
        newpath = os.path.join(TMPDIR, os.path.split(filepath)[-1])
        os.move(filepath, newpath)
        filepath = newpath
    if show_name:
        namefmt = C(filepath, 'blue', style='bright')
        if exe:
            namefmt = C(' ').join(C(exe, 'blue'), namefmt)
        elif memcheck:
            fmtpcs = [
                C('valgrind', 'magenta'),
                C('=').join(
                    C('--tool', 'blue'),
                    C('memcheck', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--show-leak-kinds', 'blue'),
                    C('all', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--track-origins', 'blue'),
                    C('yes', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--error-exitcode', 'blue'),
                    C('1', 'lightblue', style='bright')
                ),
            ]
            if quiet:
                fmtpcs.append(C('--quiet', 'blue'))
            fmtpcs.append(namefmt)
            namefmt = C(' ').join(fmtpcs)
        status(C(': ').join(
            C('  Running', 'cyan'),
            namefmt,
        ))
    if exe:
        cmd = [exe, filepath]
    elif memcheck:
        cmd = [
            'valgrind',
            '--tool=memcheck',
            '--leak-check=full',
            '--show-leak-kinds=all',
            '--track-origins=yes',
            '--error-exitcode=1',
        ]
        if quiet:
            cmd.append('--quiet')
        cmd.append(filepath)
    else:
        cmd = [filepath]
    debug(f'Trying to run: {" ".join(cmd)}')
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except subprocess.CalledProcessError:
        print_err('Snippet failed to run!')
        return 1
    try:
        stdout = proc.stdout.decode(errors='replace')
    except UnicodeDecodeError:
        stdout = f'Unable to decode stdout:\n    {proc.stdout!r}\n'
    try:
        stderr = proc.stderr.decode(errors='replace')
    except UnicodeDecodeError:
        stderr = f'Unable to decode stderr:\n    {proc.stderr!r}\n'
    if not (stdout or stderr):
        stderr = None if quiet else no_output_str(filepath, src_file=src_file)
    if stdout:
        print(stdout)
    if stderr:
        print(stderr, file=sys.stderr)
    return proc.returncode


def run_examples(
        pat=None, exe=None, show_name=False, compiler_args=None,
        memcheck=False, quiet=False):
    """ Compile and run source examples, with optional filtering pattern.
    """
    errs = 0
    success = 0
    skipped = 0
    total = 0
    for filepath, snippets in find_src_examples().items():
        usesnippets = []
        for snippet in snippets:
            total += 1
            if (pat is not None) and (pat.search(snippet.name) is None):
                debug(f'Skipping snippet for pattern: {snippet.name}')
                skipped += 1
                continue
            usesnippets.append(snippet)
        filefmt = C(filepath, 'blue', style='bright')
        if not usesnippets:
            debug(f'All snippets skipped for: {filefmt}')
            continue
        allsnipscnt = len(snippets)
        snipscnt = len(usesnippets)
        if snipscnt != allsnipscnt:
            count = C('/').join(
                C(snipscnt, 'cyan', style='bright'),
                C(allsnipscnt, 'blue', style='bright')
            )
        else:
            count = C(allsnipscnt, 'blue', style='bright')
        plural = 'snippet' if count == 1 else 'snippets'
        status(f'\nCompiling {count} {plural} for: {filefmt}')
        errs += run_snippets(
            usesnippets,
            exe=exe,
            show_name=show_name,
            compiler_args=compiler_args,
            memcheck=memcheck,
            quiet=quiet,
        )
        success += (snipscnt - errs)

    status(C(' ').join(
        C(': ').join(
            C('\nSource Examples', 'cyan'),
            C(', ').join(
                C(': ').join(
                    C('Success', 'cyan'),
                    C(success, 'green' if success else 'red', style='bright')
                ),
                C(': ').join(
                    C('Total', 'cyan'),
                    C(total, 'blue' if total else 'red', style='bright')
                ),
                C(': ').join(
                    C('Skipped', 'cyan'),
                    C(skipped, 'blue' if skipped else 'dimgrey', style='bright')
                ),
                C(': ').join(
                    C('Error', 'cyan'),
                    C(errs, 'red' if errs else 'dimgrey', style='bright')
                ),
            ).join('(', ')'),
        )
    ))
    return errs


def run_snippets(
        snippets, exe=None, show_name=False, compiler_args=None,
        memcheck=False, quiet=False):
    """ Compile and run several c code snippets. """
    errs = 0
    for snippet in snippets:
        binaryname = snippet.compile(user_args=compiler_args)
        errs += 0 if run_compiled_exe(
            binaryname,
            exe=exe,
            src_file=snippet.src_file,
            show_name=show_name,
            memcheck=memcheck,
            quiet=quiet,
        ) == 0 else 1
    return errs


def noop(*args, **kwargs):
    """ Used to replace other functions with a no-op function call,
        to disable them.
    """
    return None


def preprocess_snippets(snippets, compiler_args=None):
    """ Compile and run several c code snippets. """
    errs = 0
    for snippet in snippets:
        errs += snippet.preprocess(user_args=compiler_args)
    return errs


def status(*args, **kwargs):
    """ Wrapper for print() that may be replaced with noop() when --quiet
        is used.
    """
    print(*args, **kwargs)


def temp_file(ext='.c', extra_prefix=None):
    """ Return a temporary (open) fd and name. """
    if extra_prefix:
        prefix = f'{TMPPREFIX}-{extra_prefix}'
    else:
        prefix = TMPPREFIX
    fd, name = tempfile.mkstemp(prefix=prefix, suffix=ext)
    return fd, name


def temp_file_name(ext=None, extra_prefix=None):
    """ Return a temporary file name. """
    if extra_prefix:
        prefix = f'{TMPPREFIX}-{extra_prefix}'
    else:
        prefix = TMPPREFIX
    return tempfile.mktemp(prefix=prefix, suffix=ext)


def try_repat(s):
    """ Try compiling a regex pattern.
        If None is passed, None is returned.
        On errors, a message is printed and the program exits.
        On success, a compiled regex pattern is returned.
    """
    if not s:
        return None
    try:
        p = re.compile(s)
    except re.error as ex:
        print('\nInvalid pattern: {}\n{}'.format(s, ex))
        sys.exit(1)
    return p


def view_snippet(filepath=None, text=None, show_name=False, quiet=False):
    if not (filepath or text):
        raise ViewError('no snippet info to view.')
    if filepath:
        try:
            with open(filepath, 'r') as f:
                text = f.read()
        except FileNotFoundError:
            raise ViewError(f'snippet is gone: {filepath}')
        except EnvironmentError as ex:
            raise ViewError(f'can\'t read snippet file: {filepath}\n{ex}')
    snippet = Snippet(text, name=filepath if filepath else 'last-snippet')
    snippet.indent = 0
    snippet.quiet_mode = quiet
    if quiet:
        snippet.code_indent = 0

    print(C(snippet))
    if show_name:
        print(C(': ').join(
            '\nResulting executable',
            C(
                config.get(
                    'last_binary',
                    C('none', 'red').join('<', '>')
                ),
                'blue'
            )
        ))
    return 0


class CompileError(ValueError):
    def __init__(self, filepath):
        self.filepath = filepath

    def __str__(self):
        return f'Can\'t compile snippet: {self.filepath}'


class EditError(CompileError):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return f'Failed to edit last snippet: {self.msg}'


class ViewError(EditError):
    def __str__(self):
        return f'Failed to view last snippet: {self.msg}'


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return 'Invalid argument, {}'.format(self.msg)
        return 'Invalid argument!'


class UserCancelled(KeyboardInterrupt):
    def __str__(self):
        return 'User cancelled.'


class Snippet(object):
    """ A Snippet is just a string, with an optional name. """
    indent = 4
    code_indent = 4
    quiet_mode = False

    def __init__(self, code, name=None):
        self.code = code
        self.name = str(name or 'unknown snippet')
        if self.name.startswith('//'):
            self.name = str(format_leader(self.name))
        else:
            self.name = str(C(self.name, 'blue'))
        # Set when code is written to a temp file:
        self.src_file = None

    def __bool__(self):
        return bool(self.code)

    def __colr__(self):
        if self.code.startswith(self.name):
            # It may not, because not all snippets have an id/name.
            code = '\n'.join(self.code.split('\n')[1:])
        else:
            code = self.code
        spaces = ' ' * self.indent
        code_spaces = ' ' * self.code_indent
        code = highlight_snippet(code).replace('\n', f'\n{code_spaces}')
        codefmt = C(f'{code_spaces}{code}')
        if self.quiet_mode:
            return codefmt
        return C('\n').join(
            f'\n{spaces}{self.name}:',
            codefmt
        )

    def __repr__(self):
        return '\n'.join((
            f'{type(self).__name__}(',
            f'    code=({type(self.code).__name__}) {self.code!r}',
            f'    name=({type(self.name).__name__}) {self.name!r}',
            ')',
        ))

    def __str__(self):
        return str(self.code)

    def compile(self, user_args=None):
        status(C(': ').join(
            C('Compiling', 'cyan'),
            self.name,
        ))

        self.write_code(self.wrap_code(self.code), ext='.c')
        config['last_c_file'] = self.src_file
        config['last_snippet'] = self.code
        basename = os.path.split(os.path.splitext(self.src_file)[0])[-1]
        objname = f'{basename}.o'
        cfiles = [self.src_file, COLRC_FILE]
        cmd = get_gcc_cmd(cfiles, user_args=user_args)
        try:
            debug('Compiling C files:')
            debug(' '.join(cmd), align=True)
            compret = run_compile_cmd(self.src_file, cmd)
        except subprocess.CalledProcessError:
            raise CompileError(self.src_file)
        else:
            if compret != 0:
                raise CompileError(self.src_file)
        colrbase = os.path.splitext(COLRC_FILE)[0]
        colrobj = f'{colrbase}.o'
        tmpobjnames = get_obj_files()
        objnames = []
        objnames.extend(tmpobjnames)
        objnames.append(colrobj)
        binaryname = temp_file_name(ext='.binary')
        linkcmd = get_gcc_cmd(
            objnames,
            output_file=binaryname,
            user_args=user_args,
        )
        try:
            debug('Linking object files:')
            debug(' '.join(linkcmd), align=True)
            linkret = run_compile_cmd(objname, linkcmd)
        except subprocess.CalledProcessError:
            raise CompileError(self.src_file)
        else:
            if linkret != 0:
                clean_objects(tmpobjnames)
                raise CompileError(self.src_file)

        clean_objects(tmpobjnames)

        if not os.path.exists(binaryname):
            debug('Compilation failed, bailing out.')
            raise CompileError(self.src_file)
        debug('Making the binary executable...')
        os.chmod(binaryname, stat.S_IRWXU)
        config['last_binary'] = binaryname
        return binaryname

    @staticmethod
    def is_main_sig(line):
        """ Returns True if the `line` looks like a main() signature. """
        line = line.lstrip()
        return (
            line.startswith('int main') or
            line.startswith('void main') or
            line.startswith('main(')
        )

    def preprocess(self, user_args=None):
        """ Run this snippet through gcc's preprocessor and print the output.
        """
        status(C(': ').join(
            C('Compiling', 'cyan'),
            self.name,
        ))

        filepath = self.write_code(self.wrap_code(self.code), ext='.c')
        config['last_c_file'] = filepath
        config['last_snippet'] = self.code
        cfiles = [filepath, COLRC_FILE]
        cmd = get_gcc_cmd(cfiles, user_args=user_args, preprocess=True)
        try:
            debug('Preprocessing C files:')
            debug(' '.join(cmd), align=True)
            compret = run_compile_cmd(filepath, cmd)
        except subprocess.CalledProcessError:
            raise CompileError(filepath)
        else:
            if compret != 0:
                raise CompileError(filepath)
        return 0

    def wrap_code(self, code):
        """ Wrap a C snippet in a main() definition, with colr.h included.
            If main() is already defined, the snippet is not wrapped.
            If colr.h is already included, no duplicate include is added.
        """
        line_table = set(line.strip() for line in code.splitlines())
        # dbug.h must come first, so we don't redefine dbug() with colr.h,
        # because it thinks it isn't defined.
        includes = ('dbug.h', 'colr.h')
        lines = []
        for includename in includes:
            includedef = f'#include "{includename}"'
            if includedef in line_table:
                debug(f'Not including {includename}')
                continue
            lines.append(includedef)
            debug(f'Including {includename}')

        for macroname in sorted(MACROS):
            defline = MACROS[macroname]['define']
            if defline in line_table:
                debug(f'Not including macro: {macroname}')
                continue
            lines.append(f'#ifndef {macroname}')
            lines.append(f'    {defline}')
            lines.append(f'#endif // ifdef {macroname}')
            debug(f'Including macro: {macroname}')

        main_sigs = [s for s in line_table if self.is_main_sig(s)]
        main_sig = main_sigs[0] if main_sigs else None
        if main_sig:
            debug('No main() needed.')
            lines.append(code)
        else:
            indent = 4 if code.lstrip()[0] == code[0] else 0
            lines.append(self.wrap_main(code.rstrip(), indent=indent))
        return '\n'.join(lines)

    def wrap_main(self, code, indent=0):
        """ Wrap a piece of code in a main() function. """
        debug('Wrapping in main()...')
        if ('argc' in code) or ('argv' in code):
            mainsig = 'int main(int argc, char* argv[])'
            debug('Using argc and argv.', align=True)
        else:
            mainsig = 'int main(void)'
            debug('Not using argc or argv.', align=True)
        if code.endswith(';') or code.endswith('}'):
            debug('No semi-colon needed.', align=True)
        else:
            debug('Adding semi-colon to code.', align=True)
            code = f'{code};'

        if indent:
            spaces = ' ' * indent
            code = '\n'.join(f'{spaces}{s}' for s in code.splitlines())
        return f'{mainsig} {{\n{code}\n}}'

    def write_code(self, s, ext='.c'):
        """ Write a string to a temporary file, and return the file name. """
        fd, filepath = temp_file(ext=ext)
        os.write(fd, s.encode())
        os.close(fd)
        debug(f'Wrote code to: {filepath}')
        self.src_file = filepath
        return filepath


if __name__ == '__main__':
    try:
        mainret = main(docopt(USAGESTR, version=VERSIONSTR, script=SCRIPT))
    except (InvalidArg, CompileError) as ex:
        print_err(ex)
        mainret = 1
    except (EOFError, KeyboardInterrupt):
        print_err('\nUser cancelled.\n')
        mainret = 2
    except BrokenPipeError:
        print_err('\nBroken pipe, input/output was interrupted.\n')
        mainret = 3
    config.save()
    sys.exit(mainret)
