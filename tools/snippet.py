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
        Preset as ColrPreset,
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
VERSION = '0.2.5'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

CONFIGFILE = os.path.join(SCRIPTDIR, 'snippet.json')
config = load_json_settings(
    CONFIGFILE,
    default={
        'defines': {},
        'editor': [],
        'last_snippet': os.path.join(SCRIPTDIR, 'snippet.last.c'),
        'last_binary': None,
        'last_c_file': None,
        'includes': {'system': [], 'local': []},
        'macros': {},
    }
)
# Set in main().
LAST_SNIPPET = None

TMPPREFIX = NAME.replace(' ', '').lower()
TMPDIR = tempfile.gettempdir()

if config['editor']:
    EDITOR_DESC = f'set from config: {config["editor"][0]}'
else:
    EDITOR = os.environ.get('EDITOR', None)
    if EDITOR:
        config['editor'] = [EDITOR]
        EDITOR_DESC = f'set from env: {config["editor"][0]}'
    else:
        config['editor'] = ['vim']
        EDITOR_DESC = f'not set, using: {config["editor"][0]}'

# TODO: This tool cannot be used with other projects because of this:
COLR_DIR = os.path.abspath(os.path.join(os.path.abspath(SCRIPTDIR), '..'))
debug(f'Using ColrC dir: {COLR_DIR}')
COLRC_FILE = os.path.join(COLR_DIR, 'colr.c')
COLRH_FILE = os.path.join(COLR_DIR, 'colr.h')
EXAMPLES_SRC = (COLRC_FILE, COLRH_FILE)

usage_incl_lines = []
for incltype in sorted(config['includes']):
    incls = config['includes'][incltype]
    if not incls:
        continue
    usage_incl_lines.append(f'        {incltype.title()}:')
    usage_incl_lines.extend(f'            {s}' for s in incls)
