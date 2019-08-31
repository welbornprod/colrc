#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" unused.py
    Uses cppcheck to look for unused functions in ColrC.
    -Christopher Welborn 08-30-2019
"""

import json
import os
import re
import subprocess
import sys

from colr import (
    AnimatedProgress,
    Colr as C,
    Frames,
    Preset,
    auto_disable as colr_auto_disable,
    docopt,
)
from outputcatcher import ProcessOutput
from printdebug import DebugColrPrinter
from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import Terminal256Formatter

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug
debug_err = debugprinter.debug_err

pyg_lexer = get_lexer_by_name('c')
pyg_fmter = Terminal256Formatter(bg='dark', style='monokai')

anim_frames = Frames.dots_orbit.as_rainbow()

colr_auto_disable()

NAME = 'ColrC - Usage Stats'
VERSION = '0.0.1'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

COLRC_DIR = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
COLRC_FILES = tuple(
    os.path.join(COLRC_DIR, s)
    for s in ('colr.h', 'colr.c')
)

TOOL_FILES = tuple(
    os.path.join(COLRC_DIR, s)
    for s in ('colr_tool.h', 'colr_tool.c')
)
TEST_DIR = os.path.join(COLRC_DIR, 'test')
TEST_FILES = tuple(
    os.path.join(TEST_DIR, s)
    for s in os.listdir(TEST_DIR)
    if s.endswith(('.h', '.c'))
)
EXAMPLE_DIR = os.path.join(COLRC_DIR, 'examples')
EXAMPLE_FILES = tuple(
    os.path.join(EXAMPLE_DIR, s)
    for s in os.listdir(EXAMPLE_DIR)
    if s.endswith('.c')
)
SUPPRESS_FILE = os.path.join(SCRIPTDIR, 'cppcheck.suppress.txt')
CPPCHECK_ARGS = [
    '--std=c11',
    '--enable=all',
    '--force',
    '--inline-suppr',
    '--error-exitcode=1',
]
if os.path.exists(SUPPRESS_FILE):
    CPPCHECK_ARGS.append(f'--suppressions-list={SUPPRESS_FILE}')
CPPCHECK_ARGS.extend(COLRC_FILES)
CPPCHECK_ARGS.extend(TOOL_FILES)

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-h | -v]
        {SCRIPT} [-D] [-a] [-d] [-f | -r] [-t] [PATTERN]

    Options:
        PATTERN        : Only show names matching this regex/text pattern.
        -a,--all       : Show everything cppcheck thinks is unused.
        -D,--debug     : Show more info while running.
        -d,--testdeps  : Functions with tests, or in test files are not unused.
        -f,--full      : Show full info.
        -h,--help      : Show this help message.
        -r,--raw       : Show raw info.
        -t,--untested  : Show untested functions.
        -v,--version   : Show version.
"""

CFile = Preset(fore='cyan')
CName = Preset(fore='blue')
CNum = Preset(fore='blue', style='bright')
CTestdep = Preset(fore=(255, 128, 64))
CTotal = Preset(fore='yellow')
CUntested = Preset(fore='red', style='bright')
CUnused = Preset(fore='red')


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    names = get_cppcheck_names(pat=try_repat(argd['PATTERN']))
    if not names:
        print_err('No unused names reported by cppcheck.')
        return 1
    filepaths = []
    filepaths.extend(COLRC_FILES)
    filepaths.extend(TOOL_FILES)
    filepaths.extend(TEST_FILES)
    filepaths.extend(EXAMPLE_FILES)
    info = check_files(filepaths, names)
    if not argd['--all']:
        info = filter_used(
            info,
            untested=argd['--untested'],
            test_deps=argd['--testdeps'],
        )
    if argd['--full']:
        ret = print_full(info)
    elif argd['--raw']:
        ret = print_raw(info)
    else:
        ret = print_simple(info)
    if not argd['--raw']:
        namelen = len(info)
        plural = 'function' if namelen == 1 else 'functions'
        method = 'unused'
        if argd['--all']:
            method = 'unused'
            plural = f'{plural} (according to cppcheck)'
        elif argd['--untested']:
            method = 'untested'
        elif argd['--testdeps']:
            method = 'unused test dependencies'
        print(f'\nFound {CNum(namelen)} possibly {method} {plural}.')

    return ret


def check_file(filepath, names):
    """ Look for names in a file. """
    counts = {}
    in_comment = False
    try:
        with open(filepath, 'r') as f:
            for line in f:
                stripped = line.strip()
                if stripped.startswith('/*'):
                    in_comment = True
                if in_comment:
                    if '*/' in line:
                        in_comment = False
                    continue
                if stripped.startswith('//'):
                    continue
                for name in names:
                    if name not in line:
                        continue
                    presubpat = re.compile(f'[\\w_]{name}')
                    if name not in presubpat.sub('', line):
                        continue
                    sufsubpat = re.compile(f'{name}[\\w_]')
                    if name not in sufsubpat.sub('', line):
                        continue
                    counts.setdefault(name, {'count': 0, 'lines': []})
                    counts[name]['count'] += line.count(name)
                    counts[name]['lines'].append(line)
    except EnvironmentError as ex:
        print_err(f'Cannot read file: {filepath}\n{ex}')
    finally:
        return counts


