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
from collections import UserString

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

lexer_c = get_lexer_by_name('c')
lexer_json = get_lexer_by_name('js')
fmter_256 = Terminal256Formatter(bg='dark', style='monokai')

anim_frames = Frames.dots_orbit.as_rainbow()

# Pattern to find a function-like macro line.
macro_pat = re.compile(r'#define ([\w_]+)([ \t]+)?\([^\(]')

colr_auto_disable()

NAME = 'ColrC - Usage Stats'
VERSION = '0.0.3'
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
MACRO_FILES = tuple(
    os.path.join(COLRC_DIR, s)
    for s in ('colr.h', 'colr_tool.h')
)
TEST_DIR = os.path.join(COLRC_DIR, 'test')
IGNORE_TEST_FILES = ('snow', )
TEST_FILES = tuple(
    os.path.join(TEST_DIR, s)
    for s in os.listdir(TEST_DIR)
    if s.endswith(('.h', '.c')) and (not s.startswith(IGNORE_TEST_FILES))
)
TEST_MACRO_FILES = tuple(
    os.path.join(TEST_DIR, s)
    for s in os.listdir(TEST_DIR)
    if s.endswith('.h') and (not s.startswith(IGNORE_TEST_FILES))
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
    '--enable=unusedFunction',
    '--force',
    '--inconclusive',
    '--inline-suppr',
    '--error-exitcode=1',
    '-DIS_C11',
    '-D__GNUC__',
    '-DDEBUG',
    '-DCOLR_DEBUG',
    f'-I{COLRC_DIR}',
    f'-I{TEST_DIR}',
]
if os.path.exists(SUPPRESS_FILE):
    CPPCHECK_ARGS.append(f'--suppressions-list={SUPPRESS_FILE}')

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-h | -v]
        {SCRIPT} [-D] [-T] -l
        {SCRIPT} [-D] [-T] -s PATTERNS...
        {SCRIPT} [-D] [-T] [-F | -M] -N [PATTERN]
        {SCRIPT} [-D] [-T] [-F | -M] (-E | -e) [-f | -n | -r] [PATTERN]
        {SCRIPT} [-D] [-T] [-a] [-d] [-F | -M] [-f | -n | -r] [-t] [PATTERN]

    Options:
        PATTERN          : Only show names matching this regex/text pattern.
        PATTERNS         : One or more patterns to send to the search command.
        -a,--all         : Show everything cppcheck thinks is unused.
        -D,--debug       : Show more info while running.
        -d,--testdeps    : Functions with tests, or in tests are not unused.
        -E,--noexamples  : Show anything not used in the examples.
        -e,--examples    : Show anything used in the examples.
        -F,--onlyfuncs   : Use only functions, not function-like macros.
        -f,--full        : Show full info.
        -h,--help        : Show this help message.
        -l,--legend      : Print a color-code legend.
        -M,--onlymacros  : Use only function-like macros.
        -N,--listnames   : Show all function names reported by cppcheck, or
                           macro names unless -F is used.
        -n,--names       : Show just the names in the final report.
        -r,--raw         : Show raw info.
        -s,--search      : Use `ag` to search for symbols in the project.
        -T,--checktests  : Gather info about the tests, not the project.
        -t,--untested    : Show untested functions.
        -v,--version     : Show version.
