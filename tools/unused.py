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
import stat
import sys
from collections import (
    UserDict,
    UserList,
    UserString,
)
from functools import total_ordering

from colr import (
    AnimatedProgress,
    Colr as C,
    Frames,
    Preset,
    auto_disable as colr_auto_disable,
    docopt,
)
from outputcatcher import (
    ProcessOutput,
)
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
VERSION = '0.0.6'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

COLRC_DIR = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
COLRC_FILES = tuple(
    os.path.join(COLRC_DIR, s)
    for s in ('colr.h', 'colr.c')
)
COLRC_MAIN_FILE = os.path.join(COLRC_DIR, 'colr.c')

TOOL_FILES = tuple(
    os.path.join(COLRC_DIR, s)
    for s in ('colr_tool.h', 'colr_tool.c')
)
TOOL_MAIN_FILE = os.path.join(COLRC_DIR, 'colr_tool.c')

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
TEST_MAIN_FILE = os.path.join(TEST_DIR, 'test_ColrC.c')
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
# File to hold `get_cppcheck_names()` cached results.
CPPCHECK_CACHE = os.path.join(SCRIPTDIR, 'cppcheck.cached.json')
CPPCHECK_TEST_CACHE = os.path.join(SCRIPTDIR, 'cppcheck.cached.test.json')

# Sort keys for Names.
SORT_KEYS = {
    None: {'func': str, 'desc': 'name'},
    'colr': {
        'func': lambda name: name.info['colr_cnt'],
        'desc': 'colr.c usage',
    },
    'example': {
        'func': lambda name: name.info['example_cnt'],
        'desc': 'example usage',
    },
    'file': {
        'func': lambda name: name.info['files'],
        'desc': 'file list',
    },
    'name': {
        'func': str,
        'desc': 'name',
    },
    'label': {
        'func': lambda name: str(name.label()),
        'desc': 'label',
    },
    'test': {
        'func': lambda name: name.info['test_cnt'],
        'desc': 'test usage',
    },
    'tool': {
        'func': lambda name: name.info['tool_cnt'],
        'desc': 'colr-tool usage',
    },
    'total': {
        'func': lambda name: name.info['total'],
        'desc': 'total usage',
    },
    'type': {
        'func': lambda name: type(name).__name__,
        'desc': 'type',
    },
}
ACCEPTED_SORT_KEYS = ', '.join(sorted(s for s in SORT_KEYS if s))
# Sort key aliases.
SORT_KEYS['c'] = SORT_KEYS['colr']
SORT_KEYS['e'] = SORT_KEYS['example']
SORT_KEYS['x'] = SORT_KEYS['example']
SORT_KEYS['f'] = SORT_KEYS['file']
SORT_KEYS['n'] = SORT_KEYS['name']
SORT_KEYS['l'] = SORT_KEYS['label']
SORT_KEYS['t'] = SORT_KEYS['test']
SORT_KEYS['files'] = SORT_KEYS['file']

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-h | -v]
        {SCRIPT} [-D] [-f] -b
        {SCRIPT} [-D] -l
        {SCRIPT} [-D] (-c | -M) [-T] [-F] [-f | -n | -r] [-S key]
                 [-a] [-d] [-t] [-s | PATTERN]
        {SCRIPT} [-D] [-j file | -T] [-F | -M] -o file
        {SCRIPT} [-D] [-j file | -T] -s PATTERNS...
        {SCRIPT} [-D] [-j file | -T] [-F | -M] -N [-s | PATTERN]
        {SCRIPT} [-D] [-j file | -T] [-F | -M] [-f | -n | -r] [-S key]
                 (-E | -e) [-s | PATTERN]
        {SCRIPT} [-D] [-j file | -T] [-F | -M] [-f | -n | -r] [-S key]
                 [-a] [-d] [-t] [-s | PATTERN]

    Options:
        PATTERN              : Only show names matching this regex/text pattern.
        PATTERNS             : One or more function/macro names for the search
                               command.
                               Without any other arguments, this is just like
                               running `ag --cc (PATTERN\\b)`.
        -a,--all             : Show everything cppcheck thinks is unused.
        -b,--badtests        : Show bad tests. Use with -f to show actual code.
        -c,--cfuncs          : Use `cfuncs` command to gather names.
        -D,--debug           : Show more info while running.
        -d,--testdeps        : Show test dependencies.
        -E,--noexamples      : Show anything not used in the examples.
        -e,--examples        : Show anything used in the examples.
        -F,--onlyfuncs       : Use only functions, not function-like macros.
        -f,--full            : Show full info.
        -h,--help            : Show this help message.
        -j file,--json file  : Load info from a JSON file.
        -l,--legend          : Print a color-code legend.
        -M,--onlymacros      : Use only function-like macros.
        -N,--listnames       : Show all function names reported by cppcheck, or
                               macro names unless -F is used.
                               This is like -a -n.
        -n,--names           : Show just the names in the final report.
        -o file,--out file   : Write info in JSON format to a file.
                               This is like --all and --raw.
        -r,--raw             : Show raw info.
        -S key,--sortby key  : Sort names. Can be one of:
                               {ACCEPTED_SORT_KEYS}
                               Names are always sorted alphabetically before
                               applying a sort key.
        -s,--search          : Use `ag` to search for symbols in the project.
                               When combined with other options, all listed
                               names are searched for.
        -T,--checktests      : Gather info about the tests, not the project.
        -t,--untested        : Show untested functions.
        -v,--version         : Show version.
