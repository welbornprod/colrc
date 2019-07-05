#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" run_snippet.py
    Compiles a small ColrC snippet and runs it. This is for small testing
    and debugging purposes.
    -Christopher Welborn 07-04-2019
"""

import os
import subprocess
import stat
import sys
import tempfile

from colr import (
    Colr as C,
    auto_disable as colr_auto_disable,
    docopt,
)
from printdebug import DebugColrPrinter
debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug

colr_auto_disable()

NAME = 'run_snippet.py'
VERSION = '0.0.1'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

USAGESTR = """{versionstr}
    Usage:
        {script} -h | -v
        {script} [-D] [-n] [CODE]
        {script} [-D] [-n] [-f file]

    Options:
        CODE                 : Code to compile. It is wrapped in a main()
                               function, with colr.h included.
                               Default: stdin
        -D,--debug           : Show some debug info while running.
        -f name,--file name  : Read file to get snippet to compile.
        -h,--help            : Show this help message.
        -n,--name            : Print the resulting binary name, for further
                               testing.
        -v,--version         : Show version.
""".format(script=SCRIPT, versionstr=VERSIONSTR)

COLR_DIR = SCRIPTDIR
COLRC_FILE = os.path.join(COLR_DIR, 'colr.c')


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    if argd['--file']:
        code = read_file(argd['--file'])
    else:
        code = argd['CODE'] or read_stdin()

    binaryname = compile_code(code)
    if argd['--name']:
        print(C(': ').join(
            C('Running', 'cyan'),
            C(binaryname, 'blue', style='bright'),
        ))
    return run_compiled_exe(binaryname)


def clean_objects(filepaths):
    debug('Removing temporary object files...')
    for filepath in filepaths:
        if os.path.exists(filepath):
            os.remove(filepath)


def compile_code(s, user_args=None):
    filepath = write_code(wrap_code(s), ext='.c')
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
    return binaryname


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
        if s.startswith('run_snippet.') and s.endswith('.o')
    ]


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
        return f.read()


def read_stdin():
    """ Return data from stdin, with a message printed if stdin is a tty. """
    if sys.stdin.isatty() and sys.stdout.isatty():
        print('\nReading from stdin until end of file (Ctrl + D)...\n')
    return sys.stdin.read()


def run_compile_cmd(filepath, args):
    """ Use subprocess to run a gcc command. """
    try:
        proc = subprocess.Popen(args)
    except subprocess.CalledProcessError:
        raise CompileError(filepath)
    return proc.wait()


def run_compiled_exe(filepath):
    """ Run an executable (the compiled snippet). """
    if not filepath.startswith('/tmp'):
        newpath = os.path.join('/tmp', os.path.split(filepath)[-1])
        os.move(filepath, newpath)
        filepath = newpath

    debug(f'Trying to run: {filepath}')
    try:
        subprocess.check_call([filepath])
    except subprocess.CalledProcessError:
        print_err('Snippet failed to run!')
        return 1
    return 0


def temp_file(ext='.c'):
    """ Return a temporary (open) fd and name. """
    fd, name = tempfile.mkstemp(prefix=NAME, suffix=ext)
    return fd, name


def temp_file_name(ext=None):
    """ Return a temporary file name. """
    return tempfile.mktemp(prefix=NAME, suffix=ext)


def wrap_code(s):
    """ Wrap a C snippet in a main() definition, with colr.h included.
        If main() is already defined, the snippet is not wrapped.
        If colr.h is already included, no duplicate include is added.
    """
    include_colr_h = '#include "colr.h"' not in s
    wrap_main = 'main(' not in s

    lines = []
    if include_colr_h:
        debug('Including colr.h...')
        lines.append('#include "colr.h"')
    else:
        debug('Not including colr.h!')
    if wrap_main:
        debug('Wrapping in main()...')
        lines.append('int main(void) {')
    else:
        debug('Not wrapping in main()!')
    lines.append(s)

    if wrap_main and (not s.endswith(';')):
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


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return 'Invalid argument, {}'.format(self.msg)
        return 'Invalid argument!'


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
    sys.exit(mainret)
