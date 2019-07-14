#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" run_snippet.py
    Compiles a small ColrC snippet and runs it. This is for small testing
    and debugging purposes.
    -Christopher Welborn 07-04-2019
"""

import os
import re
import subprocess
import stat
import sys
import tempfile

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

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug

pyg_lexer = get_lexer_by_name('c')
pyg_fmter = Terminal256Formatter(bg='dark', style='monokai')

colr_auto_disable()

NAME = 'ColrC - Snippet Runner'
VERSION = '0.0.2'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

CONFIGFILE = os.path.join(SCRIPTDIR, 'run_snippet.json')
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

COLR_DIR = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
COLRC_FILE = os.path.join(COLR_DIR, 'colr.c')
COLRH_FILE = os.path.join(COLR_DIR, 'colr.h')
DBUGH_FILE = os.path.join(COLR_DIR, 'dbug.h')
EXAMPLES_SRC = (COLRC_FILE, COLRH_FILE)

MACROS = {
    'print_repr': {
        'define': '#define print_repr(x) printf("%s\\n", colr_repr(x))',
        'desc': 'Wrapper for printf("%s\\n", colr_repr(x)).',
    },
}

USAGE_MACROS = '\n'.join(
    f'{name:>27}  : {MACROS[name]["desc"]}'
    for name in sorted(MACROS)
)

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} -c | -h | -v
        {SCRIPT} [-D] -L
        {SCRIPT} [-D] [-n] [-q] [-r exe] -E [PATTERN]
        {SCRIPT} [-D] [-n] [-q] [-r exe] [CODE]
        {SCRIPT} [-D] [-n] [-q] [-r exe] [-f file]
        {SCRIPT} [-D] [-n] [-q] [-r exe] (-e | -l)

    Options:
        CODE                 : Code to compile. It is wrapped in a main()
                               function, with colr.h included.
                               Default: stdin
        PATTERN              : Only run examples with a leading comment that
                               matches this text/regex pattern.
        -c,--clean           : Clean {TMPDIR} files, even though they will be
                               cleaned when the OS reboots.
        -D,--debug           : Show some debug info while running.
        -E,--examples        : Use source examples as the snippets.
        -e,--editlast        : Edit the last snippet in $EDITOR and run it.
        -f name,--file name  : Read file to get snippet to compile.
        -h,--help            : Show this help message.
        -L,--listexamples    : List example code snippets in the source.
        -l,--last            : Re-run the last snippet.
        -n,--name            : Print the resulting binary name, for further
                               testing.
        -q,--quiet           : Don't print any status messages.
        -r exe,--run exe     : Run a program on the compiled binary, like
                               `gdb` or `kdbg`.
        -v,--version         : Show version.

    Predefined Macros:
{USAGE_MACROS}
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    global status
    if argd['--quiet']:
        status = noop

    if argd['--clean']:
        return clean_tmp()
    elif argd['--listexamples']:
        return list_examples()
    elif argd['--examples']:
        pat = try_repat(argd['PATTERN'])
        return run_examples(pat=pat, exe=argd['--run'])

    if argd['--file']:
        snippets = [
            read_file(s)
            for s in argd['--file']
        ]
    elif argd['--editlast']:
        if not config['last_snippet']:
            raise InvalidArg('no "last snippet" found.')
        snippet = edit_snippet(config['last_snippet'])
        if not snippet:
            raise UserCancelled()
        snippets = [Snippet(snippet, name='edited-snippet')]
    elif argd['--last']:
        if not config['last_snippet']:
            raise InvalidArg('no "last snippet" found.')
        snippets = [Snippet(config['last_snippet'], name='last-snippet')]
    else:
        snippets = [Snippet(argd['CODE'], name='cmdline') or read_stdin()]

    return run_snippets(snippets, exe=argd['--run'], show_name=argd['--name'])


def clean_objects(filepaths):
    debug('Removing temporary object files...')
    for filepath in filepaths:
        if os.path.exists(filepath):
            os.remove(filepath)


def clean_tmp():
    tmpfiles = [
        s
        for s in os.listdir(TMPDIR)
        if s.startswith(TMPPREFIX)
    ]
    for filepath in tmpfiles:
        if os.path.exists(filepath):
            os.remove(filepath)

    tmplen = len(tmpfiles)
    plural = 'file' if tmplen == 1 else 'files'
    tmpfmt = C(tmplen, 'blue', style='bright')
    status(f'Cleaned {tmpfmt} temporary {plural} in: {TMPDIR}')
    return 0 if tmpfiles else 1


def edit_snippet(initial_text=None):
    fd, filepath = temp_file(extra_prefix='last_snippet')
    os.write(fd, '\n'.join((
        '// If this file is empty, or all lines are commented out with',
        '// single-line comments, the process is cancelled.',
        '',
    )).encode())
    if initial_text:
        os.write(fd, initial_text.encode())
    os.close(fd)
    editor = os.environ.get('EDITOR', 'vim')
    try:
        ret = os.system(' '.join((editor, filepath)))
    except Exception as ex:
        raise EditError(f'Failed to edit last snippet: {ex}')
    else:
        if ret != 0:
            raise EditError(f'Editor ({editor}) returned non-zero!')

    with open(filepath, 'r') as f:
        usablelines = [
            s
            for s in f
            if not s.lstrip().startswith('//')
        ]
    if not usablelines:
        raise UserCancelled()

    return ''.join(usablelines)


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


def get_gcc_cmd(input_files, output_file=None, user_args=None):
    """ Get the cmd needed to run gcc on a file (without -c or -o). """
    c_files = [s for s in input_files if s.endswith('.c')]
    cmd = ['gcc']
    if c_files:
        cmd.append('-c')
    cmd.extend(input_files)
    if output_file:
        cmd.extend(('-o', output_file))
    cmd.append(f'-iquote{COLR_DIR}')
    cmd.extend(get_make_flags(user_args=user_args))
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


def list_examples():
    """ List all example snippets found in the source. """
    snippetinfo = find_src_examples()
    if not snippetinfo:
        print_err('No example snippets found.')
        return 1

    length = 0
    for filepath in sorted(snippetinfo):
        snippets = snippetinfo[filepath]
        length += len(snippets)
        print(C('').join(C(f'\n{filepath}', 'cyan', style='underline'), ':'))
        for snippet in snippets:
            print(C(snippet))

    plural = 'snippet' if length == 1 else 'snippets'
    lenfmt = C(length, 'blue', style='bright')
    print(f'\nFound {lenfmt} {plural}.')
    return 0 if length else 1


def no_output_str(filepath):
    """ Used as output (a message about no output) when a compiled example
        has none.
    """
    msg = C('\n').join(
        C(': ').join(
            C('   Source', 'cyan'),
            C(f'{filepath}.c', 'blue'),
        ),
        C(' ').join(
            C('no output from', 'dimgrey'),
            C(filepath, 'lightblue'),
        ).join('           <', '>'),
    )
    return f'{msg}\n'


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


def run_compiled_exe(filepath, exe=None):
    """ Run an executable (the compiled snippet). """
    if not filepath.startswith(TMPDIR):
        newpath = os.path.join(TMPDIR, os.path.split(filepath)[-1])
        os.move(filepath, newpath)
        filepath = newpath
    cmd = [exe, filepath] if exe else [filepath]
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
        stderr = no_output_str(filepath)
    if stdout:
        print(stdout)
    if stderr:
        print(stderr, file=sys.stderr)
    return proc.returncode


def run_examples(pat=None, exe=None, show_name=False):
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
        errs += run_snippets(usesnippets, exe=exe, show_name=show_name)
        success += (snipscnt - errs)

    status(C(' ').join(
        C(': ').join(
            C('\nSnippets', 'cyan'),
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


def run_snippets(snippets, exe=None, show_name=False):
    """ Compile and run several c code snippets. """
    errs = 0
    for snippet in snippets:
        binaryname = snippet.compile()
        if show_name:
            namefmt = C(binaryname, 'blue', style='bright')
            if exe:
                namefmt = C(' ').join(C(exe, 'blue'), namefmt)
            status(C(': ').join(
                C('  Running', 'cyan'),
                namefmt,
            ))
        errs += run_compiled_exe(binaryname, exe=exe)
    return errs


def noop(*args, **kwargs):
    """ Used to replace other functions with a no-op function call,
        to disable them.
    """
    return None


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


def wrap_code(s):
    """ Wrap a C snippet in a main() definition, with colr.h included.
        If main() is already defined, the snippet is not wrapped.
        If colr.h is already included, no duplicate include is added.
    """
    include_colr_h = '#include "colr.h"' not in s
    include_dbug_h = '#include "dbug.h"' not in s
    wrap_main = 'main(' not in s

    lines = []
    if include_dbug_h:
        debug('Including dbug.h...')
        lines.append('#include "dbug.h"')
    else:
        debug('Not including dbug.h!')
    if include_colr_h:
        debug('Including colr.h...')
        lines.append('#include "colr.h"')
    else:
        debug('Not including colr.h!')

    for macroname in sorted(MACROS):
        lines.append(f'#ifndef {macroname}')
        defline = MACROS[macroname]['define']
        lines.append(defline)
        lines.append(f'#endif // ifdef {macroname}')
        debug(f'Including macro: {macroname}')

    if wrap_main:
        debug('Wrapping in main()...')
        lines.append('int main(void) {')
    else:
        debug('Not wrapping in main()!')
    lines.append(s)

    if wrap_main and (not s.rstrip().endswith(';')):
        debug('Adding semi-colon...')
        lines[-1] = ''.join((lines[-1], ';'))
    if wrap_main:
        lines.append('}')
    return '\n'.join(lines)


def write_code(s, ext='.c'):
    """ Write a string to a temporary file, and return the file name. """
    fd, filepath = temp_file(ext=ext)
    os.write(fd, s.encode())
    os.close(fd)
    debug(f'Wrote code to: {filepath}')
    return filepath


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
    def __init__(self, code, name=None):
        self.code = code
        self.name = name or 'unknown snippet'
        if self.name.startswith('//'):
            self.name = format_leader(self.name)
        else:
            self.name = C(self.name, 'blue')

    def __bool__(self):
        return bool(self.code)

    def __colr__(self):
        if self.code.startswith(self.name):
            # It may not, because not all snippets have an id/name.
            code = '\n'.join(self.code.split('\n')[1:])
        else:
            code = self.code
        code = highlight_snippet(code).replace('\n', '\n    ')
        return C('\n').join(
            f'\n    {self.name}:',
            f'    {code}'
        )

    def __str__(self):
        return str(self.code)

    def compile(self, user_args=None):
        status(C(': ').join(
            C('Compiling', 'cyan'),
            self.name,
        ))

        filepath = write_code(wrap_code(self.code), ext='.c')
        config['last_c_file'] = filepath
        config['last_snippet'] = self.code
        basename = os.path.split(os.path.splitext(filepath)[0])[-1]
        objname = f'{basename}.o'
        cfiles = [filepath, COLRC_FILE]
        cmd = get_gcc_cmd(cfiles, user_args=user_args)
        try:
            debug('Compiling C files:')
            debug(' '.join(cmd), align=True)
            compret = run_compile_cmd(filepath, cmd)
        except subprocess.CalledProcessError:
            raise CompileError(filepath)
        else:
            if compret != 0:
                raise CompileError(filepath)
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
            raise CompileError(filepath)
        else:
            if linkret != 0:
                clean_objects(tmpobjnames)
                raise CompileError(filepath)

        clean_objects(tmpobjnames)

        if not os.path.exists(binaryname):
            debug('Compilation failed, bailing out.')
            raise CompileError(filepath)
        debug('Making the binary executable...')
        os.chmod(binaryname, stat.S_IRWXU)
        config['last_binary'] = binaryname
        return binaryname


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