"""


def rgb(r, g, b):
    """ This simply allows my editor to trigger the css color-picker,
        by wrapping the r,g, and b values in a css-like rgb color.
    """
    return r, g, b


# Some Colr presets, so changing "themes" can be done in one place.
CFile = Preset(fore='cyan')
CName = Preset(fore=rgb(77, 88, 237))
CMacro = Preset(fore=rgb(100, 149, 237))
CFalseName = Preset(fore=rgb(0, 121, 0))
CFalseMacro = Preset(fore=rgb(95, 163, 0))
CTestDep = Preset(fore=rgb(255, 128, 64))
CMacroTestDep = Preset(fore=rgb(246, 112, 255))
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
        'Function False Positive': CFalseName,
        'Function Test Depencency': CTestDep,
        'Unused/Untested Function': CUntested,
        'Unused Function': CUnused,
    },
    'macro': {
        'Macro': CMacro,
        'Macro False Positive': CFalseMacro,
        'Macro Test Dependency': CMacroTestDep,
        'Unused/Untested Macro': CMacroUntested,
        'Unused Macro': CMacroUnused,
    },
}


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    if argd['--legend']:
        return print_legend()
    elif argd['--search'] and argd['PATTERNS']:
        return search(argd['PATTERNS'])
    elif argd['--badtests']:
        return print_bad_tests(code=argd['--full'])

    pat = try_repat(argd['PATTERN'])
    if argd['--json']:
        info = UnusedInfo.from_json_file(argd['--json'])
        info.filter_pattern(pat)
        if argd['--listnames']:
            return print_names(info)
        if argd['--onlyfuncs']:
            info = info.only_functions()
        elif argd['--onlymacros']:
            info = info.only_macros()
    else:
        if argd['--cfuncs']:
            if argd['--onlyfuncs'] and argd['--onlymacros']:
                # This is just bad arg handling in the docopt usage.
                argd['--onlyfuncs'] = argd['--onlymacros'] = False
            names = get_cfuncs_names(
                pat=pat,
                use_tests=argd['--checktests'],
                untested=argd['--untested'],
            )
            if not argd['--onlyfuncs']:
                names.extend(
                    get_macro_names(pat=pat, use_tests=argd['--checktests'])
                )
        elif argd['--onlymacros']:
            names = get_macro_names(pat=pat, use_tests=argd['--checktests'])
        elif argd['--onlyfuncs']:
            names = get_cppcheck_cached(pat=pat, use_tests=argd['--checktests'])
            if not names:
                print_err('No unused functions reported by cppcheck.')
                return 1
        else:
            names = get_cppcheck_cached(pat=pat, use_tests=argd['--checktests'])
            names.extend(
                get_macro_names(pat=pat, use_tests=argd['--checktests'])
            )
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
        info = UnusedInfo(check_files(filepaths, names))

    if argd['--examples'] or argd['--noexamples']:
        info.filter_examples(with_examples=argd['--examples'])

    if not info:
        print_err('No info to use!')
        return 1

    if not (argd['--all'] or argd['--out']):
        # all macro names are gathered, not all of them are unused.
        info.filter_used_macros()
        # cppcheck has a lot of false positives, do some more filtering.
        info.filter_used(
            untested=argd['--untested'],
            test_deps=argd['--testdeps'],
        )
    elif argd['--all'] or argd['--out']:
        # Keep false-positives, but mark them as such.
        info.filter_used(mark_only=True)

    sort_key = name_sort_key(sort_by=argd['--sortby'])

    if argd['--full']:
        ret = print_full(info, sort_key=sort_key)
    elif argd['--names']:
        ret = print_names(info, sort_key=sort_key)
    elif argd['--out']:
        ret = print_file(info, argd['--out'])
    elif argd['--raw']:
        ret = json_write(info)
    elif argd['--search']:
        return info.search()
    else:
        ret = print_simple(info, sort_key=sort_key)
    if not (argd['--raw'] or argd['--names'] or argd['--out']):
        print_footer(argd, info)

    return ret


def cache_cppcheck_names(names):
    """ Save cppcheck names to the cache file. """
    if not names:
        debug('No names to cache!')
        return False
    cache_file = CPPCHECK_CACHE
    if isinstance(names[0], (TestFunctionName, TestMacroName)):
        cache_file = CPPCHECK_TEST_CACHE
        debug(f'Using test cache file: {cache_file}')
    else:
        debug(f'Using cache file: {cache_file}')

    data = [n.as_json_obj() for n in names]
    try:
        with open(cache_file, 'w') as f:
            json.dump(data, f, sort_keys=True, indent=4)
    except EnvironmentError as ex:
        debug_err(f'Unable to save cache file: {cache_file}\n{ex}')
        return False
    except ValueError as ex:
        debug_err(f'Unable to serialize cache: {ex}')
        return False
    debug(f'cppcheck results were cached: {cache_file}')
    return True


def cache_old(use_tests=False):
    """ Returns True if the cppcheck names cache is old. """
    files = []
    cache_file = CPPCHECK_CACHE
    if use_tests:
        files.extend(s for s in TEST_FILES if s.endswith('.c'))
        cache_file = CPPCHECK_TEST_CACHE
    else:
        files.extend(s for s in COLRC_FILES if s.endswith('.c'))
        files.extend(s for s in TOOL_FILES if s.endswith('.c'))

    is_old = (not file_newer(cache_file, files))
    debug(f'Cache is {"old" if is_old else "good"}: {cache_file}')
    return is_old


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
    counts = set()
    # Get info for each file, the easy part.
    for filepath in filepaths:
        filecounts = check_file(filepath, names)
        if not filecounts:
            continue
        # Reorganize into a dict of names with file references and counts.
        for name in filecounts:
            fileinfo = filecounts[name]
            filename = os.path.split(filepath)[-1]
            name.info['files'][filename] = {
                'count': fileinfo['count'],
                'lines': fileinfo['lines'],
            }
            name.info['total'] += fileinfo['count']
            counts.add(name)

    # Get general counts for tests, examples, colr-tool, and colr.c.
    for name in counts:
        name.set_counts()
    return counts


def file_newer(filepath, others):
    """ Returns True if `filepath` is a newer file than all the other files
        in `others`.
        On error, a debug message is printed and the "bad" file is skipped.
        If `filepath` itself errors, `False` is returned.
    """
    if not os.path.exists(filepath):
        return False

    if isinstance(others, str):
        others = [others]
    try:
        fstat = os.stat(filepath)
    except EnvironmentError as ex:
        debug(f'Stat failed for main file: {filepath}\n{ex}')
        return False

    for other in others:
        try:
            otherstat = os.stat(other)
        except EnvironmentError as ex:
            debug(f'Stat failed for check file: {other}\n{ex}')
            continue
        if otherstat.st_mtime > fstat.st_mtime:
            debug(f'Other file is newer: {other}')
            return False
    return True


def get_cfuncs_names(pat=None, use_tests=False, untested=False):
    exe = get_command('cfuncs')
    if not exe:
        raise FatalError('missing `cfuncs` command/script.')
    cmd = [exe, '-m', '-q']
    if use_tests:
        cmd.append(TEST_MAIN_FILE)
    else:
        cmd.append(COLRC_MAIN_FILE)
    if untested or use_tests:
        cmd.append(TEST_DIR)
    else:
        cmd.append(TOOL_MAIN_FILE)
    proc = ProcessOutput(cmd)
    names = []
    for line in proc.iter_stdout():
        name = line.decode().strip()
        if pat and (pat.search(name) is None):
            continue
        names.append(FunctionName(name))
    return names


def get_command(cmdname):
    """ Look for `cmdname` executable in $PATH.
        Returns the full path if the command can be found, otherwise returns
        None.
    """
    for trypath in get_env_path():
        fullpath = os.path.join(trypath, cmdname)
        try:
            st = os.stat(fullpath)
        except FileNotFoundError:
            continue
        if st.st_mode & stat.S_IEXEC:
            debug(f'Found executable: {fullpath}')
            return fullpath
    return None


def get_cppcheck_cached(pat=None, use_tests=False):
    if cache_old(use_tests=use_tests):
        # Gonna have to call cppcheck for this.
        return get_cppcheck_names(pat=pat, use_tests=use_tests)
    # Load names from disk.
    cache_file = CPPCHECK_TEST_CACHE if use_tests else CPPCHECK_CACHE
    try:
        with open(cache_file, 'r') as f:
            data = json.load(f)
    except EnvironmentError as ex:
        debug_err(f'Unable to read cache file: {cache_file}\n{ex}')
        return get_cppcheck_names(pat=pat, use_tests=use_tests)
    except ValueError as ex:
        debug_err(f'Unable to deserialize cache: {cache_file}\n{ex}')
        return get_cppcheck_names(pat=pat, use_tests=use_tests)

    names = []
    for nameinf in data:
        name = nameinf['name']
        if pat and (pat.search(name) is None):
            continue
        names.append(Name.from_json_obj(nameinf))

    return names


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
    # Cache the results for later.
    cache_cppcheck_names(names)
    return names


def get_env_path():
    """ Return $PATH as a list of paths. """
    p = [
        s
        for s in os.environ.get('PATH', '').split(os.pathsep)
        if s
    ]
    if not p:
        raise FatalError('no paths in environment variable: PATH')
    return p


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


def get_test_info():
    """ Get test files, describe(), subdesc(), and it() info. """
    info = {}
    for filepath in TEST_FILES:
        if filepath.endswith('.h'):
            # Only gathering implementations, not definitions.
            continue
        info[filepath] = TestFile.from_file(filepath)
    return info


def highlight_code(line, lexer=lexer_c, indent=0):
    """ Highlight a C code snippet using pygments. """
    code = highlight(line, lexer, fmter_256).strip()
    if indent:
        spaces = ' ' * indent
        code = '{}{}'.format(
            spaces,
            f'\n{spaces}'.join(code.split('\n'))
        )
    return code


def json_write(info, fd=sys.stdout):
    rawjson = info.as_json()
    if fd.isatty():
        infostr = highlight_code(
            rawjson,
            lexer=lexer_json,
        )
    else:
        infostr = rawjson

    print(infostr, file=fd)
    return 1 if info else 0


def name_sort_key(sort_by='name'):
    sort_by = sort_by.strip().lower()
    keyinfo = SORT_KEYS.get(sort_by, None)
    if keyinfo is None:
        accepted = ACCEPTED_SORT_KEYS
        msg = f'Expecting one of ({accepted}), got: {sort_by}'
        raise InvalidArg(msg)
    debug(f'Sorting by: {keyinfo["desc"]}')
    return keyinfo['func']


def parse_test_desc(line):
    """ Parses a name from a subdesc() or describe() line. """
    if not (('(' in line) and (')' in line)):
        raise ValueError(f'Not a snow.h macro call: {line}')
    _, _, name = line.partition('(')
    name, _, _ = name.partition(')')
    if name.startswith('"') and name.endswith('"'):
        name = name[1:-1]
    return name


def print_bad_tests(code=False):
    """ Print bad test info. """
    info = get_test_info()
    badinfo = {k: v.bad_tests() for k, v in info.items() if v.has_bad_tests()}
    if not badinfo:
        print('No bad tests found :).')
        return 0

    ttl = 0
    for filepath in sorted(badinfo):
        testfile = badinfo[filepath]
        ttl += testfile.test_count()
        print('\n', testfile.formatted(code=code, color=sys.stdout.isatty()))

    print(f'\nPossibly bad tests: {CNum(ttl)}')
    return ttl


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


def print_file(info, filepath):
    """ Write raw info to a file. """
    try:
        with open(filepath, 'w') as f:
            json_write(info, fd=f)
    except EnvironmentError as ex:
        print_err(f'\nCan\'t write to file: {filepath}\n{ex}')
        return 1
    print(C(': ').join('Wrote info to', CFile(filepath)))
    return 0


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
            reporter = 'cfuncs' if argd['--cfuncs'] else 'cppcheck'
            plural = f'{plural} (some reported by {reporter})'
    elif argd['--untested']:
        method = 'untested'
    elif argd['--testdeps']:
        method = 'unused test dependencies'
    if argd['--examples']:
        plural = f'{plural} with examples'
    elif argd['--noexamples']:
        plural = f'{plural} without examples'
    print(f'\nFound {CNum(namelen)} possibly {method} {plural}.')


def print_full(info, sort_key=str):
    colwidth = 30
    for name in sort_names(info, key=sort_key):
        total = name.info['total']
        print(f'{C(name):<{colwidth}}  {CTotal(total)} ')
        for filepath in sorted(name.info['files']):
            fileinfo = name.info['files'][filepath]
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


def print_names(names, sort_key=str):
    for name in sort_names(names, key=sort_key):
        print(f'{C(name)}')
    return 0 if names else 1


def print_simple(info, sort_key=str):
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

    name_len = len(max(info, key=len))
    for name in sort_names(info, key=sort_key):
        namefmt = name.fmt()
        print(C(' ').join(
            namefmt.ljust(name_len),
            fmt_lbl('total', name.info['total']),
            fmt_lbl('colr', name.info['colr_cnt']),
            fmt_lbl('tool', name.info['tool_cnt']),
            fmt_lbl('test', name.info['test_cnt']),
            fmt_lbl('example', name.info['example_cnt']),
            C(' ').join('-', name.fmt_class()(name.label())),
        ))

    return 1 if info else 0


def search(patterns):
    """ Run `ag --cc <pattern> <project_dir>`. """
    pattern = '|'.join(f'({s}\\b)' for s in patterns)
    cmd = ['ag', '--cc', pattern, COLRC_DIR]
    debug(f'Running: {" ".join(cmd)}')
    return subprocess.check_call(cmd)


def sort_names(names, key=str):
    # Always sort by name.
    lst = sorted(names, key=str)
    if key.__name__ == 'str':
        return lst
    return sorted(lst, key=key)


def strip_c_line(line):
    """ Strips spaces, newlines, tabs, and comments from a line of C code.
    """
    if '//' in line:
        line, _, _ = line.partition('//')
    return line.strip()


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


class DebugFilterOp(object):
    def __init__(self, filtertype, obj):
        self.type = filtertype
        self.obj = obj
        self.length = len(obj)

    def __enter__(self):
        debug(f'Filtering {self.type} ({len(self.obj)})...', level=1)
        return self

    def __exit__(self, exc, typ, tb):
        filtered = self.length - len(self.obj)
        debug(f' Filtered {self.type} ({filtered}/{self.length})', level=1)
        return False


@total_ordering
class Files(UserDict):
    """ A collection of file names with Lines and counts for each file.
        Each Name has one or more file references. This is a collection of
        those references.
    """
    def __eq__(self, other):
        return self.data == other.data

    def __lt__(self, other):
        return list(self.data) < list(other.data)

    def as_json_obj(self):
        d = {}
        for filename, fileinfo in self.items():
            fileinfo['lines'] = [
                l.as_json_obj()
                for l in fileinfo['lines']
            ]
            d[filename] = fileinfo
        return d

    @classmethod
    def from_json_obj(cls, o):
        if not isinstance(o, dict):
            raise TypeError(f'Expecting dict, got: {type(o).__name__}')
        d = {k: v for k, v in o.items()}
        for filename, fileinfo in d.items():
            fileinfo['lines'] = [
                Line.from_json_obj(l)
                for l in fileinfo['lines']
            ]
            d[filename] = fileinfo
        return cls(d)


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return f'Invalid argument, {self.msg}'
        return 'Invalid argument!'


class InvalidJSON(InvalidArg):
    """ Raised when JSON info cannot be read or decoded. """
    def __init__(self, msg=None, filepath=None):
        self.msg = msg or ''
        self.filepath = filepath

    def __str__(self):
        msg = f'Cannot load JSON'
        if self.filepath:
            msg = ' '.join((msg, f'from: {self.filepath}'))
        if self.msg:
            return f'{msg}\n{self.msg}'
        return msg


class FatalError(InvalidArg):
    """ Raised when the program should exit immediately. """
    def __str__(self):
        if self.msg:
            return f'Fatal error, {self.msg}'
        return 'Fatal error, cannot continue.'


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

    def as_json_obj(self):
        return (self.linenum, self.text)

    @classmethod
    def from_json_obj(cls, o):
        if not isinstance(o, (list, tuple)):
            raise TypeError(f'Expecting list/tuple, got: {type(o).__name__}')
        if len(o) != 2:
            raise TypeError(f'Expecting [linenum, text], got: {o!r}')
        return cls(*o)


class Name(UserString):
    # Overridden in subclasses:
    fmt_classes = {}
    # Marked as false positive in UnusedInfo.filter_used()
    false_positive = False

    def __init__(self, o):
        super().__init__(o)
        self.info = {
            'total': 0,
            'files': Files(),
            'colr_cnt': 0,
            'tool_cnt': 0,
            'test_cnt': 0,
            'example_cnt': 0,
        }

    def __colr__(self):
        return C(self.data)

    def as_json(self, sort_keys=True, indent=4):
        """ Return this Name instance as a JSON string. """
        return json.dumps(
            self.as_json_obj(),
            sort_keys=sort_keys,
            indent=indent,
        )

    def as_json_obj(self):
        """ Return a JSON-serializable dict for this Name instance. """
        inf = {k: v for k, v in self.info.items()}
        inf['files'] = self.info['files'].as_json_obj()
        return {
            'name': self.data,
            'info': inf,
            'false_positive': self.false_positive,
            'class': type(self).__name__,
        }

    def fmt_class(self):
        if not self.fmt_classes:
            raise NotImplementedError('No fmt_classes implemented.')
        if self.false_positive:
            return self.fmt_classes['false-positive']
        elif self.is_test_dep():
            return self.fmt_classes['test-dependency']
        elif self.is_untested():
            return self.fmt_classes['untested']
        elif self.is_unused():
            return self.fmt_classes['unused']
        return C

    def fmt(self):
        return self.fmt_class()(self)

    @classmethod
    def from_json_obj(cls, d):
        """ Create an initialized Name instance from a dict, usually created
            by `self.as_json_obj()`.
        """
        clses = (FunctionName, TestFunctionName, MacroName, TestMacroName)
        clsmap = {
            c.__name__: c
            for c in clses
        }
        inf = {k: v for k, v in d['info'].items()}
        try:
            files = Files.from_json_obj(inf['files'])
            inf['files'] = files
        except KeyError as ex:
            raise ValueError(f'Missing \'info\' or \'files\' key!') from ex
        try:
            newcls = clsmap[d['class']]
        except KeyError as ex:
            raise ValueError('\n'.join((
                f'Missing class for: {d["class"]!r}',
                f'Dict: {d!r}',
                f'Classes: {clsmap!r}',
            ))) from ex

        o = newcls(d['name'])
        o.info = inf
        o.false_positive = d['false_positive']
        return o

    def is_example(self):
        return self.info['example_cnt'] > 0

    def is_macro(self):
        return False

    def is_test(self):
        return False

    def is_test_dep(self):
        return self.info['test_cnt'] > 2

    def is_untested(self):
        return not self.info['test_cnt']

    def is_unused(self):
        colr_cnt = self.info['colr_cnt']
        tool_cnt = self.info['tool_cnt']
        if (colr_cnt < 3) and (colr_cnt and not tool_cnt):
            return True
        elif (tool_cnt < 3) and (tool_cnt and not colr_cnt):
            return True
        return False

    def is_used(self):
        return not self.is_unused()

    def label(self):
        if self.false_positive:
            return 'false-positive'
        elif self.is_test_dep():
            return 'test dep.'
        elif self.is_untested():
            return 'untested'
        elif self.is_unused():
            return 'unused'
        return 'normal'

    def set_counts(self):
        for filename, fileinfo in self.info['files'].items():
            filecount = fileinfo['count']
            if filename.startswith('test_'):
                self.info['test_cnt'] += filecount
            elif filename.endswith('_example.c'):
                self.info['example_cnt'] += filecount
            elif filename.endswith(('_tool.c', '_tool.h')):
                self.info['tool_cnt'] += filecount
            elif filename.startswith('colr'):
                self.info['colr_cnt'] += filecount


class FunctionName(Name):
    fmt_classes = {
        'false-positive': CFalseName,
        'test-dependency': CTestDep,
        'untested': CUntested,
        'unused': CUnused,
    }

    def __colr__(self):
        return CName(self.data)

    def label(self):
        lbl = super().label()
        return ' '.join((lbl, 'func.'))


class TestFunctionName(FunctionName):
    def is_test(self):
        return True

    def is_untested(self):
        return self.info['test_cnt'] < 3

    def is_unused(self):
        return self.info['test_cnt'] < 3

    def label(self):
        lbl = super().label()
        return ' '.join((lbl, 'test func.'))


class MacroName(Name):
    fmt_classes = {
        'false-positive': CFalseMacro,
        'test-dependency': CMacroTestDep,
        'untested': CMacroUntested,
        'unused': CMacroUnused,
    }

    def __colr__(self):
        return CMacro(self.data)

    def is_macro(self):
        return True

    def label(self):
        lbl = super().label()
        return ' '.join((lbl, 'macro'))


class TestMacroName(MacroName):
    def is_macro(self):
        return True

    def is_test(self):
        return True

    def is_untested(self):
        return self.info['test_cnt'] < 2

    def is_unused(self):
        return self.info['test_cnt'] < 2

    def label(self):
        lbl = super().label()
        return ' '.join((lbl, 'test macro'))


class TestDescribe(UserList):
    """ Holds tests/subdesc() for one describe() call (from snow.h).
    """
    def __init__(self, name, tests=None, filepath=None, linenum=0):
        super().__init__(tests if tests else [])
        self.name = name
        self.filepath = filepath
        self.linenum = linenum
        self.lines = []

    def __colr__(self):
        return C(self.formatted(color=True))

    def __repr__(self):
        return ''.join((
            f'{type(self).__name__}(',
            f'{self.name!r}, ',
            f'tests=[tests: {len(self)}]',
            ')',
        ))

    def __str__(self):
        return self.formatted(indent=0)

    def bad_tests(self):
        return self.__class__(
            self.name,
            [sd.bad_tests() for sd in self if sd.has_bad_tests()]
        )

    def debug(self, *args, **kwargs):
        kwargs['level'] = 1
        debug('  ', *args, **kwargs)

    def formatted(self, indent=0, color=False, code=True):
        spaces = ' ' * indent
        subindent = indent + 4
        name = self.name
        if color:
            name = C(name, 'yellow')
        return '\n'.join((
            f'{spaces}{name} ({len(self)}):',
            '\n'.join(
                t.formatted(indent=subindent, color=color, code=code)
                for t in self
            ),
        ))

    def has_bad_tests(self):
        return any(sd.has_bad_tests() for sd in self)

    def parse_subdescs(self):
        """ Parse self.lines to create TestSubdescs and TestIts for self.data.
        """
        self.debug(f'Parsing subdescs for: {self.linenum}: {self.name}')
        braces = 0
        started = False
        subdescs = []
        in_it = None
        in_subdesc = None
        for i, line in enumerate(self.lines):
            stripped = line.strip()
            if stripped.startswith('subdesc('):
                in_subdesc = TestSubdesc(
                    parse_test_desc(stripped),
                    filepath=self.filepath,
                )
                braces = 1
                started = True
                self.debug(f'Found subdesc: {i}: {in_subdesc.name}')
                continue
            elif stripped.startswith('it('):
                if in_subdesc is None:
                    in_it = TestIt(parse_test_desc(stripped))
                    self.debug(f'Found lone it: {i}: {in_it.desc}')
                braces += 1
            elif stripped.endswith('{'):
                if stripped.startswith('}'):
                    braces -= 1
                braces += 1
                # self.debug(f'Pushed: {braces}@{i+1}: {line.rstrip()}')
            elif stripped.endswith('}') or stripped.endswith('};'):
                if '= {' in stripped:
                    braces += 1
                braces -= 1
                # self.debug(f'Popped: {braces}@{i+1}: {line.rstrip()}')
            if (braces == 0) and started:
                if in_subdesc is not None:
                    self.debug(' '.join((
                        f'Found end of subdesc: {i}: {in_subdesc.name}',
                        f'({len(subdescs) + 1})',
                    )))
                    in_subdesc.parse_its()
                    subdescs.append(in_subdesc)
                    in_subdesc = None
                    started = False
                elif in_it is not None:
                    debug(' '.join((
                        f'Found end of lone it: {i}: {in_it.desc}',
                        f'({len(subdescs) + 1})',
                    )))
                    subdescs.append(in_it)
                    in_it = None
                    started = False
                else:
                    raise ValueError('\n'.join((
                        f'Found closing brace with no opening: {self.filepath}',
                        f'Describe: {self.name}',
                        f'Line: {i}: {line!r}',
                    )))
            elif in_subdesc is not None:
                in_subdesc.lines.append(line)
            elif in_it is not None:
                in_it.append(line)
            else:
                # Header/Comment/Blank lines...
                pass
        self.data = subdescs
        return self

    def subdescs(self):
        return [
            sd
            for sd in self
            if isinstance(sd, TestSubdesc)
        ]

    def test_count(self):
        return sum(sd.test_count() for sd in self)

    def tests(self):
        tests = []
        for sd in self:
            tests.extend(sd.tests())
        return tests


class TestFile(UserList):
    """ Holds TestDescribes from one test file. """
    def __init__(self, filepath, describes=None):
        super().__init__(describes if describes else [])
        self.filepath = filepath

    def __colr__(self):
        return C(self.formatted(color=True))

    def __repr__(self):
        return ''.join((
            f'{type(self).__name__}(',
            f'{self.filepath!r}, ',
            f'describes=[describes: {len(self)}]',
            ')',
        ))

    def __str__(self):
        return self.formatted(indent=0)

    def bad_tests(self):
        return self.__class__(
            self.filepath,
            [d.bad_tests() for d in self if d.has_bad_tests()]
        )

    def formatted(self, indent=0, color=False, code=True):
        spaces = ' ' * indent
        subindent = indent + 4
        filepath = self.filepath
        if color:
            filepath = C(filepath, 'blue', style='bright')
        return '\n'.join((
            f'{spaces}{filepath} ({len(self)}):',
            '\n'.join(
                d.formatted(indent=subindent, color=color, code=code)
                for d in self
            ),
        ))

    @classmethod
    def from_file(cls, filepath):
        """ Parse a file, and return an initialized TestFile instance. """
        describes = []
        braces = 0
        in_desc = None

        def add_desc_line(line):
            nonlocal in_desc
            if in_desc is None:
                return
            in_desc.lines.append(line.rstrip())

        with open(filepath, 'r') as f:
            debug(f'Parsing describes for: {filepath}')
            for i, line in enumerate(f):
                stripped = strip_c_line(line)
                if stripped.startswith('describe('):
                    if braces:
                        raise ValueError('\n'.join((
                            f'Found opening with no close!: {i+1}: {line}',
                            f'Braces: {braces}'
                        )))
                    in_desc = TestDescribe(
                        parse_test_desc(stripped),
                        filepath=filepath,
                        linenum=i + 1,
                    )
                    braces = 1
                    debug(f'Found describe: {i+1}: {in_desc.name}')
                    continue
                elif stripped.endswith('{'):
                    if stripped.startswith('}'):
                        braces -= 1
                    braces += 1
                    # debug(f'Pushed: {braces}@{i+1}: {line}')
                elif stripped.endswith('}') or stripped.endswith('};'):
                    if '= {' in stripped:
                        braces += 1
                    braces -= 1
                    # debug(f'Popped: {braces}@{i+1}: {line}')
                if (braces == 0) and (in_desc is not None):
                    debug(
                        f'Found end of {in_desc.name} ({len(describes) + 1}).'
                    )
                    in_desc.parse_subdescs()
                    describes.append(in_desc)
                    in_desc = None
                elif in_desc is not None:
                    add_desc_line(line)
                else:
                    # Header lines...
                    pass
        return cls(filepath, describes)

    def has_bad_tests(self):
        return any(d.has_bad_tests() for d in self)

    def is_impl(self):
        """ Returns True if this is an actual test file. """
        return len(self) > 0

    def test_count(self):
        return sum(d.test_count() for d in self)

    def tests(self):
        tests = []
        for d in self:
            tests.extend(d.tests())
        return tests


class TestIt(UserList):
    """ Holds a function (it()) name, and it's lines. """
    def __init__(self, desc, lines=None):
        super().__init__(lines if lines else [])
        self.desc = desc

    def __colr__(self):
        return C(self.formatted(color=True))

    def __repr__(self):
        return ''.join((
            f'{type(self).__name__}(',
            f'{self.desc!r}, ',
            f'lines=[lines: {len(self)}]',
            ')',
        ))

    def __str__(self):
        return self.formatted(indent=0)

    def bad_tests(self):
        """ Compatible with TestSubdescs.bad_tests(), for lone it() calls.
            Technically, a lone it() should be considered bad.
        """
        if self.is_bad():
            return [self]
        return []

    def formatted(self, indent=0, color=False, code=True):
        spaces = ' ' * indent
        codeindent = indent + 4
        desc = self.desc
        if color:
            desc = C(desc, 'cyan')
        if not code:
            return f'{spaces}{desc}'
        codelines = '\n'.join(self.formatted_lines(indent=codeindent))
        if color:
            codelines = highlight_code(codelines)
        return '\n'.join((
            f'{spaces}{desc}:',
            codelines
        ))

    def formatted_lines(self, indent=0):
        if not self.lines:
            return []
        s = self.lines[0]
        cnt = 0
        while s.startswith(' '):
            cnt += 1
            s = s[1:]
        spaces = ' ' * indent
        return [f'{spaces}{l[cnt:]}' for l in self.lines]

    def has_bad_tests(self):
        """ Compatible with TestSubdesc.has_bad_tests(), for lone it() tests.
        """
        return self.is_bad()

    def is_bad(self):
        return (not self.is_good())

    def is_good(self):
        return (len(self) > 5) and (len(self.non_assert_lines()) > 2)

    @property
    def lines(self):
        return self.data

    def non_assert_lines(self):
        return [s for s in self if not s.lstrip().startswith('assert')]

    def test_count(self):
        """ Compatible with Testsubdesc.test_count(), for lone it() tests. """
        return 1

    def tests(self):
        """ Compatible with Testsubdesc.tests(), for lone it() tests. """
        return [self]