def check_files(filepaths, names):
    """ Look for names in several files. """
    counts = {}
    for filepath in filepaths:
        filecounts = check_file(filepath, names)
        if not filecounts:
            continue
        for name in filecounts:
            fileinfo = filecounts[name]
            counts.setdefault(
                name,
                {
                    'total': 0,
                    'files': {},
                    'colr_cnt': 0,
                    'tool_cnt': 0,
                    'test_cnt': 0,
                    'example_cnt': 0,
                }
            )
            counts[name]['files'][filepath] = {
                'count': fileinfo['count'],
                'lines': fileinfo['lines'],
            }
            counts[name]['total'] += fileinfo['count']

    for name, nameinfo in counts.items():
        for filepath, fileinfo in nameinfo['files'].items():
            filecount = fileinfo['count']
            filename = os.path.split(filepath)[-1]
            if filename.startswith('test_'):
                counts[name]['test_cnt'] += filecount
            elif filename.endswith('_example.c'):
                counts[name]['example_cnt'] += filecount
            elif filename.endswith(('_tool.c', '_tool.h')):
                counts[name]['tool_cnt'] += filecount
            elif filename.startswith('colr'):
                counts[name]['colr_cnt'] += filecount
    return counts


def filter_used(info, untested=False, test_deps=False):
    keep = {}
    for name, nameinfo in info.items():
        if untested:
            if is_untested(nameinfo):
                keep[name] = nameinfo
            continue
        if is_unused(nameinfo):
            if test_deps and not is_test_dep(nameinfo):
                continue
            keep[name] = nameinfo
            continue

    return keep


def get_cppcheck_names(pat=None):
    cmd = ['cppcheck']
    cmd.extend(CPPCHECK_ARGS)
    debug(f'Running: {" ".join(cmd)}')
    proc = ProcessOutput(cmd)
    names = []
    total = 0
    prog = AnimatedProgress(
        'Running cppcheck',
        frames=anim_frames,
        delay=0.5,
        auto_disable=True,
    )
    with prog:
        for line in proc.iter_stderr():
            if not line.endswith(b'is never used.'):
                continue
            _, _, name = line.partition(b'\'')
            name, _, _ = name.rpartition(b'\'')
            name = name.decode()
            if pat and (pat.search(name) is None):
                continue
            total += 1
            if total % 10 == 0:
                prog.text = f'Running cppcheck (names: {total})'
            names.append(name)
    return names


def highlight_line(line):
    """ Highlight a C code snippet using pygments. """
    return highlight(line, pyg_lexer, pyg_fmter).strip()


def is_untested(nameinfo):
    return not nameinfo['test_cnt']


def is_unused(nameinfo):
    colr_cnt = nameinfo['colr_cnt']
    tool_cnt = nameinfo['tool_cnt']
    if (colr_cnt < 3) and (colr_cnt and not tool_cnt):
        return True
    elif (tool_cnt < 3) and (tool_cnt and not colr_cnt):
        return True
    return False


def is_test_dep(nameinfo):
    return nameinfo['test_cnt'] > 2


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


def print_full(info):
    for name in sorted(info):
        nameinfo = info[name]
        total = nameinfo['total']
        print(f'{CName(name):<30}: total: {CTotal(total)} ')
        for filepath in sorted(nameinfo['files']):
            fileinfo = nameinfo['files'][filepath]
            filecnt = fileinfo['count']
            filename = os.path.split(filepath)[-1]
            print(f'    {CFile(filename):<26}: {CNum(filecnt)}')
            for line in fileinfo['lines']:
                print(f'{" " * 30}{highlight_line(line.strip())}')
    return 1 if info else 0


def print_raw(info):
    print(json.dumps(info, sort_keys=True, indent=4))
    return 1 if info else 0


def print_simple(info):
    def fmt_lbl(lbl, val):
        val_args = {
            'total': {'fore': 'yellow', 'style': 'bright'},
            'colr': {'fore': 'blue'},
            'tool': {
                'fore': 'red' if not val else 'blue',
                'style': 'bright'
            },
            'test': (
                {'fore': 'red', 'style': 'bright'}
                if not val else
                {'fore': 'green'}
            ),
            'example': {'fore': 'cyan'},
        }.get(lbl, {'fore': 'blue', 'style': 'bright'})
        return C(': ').join(C(lbl, 'cyan'), C(val, **val_args)).ljust(11)

    for name in sorted(info):
        nameinfo = info[name]
        if is_test_dep(nameinfo):
            namefmt = CTestdep(name)
        elif is_untested(nameinfo):
            namefmt = CUntested(name)
        elif is_unused(nameinfo):
            namefmt = CUnused(name)
        else:
            namefmt = CName(name)
        print(C(' ').join(
            namefmt.rjust(30),
            fmt_lbl('total', nameinfo['total']),
            fmt_lbl('colr', nameinfo['colr_cnt']),
            fmt_lbl('tool', nameinfo['tool_cnt']),
            fmt_lbl('test', nameinfo['test_cnt']),
            fmt_lbl('example', nameinfo['example_cnt']),
        ))

    return 1 if info else 0


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
        raise InvalidArg(f'invalid pattern: {s}\n{ex}')
    return p


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return f'Invalid argument, {self.msg}'
        return 'Invalid argument!'


if __name__ == '__main__':
    try:
        mainret = main(docopt(USAGESTR, version=VERSIONSTR, script=SCRIPT))
    except InvalidArg as ex:
        print_err(ex)
        mainret = 1
    except subprocess.CalledProcessError as ex:
        if ex.stdout:
            print(ex.stdout.decode())
        mainret = 1
    except (EOFError, KeyboardInterrupt):
        print_err('\nUser cancelled.\n')
        mainret = 2
    except BrokenPipeError:
        print_err('\nBroken pipe, input/output was interrupted.\n')
        mainret = 3
    sys.exit(mainret)