"""

# TODO: -j,--json           : Load info from a json file for testing/dev.
# TODO: -o file,--out file  : Write raw info to json file. (print_raw() > file)


def rgb(r, g, b):
    """ This simply allows my editor to trigger the css color-picker,
        by wrapping the r,g, and b values in a css-like rgb color.
    """
    return r, g, b


CFile = Preset(fore='cyan')
CName = Preset(fore=rgb(77, 88, 237))
CMacro = Preset(fore=rgb(100, 149, 237))
CTestdep = Preset(fore=rgb(255, 128, 64))
CMacroTestdep = Preset(fore=rgb(246, 112, 255))
CUntested = Preset(fore=rgb(200, 66, 66), style='bright')
CMacroUntested = Preset(fore=rgb(141, 92, 200), style='bright')
CUnused = Preset(fore=rgb(200, 93, 93))
CMacroUnused = Preset(fore=rgb(141, 92, 200))
CNum = Preset(fore='blue', style='bright')
CTotal = Preset(fore='yellow')
CLineNum = Preset(fore=rgb(119, 201, 255))

legend = {
    'func': {
        'Function': CName,
        'Function Test Depencency': CTestdep,
        'Unused/Untested Function': CUntested,
        'Unused Function': CUnused,
    },
    'macro': {
        'Macro': CMacro,
        'Macro Test Dependency': CMacroTestdep,
        'Unused/Untested Macro': CMacroUntested,
        'Unused Macro': CMacroUnused,
    },
}


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    if argd['--legend']:
        return print_legend()
    elif argd['--search']:
        return search(argd['PATTERNS'])

    pat = try_repat(argd['PATTERN'])
    if argd['--onlymacros']:
        names = get_macro_names(pat=pat, use_tests=argd['--checktests'])
    elif argd['--onlyfuncs']:
        names = get_cppcheck_names(pat=pat, use_tests=argd['--checktests'])
        if not names:
            print_err('No unused functions reported by cppcheck.')
            return 1
    else:
        names = get_cppcheck_names(pat=pat, use_tests=argd['--checktests'])
        names.extend(get_macro_names(pat=pat, use_tests=argd['--checktests']))
    if not names:
        print_err('No names to use.')
        return 1

    if argd['--listnames']:
        return print_names(names)

    filepaths = []
    if argd['--checktests']:
        filepaths.extend(TEST_FILES)
    else:
        filepaths.extend(COLRC_FILES)
        filepaths.extend(TOOL_FILES)
        filepaths.extend(TEST_FILES)
        filepaths.extend(EXAMPLE_FILES)
    info = check_files(filepaths, names)
    if argd['--examples'] or argd['--noexamples']:
        info = filter_examples(info, with_examples=argd['--examples'])

    if not argd['--all']:
        # all macro names are gathered, not all of them are unused.
        info = filter_used_macros(info)
        # cppcheck has a lot of false positives, do some more filtering.
        info = filter_used(
            info,
            untested=argd['--untested'],
            test_deps=argd['--testdeps'],
        )
    if argd['--full']:
        ret = print_full(info)
    elif argd['--names']:
        ret = print_names(sorted(info))
    elif argd['--raw']:
        ret = print_raw(info)
    else:
        ret = print_simple(info)
    if not (argd['--raw'] or argd['--names']):
        print_footer(argd, info)

    return ret


def check_file(filepath, names):
    """ Look for names in a file. """
    counts = {}
    in_comment = False
    try:
        with open(filepath, 'r') as f:
            for i, line in enumerate(f):
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
                    namestr = name.data
                    if namestr not in line:
                        continue
                    presubpat = re.compile(f'[\\w_]{namestr}')
                    if namestr not in presubpat.sub('', line):
                        continue
                    sufsubpat = re.compile(f'{namestr}[\\w_]')
                    if namestr not in sufsubpat.sub('', line):
                        continue
                    counts.setdefault(name, {'count': 0, 'lines': []})
                    counts[name]['count'] += line.count(namestr)
                    counts[name]['lines'].append(Line(i + 1, line))
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
            filename = os.path.split(filepath)[-1]
            counts[name]['files'][filename] = {
                'count': fileinfo['count'],
                'lines': fileinfo['lines'],
            }
            counts[name]['total'] += fileinfo['count']

    for name, nameinfo in counts.items():
        for filename, fileinfo in nameinfo['files'].items():
            filecount = fileinfo['count']
            if filename.startswith('test_'):
                counts[name]['test_cnt'] += filecount
            elif filename.endswith('_example.c'):
                counts[name]['example_cnt'] += filecount
            elif filename.endswith(('_tool.c', '_tool.h')):
                counts[name]['tool_cnt'] += filecount
            elif filename.startswith('colr'):
                counts[name]['colr_cnt'] += filecount
    return counts


def filter_examples(info, with_examples=True):
    keep = {}
    for name in sorted(info):
        nameinfo = info[name]
        if with_examples:
            if is_example(name, nameinfo):
                keep[name] = nameinfo
                continue
        elif not is_example(name, nameinfo):
            keep[name] = nameinfo
            continue
    return keep


def filter_used(info, untested=False, test_deps=False):
    keep = {}
    for name, nameinfo in info.items():
        if untested:
            if is_untested(name, nameinfo):
                keep[name] = nameinfo
            continue
        if is_unused(name, nameinfo):
            if test_deps and not is_test_dep(name, nameinfo):
                continue
            keep[name] = nameinfo
            continue

    return keep


def filter_used_macros(info):
    keep = {}
    for name, nameinfo in info.items():
        if not isinstance(name, MacroName):
            keep[name] = nameinfo
            debug(f'Not a macro: {name}')
            continue
        if is_unused(name, nameinfo):
            keep[name] = nameinfo
            debug(f'Unused Macro: {name}')
            continue
    debug(f'Filtered used macros: {len(info) - len(keep)}/{len(info)}')
    return keep


def format_name(name, nameinfo):
    if is_test_dep(name, nameinfo):
        return name.fmt_testdep()
    elif is_untested(name, nameinfo):
        return name.fmt_untested()
    elif is_unused(name, nameinfo):
        return name.fmt_unused()

    return C(name)


def get_cppcheck_names(pat=None, use_tests=False):
    cmd = ['cppcheck']
    cmd.extend(CPPCHECK_ARGS)
    if use_tests:
        cmd.extend(s for s in TEST_FILES if s.endswith('.c'))
        cls = TestFunctionName
    else:
        cmd.extend(s for s in COLRC_FILES if s.endswith('.c'))
        cmd.extend(s for s in TOOL_FILES if s.endswith('.c'))
        cls = FunctionName
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
            debug(f'cppcheck: {line.decode()}')
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
            names.append(cls(name))
    return names


def get_file_macros(filepath):
    total = 0
    try:
        with open(filepath, 'r') as f:
            for line in f:
                match = macro_pat.search(line)
                if match is None:
                    continue
                yield match.groups()[0]
                total += 1
        debug(f'Macros found: {total} - {filepath}')
    except FileNotFoundError:
        print_err(f'File not found: {filepath}')
    except EnvironmentError as ex:
        print_err(f'Can\'t read file: {filepath}\n{ex}')


def get_macro_names(pat=None, use_tests=False):
    cls = TestMacroName if use_tests else MacroName
    names = []
    for filepath in TEST_MACRO_FILES if use_tests else MACRO_FILES:
        for name in get_file_macros(filepath):
            if (pat is not None) and (pat.search(name) is None):
                continue
            names.append(cls(name))
    return names


def highlight_code(line, lexer=lexer_c):
    """ Highlight a C code snippet using pygments. """
    return highlight(line, lexer, fmter_256).strip()


def is_example(name, nameinfo):
    return nameinfo['example_cnt'] > 0


def is_untested(name, nameinfo):
    if isinstance(name, TestFunctionName):
        return nameinfo['test_cnt'] < 3
    elif isinstance(name, TestMacroName):
        return nameinfo['test_cnt'] < 2
    return not nameinfo['test_cnt']


def is_unused(name, nameinfo):
    if isinstance(name, TestFunctionName):
        return nameinfo['test_cnt'] < 3
    elif isinstance(name, TestMacroName):
        return nameinfo['test_cnt'] < 2

    colr_cnt = nameinfo['colr_cnt']
    tool_cnt = nameinfo['tool_cnt']
    if (colr_cnt < 3) and (colr_cnt and not tool_cnt):
        return True
    elif (tool_cnt < 3) and (tool_cnt and not colr_cnt):
        return True
    return False


def is_used(name, nameinfo):
    return not is_unused(name, nameinfo)


def is_test_dep(name, nameinfo):
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


def print_footer(argd, info):
    namelen = len(info)
    if argd['--onlymacros']:
        plural = 'macro' if namelen == 1 else 'macros'
    elif argd['--onlyfuncs']:
        plural = 'function' if namelen == 1 else 'functions'
    else:
        plural = 'function/macro' if namelen == 1 else 'functions/macros'
    method = 'unused'
    if argd['--all']:
        method = 'unused'
        if not argd['--onlymacros']:
            plural = f'{plural} (some reported by cppcheck)'
    elif argd['--untested']:
        method = 'untested'
    elif argd['--testdeps']:
        method = 'unused test dependencies'
    if argd['--examples']:
        plural = f'{plural} with examples'
    elif argd['--noexamples']:
        plural = f'{plural} without examples'
    print(f'\nFound {CNum(namelen)} possibly {method} {plural}.')


def print_full(info):
    colwidth = 30
    for name in sorted(info):
        nameinfo = info[name]
        total = nameinfo['total']
        print(f'{C(name):<{colwidth}}  {CTotal(total)} ')
        for filepath in sorted(nameinfo['files']):
            fileinfo = nameinfo['files'][filepath]
            filecnt = fileinfo['count']
            filename = os.path.split(filepath)[-1]
            print(f'    {CFile(filename):>{colwidth - 4}}: {CNum(filecnt)}')
            for line in fileinfo['lines']:
                print(f'{" " * colwidth}{line.highlighted()}')
    return 1 if info else 0


def print_legend():
    print('Current Color Code:')
    for nametype in sorted(legend):
        for desc in sorted(legend[nametype]):
            cpreset = legend[nametype][desc]
            print(f'    {cpreset(desc)}')
    return 0


def print_names(names):
    for name in sorted(names):
        print(f'{C(name)}')
    return 0 if names else 1


def print_raw(info):
    fixed = {}
    for name in info:
        nameinfo = info[name]
        for filepath in nameinfo['files']:
            fileinfo = nameinfo['files'][filepath]
            fileinfo['lines'] = [l.as_tuple() for l in fileinfo['lines']]
            nameinfo['files'][filepath] = fileinfo
        fixed[str(name)] = nameinfo

    rawjson = json.dumps(
        fixed,
        sort_keys=True,
        indent=4
    )
    if sys.stdout.isatty():
        infostr = highlight_code(
            rawjson,
            lexer=lexer_json,
        )
    else:
        infostr = rawjson

    print(infostr)
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
        namefmt = format_name(name, nameinfo)
        print(C(' ').join(
            namefmt.ljust(30),
            fmt_lbl('total', nameinfo['total']),
            fmt_lbl('colr', nameinfo['colr_cnt']),
            fmt_lbl('tool', nameinfo['tool_cnt']),
            fmt_lbl('test', nameinfo['test_cnt']),
            fmt_lbl('example', nameinfo['example_cnt']),
        ))

    return 1 if info else 0


def search(patterns):
    """ Run `ag --cc <pattern> <project_dir>`. """
    pattern = '|'.join(f'({s}\\b)' for s in patterns)
    cmd = ['ag', '--cc', pattern, COLRC_DIR]
    debug(f'Running: {" ".join(cmd)}')
    return subprocess.check_call(cmd)


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


class Line(object):
    def __init__(self, linenum, text):
        self.linenum = linenum or 0
        self.text = text
        self.stripped = text.strip()

    def __str__(self):
        return f'{self.linenum:>5}: {self.stripped}'

    def highlighted(self):
        return C(': ').join(
            CLineNum(self.linenum),
            highlight_code(self.stripped)
        )

    def as_tuple(self):
        return (self.linenum, self.text)


class Name(UserString):
    def __colr__(self):
        return C(self.data)

    def fmt_testdep(self):
        raise NotImplementedError('This method should be overridden.')

    def fmt_untested(self):
        raise NotImplementedError('This method should be overridden.')

    def fmt_unused(self):
        raise NotImplementedError('This method should be overridden.')


class FunctionName(Name):
    def __colr__(self):
        return CName(self.data)

    def fmt_testdep(self):
        return CTestdep(self)

    def fmt_untested(self):
        return CUntested(self)

    def fmt_unused(self):
        return CUnused(self)


class TestFunctionName(FunctionName):
    pass


class MacroName(Name):
    def __colr__(self):
        return CMacro(self.data)

    def fmt_testdep(self):
        return CMacroTestdep(self)

    def fmt_untested(self):
        return CMacroUntested(self)

    def fmt_unused(self):
        return CMacroUnused(self)


class TestMacroName(MacroName):
    pass


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