class TestSubdesc(UserList):
    """ Holds it() functions for one describe()/subdesc() call (from snow.h).
    """
    def __init__(self, name, tests=None, filepath=None):
        super().__init__(tests if tests else [])
        self.name = name
        self.filepath = filepath
        self.lines = []

    def __colr__(self):
        return C(self.formatted(color=True))

    def __repr__(self):
        return ''.join((
            f'{type(self).__name__}(',
            f'{self.name!r}, ',
            f'tests=[tests: {len(self)}]',
            ')',
        ))

    def __str__(self):
        return self.formatted(indent=0)

    def bad_tests(self):
        return self.__class__(
            self.name,
            [t for t in self if t.is_bad()]
        )

    def debug(self, *args, **kwargs):
        kwargs['level'] = 1
        debug('    ', *args, **kwargs)

    def formatted(self, indent=0, color=False, code=True):
        spaces = ' ' * indent
        subindent = indent + 4
        name = C(self.name, 'blue') if color else self.name
        return '\n'.join((
            f'{spaces}{name} ({len(self)}):',
            '\n'.join(
                t.formatted(indent=subindent, color=color, code=code)
                for t in self
            ),
        ))

    def has_bad_tests(self):
        return any(t.is_bad() for t in self)

    def parse_its(self):
        """ Parse self.lines to create TestSubdescs and TestIts for self.data.
        """
        self.debug(f'Parsing its for: {self.name}')
        braces = 0
        its = []
        in_it = None
        started = False
        for i, line in enumerate(self.lines):
            stripped = strip_c_line(line)
            if stripped.startswith('it('):
                in_it = TestIt(parse_test_desc(stripped))
                self.debug(f'Found it: {i+1}: {in_it.desc}')
                braces = 1
                started = True
                continue
            elif stripped.endswith('{'):
                if stripped.startswith('}'):
                    braces -= 1
                braces += 1
                # self.debug(f'Pushed: {braces}@{i+1}: {line}')
            elif stripped.endswith('}') or stripped.endswith('};'):
                if '= {' in stripped:
                    braces += 1
                braces -= 1
                # self.debug(f'Popped: {braces}@{i+1}: {line}')
            if (braces == 0) and started:
                if in_it is not None:
                    self.debug(' '.join((
                        f'Found end of it: {i+1}: {in_it.desc}',
                        f'({len(its) + 1})',
                    )))
                    its.append(in_it)
                    in_it = None
                    started = False
                else:
                    raise ValueError('\n'.join((
                        f'Found closing brace with no opening: {self.filepath}',
                        f'Describe: {self.name}',
                        f'Line: {i}: {line!r}',
                    )))
            elif (in_it is not None):
                in_it.append(line)
            else:
                # Comment lines...
                pass
        self.data = its
        return self

    def test_count(self):
        return len(self)

    def tests(self):
        return self.data