USAGE_INCLUDES = '\n'.join(usage_incl_lines)
USAGE_MACROS = '\n'.join(
    f'{name:>27}  : {config["macros"][name]["desc"]}'
    for name in sorted(config['macros'])
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
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-t name] -x [PATTERN] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-t name] [CODE] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-t name] [-f file...] [-- ARGS...]
        {SCRIPT} [-D] [-n] [-q] [-m | -r exe | -s] [-t name] [-w] (-e [CODE] | -l) [-- ARGS...]
        {SCRIPT} [-D] -E [-t name] [CODE] [-- ARGS...]
        {SCRIPT} [-D] -E [-t name] [-f file...] [-- ARGS...]
        {SCRIPT} [-D] -E [-t name] [-w] (-e [CODE] | -l) [-- ARGS...]

    Options:
        ARGS                   : Extra arguments for the compiler.
        CODE                   : Code to compile. It is auto-wrapped in a main()
                                 function if no main() signature is found.
                                 Auto-includes are included unless include
                                 lines for them are found.
                                 When used with -e, the editor is started with
                                 this as it's content.
                                 Default: stdin
        PATTERN                : Only use examples with a leading comment
                                 that matches this text/regex pattern.
        -b,--lastbinary        : Re-run the last binary that was compiled.
        -c,--clean             : Clean {TMPDIR} files, even though they will be
                                 cleaned when the OS reboots.
        -D,--debug             : Show some debug info while running.
        -E,--preprocessor      : Run through gcc's preprocessor and print the
                                 output.
        -e,--editlast          : Edit the last snippet in $EDITOR and run it.
                                 If the CODE argument is given, it will replace
                                 the contents of the last snippet.
                                 $EDITOR is {EDITOR_DESC}
        -f name,--file name    : Read file to get snippet to compile.
        -h,--help              : Show this help message.
        -L,--listexamples      : List example code snippets in the source.
        -l,--last              : Re-run the last snippet.
        -m,--memcheck          : Run the snippet through `valgrind`.
        -N,--listnames         : List example snippet names from the source.
        -n,--name              : Print the resulting binary name, for further
                                 testing.
        -q,--quiet             : Don't print any status messages.
        -r exe,--run exe       : Run a program on the compiled binary, like
                                 `gdb` or `kdbg`.
        -s,--sanitize          : Use -fsanitize compiler arguments.
        -t name,--target name  : Make target to get compiler flags from.
        -V,--viewlast          : View the last snippet that was compiled.
        -v,--version           : Show version.
        -w,--wrapped           : Use the "wrapped" version, which is the resulting
                                 `.c` file for snippets.
        -x,--examples          : Use source examples as the snippets.

    Auto-Includes:
{USAGE_INCLUDES or '    <no includes set in config>'}

    Predefined Macros:
{USAGE_MACROS or '    <no macros set in config>'}
"""  # noqa (ignore long lines)

CErr = ColrPreset(fore='red')
CFile = ColrPreset(fore='blue', style='bright')
CName = ColrPreset(fore='blue')
CInfo = ColrPreset(fore='cyan')
CNum = ColrPreset(fore='blue', style='bright')
CNumInfo = ColrPreset(fore='cyan', style='bright')


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    global status
    last_snippet_read()
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
            show_name=argd['--name'],
            memcheck=argd['--memcheck'],
            quiet=argd['--quiet'],
            make_target=argd['--target'],
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
            if not LAST_SNIPPET:
                raise InvalidArg('no "last snippet" found.')
            return view_snippet(
                text=LAST_SNIPPET,
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
            if not (argd['CODE'] or LAST_SNIPPET):
                raise InvalidArg('no "last snippet" found, no code given.')
            snippet = edit_snippet(text=argd['CODE'] or LAST_SNIPPET)
        if not snippet:
            raise UserCancelled()
        snippets = [snippet]
    elif argd['--last']:
        if not LAST_SNIPPET:
            raise InvalidArg('no "last snippet" found.')
        snippets = [Snippet(LAST_SNIPPET, name='last-snippet')]
    else:
        snippets = [
            Snippet(argd['CODE'], name='cmdline-snippet') or read_stdin()
        ]

    if argd['--preprocessor']:
        return preprocess_snippets(
            snippets,
            compiler_args=argd['ARGS'],
            make_target=argd['--target'],
        )

    return run_snippets(
        snippets,
        exe=argd['--run'],
        show_name=argd['--name'],
        compiler_args=argd['ARGS'],
        memcheck=argd['--memcheck'],
        quiet=argd['--quiet'],
        make_target=argd['--target'],
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
        cmd = ' '.join(config['editor'])
        cmdargs = ' '.join((cmd, filepath))
        debug(f'Running: {cmdargs}')
        ret = os.system(cmdargs)
    except Exception as ex:
        raise EditError(f'Failed to edit last snippet: {ex}')
    else:
        if ret != 0:
            raise EditError(f'Editor ({config["editor"]}) returned non-zero!')

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
            CName(desc),
            C(name, 'blue', style='bright'),
        )
    else:
        fmted = CName(firsttrim)
    return fmted


def get_example_snippets(pat=None):
    snippetinfo = {'all': {'total': 0, 'skipped': 0}}
    for filepath, snippets in find_src_examples().items():
        filetotal = 0
        fileskipped = 0
        usesnippets = []
        for snippet in snippets:
            filetotal += 1
            if (pat is not None) and (pat.search(snippet.name) is None):
                debug(f'Skipping snippet for pattern: {snippet.name}')
                fileskipped += 1
                continue
            usesnippets.append(snippet)
        if not usesnippets:
            debug(f'All snippets skipped for: {CFile(filepath)}')
            continue
        snippetinfo[filepath] = {
            'total': filetotal,
            'skipped': fileskipped,
            'snippets': usesnippets,
        }
        snippetinfo['all']['total'] += filetotal
        snippetinfo['all']['skipped'] += fileskipped
    return snippetinfo


def get_gcc_cmd(
        input_files, output_file=None, user_args=None, preprocess=False,
        make_target=None):
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
    cmd.extend(get_make_flags(user_args=[make_target] if make_target else None))
    cmd.extend(user_args or [])
    return cmd


def get_make_flags(user_args=None):
    """ Get gcc flags from a `make` dry run, and return them. """
    # These flags don't make since for general snippet compiling.
    # The ColrC dir is already taken care of, and -c/-o will be used when
    # get_gcc_cmd() is called.
    ignore_flags = {'-c', '-o', '-iquote../'}
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


def last_snippet_read():
    """ Read the last-snippet file, and set the global LAST_SNIPPET. """
    global LAST_SNIPPET
    snippetfile = config['last_snippet']
    try:
        with open(snippetfile, 'r') as f:
            LAST_SNIPPET = f.read()
    except FileNotFoundError:
        LAST_SNIPPET = None
    except EnvironmentError as ex:
        print_err(
            f'\nError reading last-snippet file: {snippetfile}\n{ex}'
        )
        LAST_SNIPPET = None


def last_snippet_write(code):
    """ Write to the last-snippet file. """
    if not code:
        debug('No snippet to write.')
        return False
    snippetfile = config['last_snippet']
    try:
        with open(snippetfile, 'w') as f:
            f.write(code)
    except EnvironmentError as ex:
        print_err(
            f'Error writing last-snippet file: {snippetfile}\n{ex}'
        )
    else:
        debug(f'Wrote last-snippet file: {snippetfile}')
        return True
    return False


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
                print(CName(f'    {snippet.name}'))
            else:
                print(C(snippet))

    if (name_pat is not None) and (skipped == length):
        print(C(': ').join(
            CErr('No snippets matching'),
            CName(name_pat.pattern)
        ))
        return 1
    found = length - skipped
    plural = 'snippet' if found == 1 else 'snippets'
    msg = f'Found {CNum(found)} {plural}.'
    if found != length:
        msg = f'{msg} Total: {CNumInfo(length)}'
    if skipped:
        msg = f'{msg}, Skipped: {CNum(skipped)}'
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
            CInfo('   Source'),
            CName(f'{srcpath}'),
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
            namefmt = C(' ').join(CName(exe), namefmt)
        elif memcheck:
            fmtpcs = [
                C('valgrind', 'magenta'),
                C('=').join(
                    CName('--tool'),
                    C('memcheck', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--show-leakName-kinds'),
                    C('all', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--trackName-origins'),
                    C('yes', 'lightblue', style='bright')
                ),
                C('=').join(
                    C('--errorName-exitcode'),
                    C('1', 'lightblue', style='bright')
                ),
            ]
            if quiet:
                fmtpcs.append(C('-Name-quiet'))
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
        memcheck=False, quiet=False, make_target=None):
    """ Compile and run source examples, with optional filtering pattern.
    """
    errs = 0
    success = 0
    skipped = 0
    total = 0
    for filepath, snippetinfo in get_example_snippets(pat=pat).items():
        if filepath == 'all':
            total = snippetinfo['total']
            skipped = snippetinfo['skipped']
            continue

        if snippetinfo['skipped']:
            # Some were skipped.
            count = C('/').join(
                CNumInfo(snippetinfo['skipped']),
                CNum(snippetinfo['total'])
            )
        else:
            count = CNum(snippetinfo['total'])
        plural = 'snippet' if count == 1 else 'snippets'
        status(f'\nCompiling {count} {plural} for: {CFile(filepath)}')
        errs += run_snippets(
            snippetinfo['snippets'],
            exe=exe,
            show_name=show_name,
            compiler_args=compiler_args,
            memcheck=memcheck,
            quiet=quiet,
            make_target=make_target,
        )
        snipscnt = snippetinfo['total'] - snippetinfo['skipped']
        success += snipscnt - errs

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
        memcheck=False, quiet=False, make_target=None):
    """ Compile and run several c code snippets. """
    errs = 0
    for snippet in snippets:
        binaryname = snippet.compile(
            user_args=compiler_args,
            make_target=make_target,
        )
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


def preprocess_snippets(snippets, compiler_args=None, make_target=None):
    """ Compile and run several c code snippets. """
    errs = 0
    for snippet in snippets:
        errs += snippet.preprocess(
            user_args=compiler_args,
            make_target=make_target,
        )
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


def view_examples(pat=None, show_name=False, quiet=False):
    errs = 0
    for filepath, snippetinfo in get_example_snippets(pat=pat):
        errs += view_snippets(
            snippetinfo['snippets'],
            show_name=show_name,
            quiet=quiet
        )
    return errs


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
    if isinstance(text, Snippet):
        # Snippet was passed in.
        snippet = text
    else:
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
                    CErr('none').join('<', '>')
                ),
                'blue'
            )
        ))
    return 0


def view_snippets(snippets, show_name=False, quiet=False):
    return sum(
        view_snippet(text=s, show_name=show_name, quiet=quiet)
        for s in snippets
    )


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
            self.name = str(CName(self.name))
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

    def compile(self, user_args=None, make_target=None):
        status(C(': ').join(
            C('Compiling', 'cyan'),
            self.name,
        ))

        self.write_code(self.wrap_code(self.code), ext='.c')
        config['last_c_file'] = self.src_file
        last_snippet_write(self.code)
        basename = os.path.split(os.path.splitext(self.src_file)[0])[-1]
        objname = f'{basename}.o'
        cfiles = [self.src_file, COLRC_FILE]
        colrobj = 'colr.o'
        if os.path.exists(colrobj):
            try:
                os.remove(colrobj)
                debug(f'Removed existing: {colrobj}')
            except EnvironmentError as ex:
                print_err(f'Unable to remove {colrobj}: {ex}')

        cmd = get_gcc_cmd(cfiles, user_args=user_args, make_target=make_target)
        try:
            debug('Compiling C files:')
            debug(' '.join(cmd), align=True)
            compret = run_compile_cmd(self.src_file, cmd)
        except subprocess.CalledProcessError:
            raise CompileError(self.src_file)
        else:
            if compret != 0:
                raise CompileError(self.src_file)
        tmpobjnames = get_obj_files()
        debug(f'Found object files: {", ".join(tmpobjnames)}')
        objnames = []
        objnames.extend(tmpobjnames)
        objnames.append(colrobj)
        binaryname = temp_file_name(ext='.binary')
        linkcmd = get_gcc_cmd(
            objnames,
            output_file=binaryname,
            user_args=user_args,
            make_target=make_target,
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
    def fix_macro_lines(deflines, indent=0):
        """ Indent macro definitions if `indent` is set, and add the continuation
            char `\\` for all lines that need it if it's not already there.
        """
        spaces = ' ' * indent
        if len(deflines) == 1:
            return [f'{spaces}{deflines[0]}']
        lines = []
        for line in deflines:
            if not line.endswith('\\'):
                lines.append(f'{spaces}{line} \\')
            else:
                lines.append(f'{spaces}{line}')
        lines[-1] = lines[-1].rstrip('\\').rstrip()
        return lines

    def has_define(self, name, lines=None):
        """ Return True if self.code seems to define a given name. """
        lines = lines or set(
            line.strip() for line in self.code.splitlines()
        )
        defpat = re.compile(rf'#([ \t]+)?define([ \t]+){name}[( \t]')
        for line in lines:
            if defpat.search(line) is not None:
                debug(f'Found define: {line}')
                return True
        return False

    @staticmethod
    def is_main_sig(line):
        """ Returns True if the `line` looks like a main() signature. """
        line = line.lstrip()
        return (
            line.startswith('int main') or
            line.startswith('void main') or
            line.startswith('main(')
        )

    def preprocess(self, user_args=None, make_target=None):
        """ Run this snippet through gcc's preprocessor and print the output.
        """
        status(C(': ').join(
            C('Compiling', 'cyan'),
            self.name,
        ))

        filepath = self.write_code(self.wrap_code(self.code), ext='.c')
        config['last_c_file'] = filepath
        last_snippet_write(self.code)
        cfiles = [filepath, COLRC_FILE]
        cmd = get_gcc_cmd(
            cfiles,
            user_args=user_args,
            preprocess=True,
            make_target=make_target,
        )
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
        lines = []
        for defname in sorted(config['defines']):
            if self.has_define(defname, lines=line_table):
                debug(f'Not defining {defname}')
                continue
            defline = f'#define {defname} {config["defines"][defname]}'
            lines.append(f'#ifndef {defname}')
            lines.append(f'    {defline}')
            lines.append(f'#endif // {defname}')
            debug(f'Defined {defname} {config["defines"][defname]}')

        includes = config['includes']
        for includetype in sorted(includes):
            for includename in includes[includetype]:
                includedef = f'#include "{includename}"'
                if includedef in line_table:
                    debug(f'Not including {includetype} {includename}')
                    continue
                lines.append(includedef)
                debug(f'Including {includetype} {includename}')

        for macroname in sorted(config['macros']):
            deflines = config['macros'][macroname]['define']
            if isinstance(deflines, str):
                deflines = [deflines]
            if self.has_define(macroname, lines=line_table):
                debug(f'Not including macro: {macroname}')
                continue
            lines.append(f'#ifndef {macroname}')
            lines.extend(self.fix_macro_lines(deflines, indent=4))
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
    config.save(sort_keys=True)
    sys.exit(mainret)