class UnusedInfo(UserList):
    name_classes = (
        FunctionName,
        TestFunctionName,
        MacroName,
        TestMacroName
    )

    def as_json(self, sort_keys=True, indent=4):
        return json.dumps(
            self.as_json_obj(),
            sort_keys=sort_keys,
            indent=indent,
        )

    def as_json_obj(self):
        fixed = {}
        for name in self:
            name.info['files'] = name.info['files'].as_json_obj()
            namestr = str(name)
            fixed[namestr] = {'info': name.info}
            fixed[namestr]['class'] = type(name).__name__
            fixed[namestr]['false_positive'] = name.false_positive
        return fixed

    def filter_examples(self, with_examples=True):
        unused = []
        with DebugFilterOp('examples', self):
            for name in self:
                if with_examples:
                    if name.is_example():
                        unused.append(name)
                elif not name.is_example():
                    unused.append(name)
            self.data = unused
        return self

    def filter_pattern(self, pat):
        """ Filter Names by regex pattern. """
        if not pat:
            # None as a pattern means "nevermind, don't filter anything".
            return self
        with DebugFilterOp('pattern', self):
            self.data = [
                name
                for name in self
                if pat.search(str(name)) is not None
            ]
        return self

    def filter_used(self, untested=False, test_deps=False, mark_only=False):
        unused = []
        debugmsg = 'false-positives'
        if mark_only:
            debugmsg = ' '.join((debugmsg, '(mark only)'))
        with DebugFilterOp(debugmsg, self):
            for name in self:
                if untested:
                    if name.is_untested():
                        unused.append(name)
                elif name.is_unused():
                    if test_deps and not name.is_test_dep():
                        continue
                    unused.append(name)
                else:
                    # Used for marking only.
                    name.false_positive = True
            if not mark_only:
                self.data = unused
        return self

    def filter_used_macros(self):
        with DebugFilterOp('used macros', self):
            unused = []
            for name in self:
                if not isinstance(name, MacroName):
                    unused.append(name)
                elif name.is_unused():
                    unused.append(name)
            self.data = unused
        return self

    @classmethod
    def from_json_file(cls, filepath):
        """ Load raw info from a json file. """
        try:
            with open(filepath, 'r') as f:
                try:
                    o = json.load(f)
                except json.JSONDecodeError as ex:
                    raise InvalidJSON(f'Decoding error: {ex}')
        except EnvironmentError as ex:
            raise InvalidJSON(f'Error reading file: {ex}')

        return cls.from_json_obj(o)

    @classmethod
    def from_json_obj(cls, o):
        # Convert from raw dict into the appropriate classes.
        clsmap = {
            c.__name__: c
            for c in cls.name_classes
        }
        fixed = []
        for namestr, nameinfo in o.items():
            namecls = clsmap[nameinfo['class']]
            name = namecls(namestr)
            name.false_positive = nameinfo['false_positive']
            nameinfo['info']['files'] = Files.from_json_obj(
                nameinfo['info']['files']
            )
            name.info = nameinfo['info']
            fixed.append(name)
        return cls(sorted(fixed))

    def only_functions(self):
        self.data = [n for n in self if isinstance(n, FunctionName)]
        return self

    def only_macros(self):
        self.data = [n for n in self if isinstance(n, MacroName)]
        return self

    def search(self):
        """ Use `ag` to search for all current names.
            Returns an exit status code.
        """
        return search(str(name) for name in self)


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
