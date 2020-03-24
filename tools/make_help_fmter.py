#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" make_help_fmter.py
    Colorizes `make help/targets` output, because.
    -Christopher Welborn 08-11-2019
"""

import os
import re
import sys

from colr import (
    codes,
    Colr as C,
    Preset as ColrPreset,
    auto_disable as colr_auto_disable,
    docopt,
    enable as colr_enable,
)
from printdebug import DebugColrPrinter

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug
debug_err = debugprinter.debug_err

colr_auto_disable()

NAME = 'Make Help Formatter'
VERSION = '0.0.2'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-c] [-h | -v]

    Options:
        -c,--color    : Force color output, even when stdout is not a terminal.
        -h,--help     : Show this help message.
        -v,--version  : Show version.
"""

# Some colr presets.
CTarget = ColrPreset(fore='blue')
# Color for the suffix of targets with a common prefix.
CTargetSuffix = ColrPreset(fore='dodgerblue3')
CDesc = ColrPreset(fore='reset')
CCmd = ColrPreset(fore='cyan')
CParens = ColrPreset(fore='dimgrey')

desc_pats = {
    # Parens.
    re.compile(r'\([^\)]+\)'): CParens,
}


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    if argd['--color']:
        colr_enable()
    if not (sys.stdout.isatty() or argd['--color']):
        for line in sys.stdin:
            print(line, end='')
        return 0
    format_stdin()
    return 0


def format_desc(desc):
    for pat in desc_pats:
        matches = pat.findall(desc)
        for match in matches:
            if isinstance(match, tuple):
                match = match[0]
            repl = C('').join(desc_pats[pat](match), '')
            desc = pat.sub(str(repl), desc)
    return CDesc(desc)


def format_stdin():
    in_continue = False
    lines = [s for s in sys.stdin]
    prefixes = get_prefixes(lines)
    for line in lines:
        try:
            line.index(':')
            in_continue = False
        except ValueError:
            in_continue = True

        if in_continue:
            line = str(format_desc(line))
        elif not line.startswith((' ', '\t')):
            # Header line.
            print(line, end='')
            continue
        else:
            target, desc = parse_line(line)
            line = str(C(':').join(
                format_target(target, prefixes=prefixes), format_desc(desc)
            ))
        line = replace_cmd(line)

        print(line, end='')


def format_target(target, prefixes=None):
    targetname = target.strip()
    prefixes = prefixes or []
    for s in prefixes:
        if not targetname.startswith(s):
            continue
        targetindent, _, targetpad = target.partition(targetname)
        targetsuf = targetname.replace(s, '').strip()
        targetpre = targetname.replace(targetsuf, '')
        return C('').join(
            targetindent,
            CTarget(targetpre),
            CTargetSuffix(targetsuf),
            targetpad,
        )
    return CTarget(target)


def get_prefixes(lines):
    """ Get common prefixes from a list of lines. """
    prefixes = set()
    for line in lines:
        target, _ = parse_line(line)
        if not target:
            continue
        target = target.strip()
        for line2 in lines[:]:
            target2, _ = parse_line(line2)
            if not target2:
                continue
            target2 = target2.strip()
            if target == target2:
                continue
            if target2.startswith(target):
                # Don't do the smallest prefixes
                prefixes.add(target)
    return sorted(prefixes)


def parse_line(line):
    if ':' not in line:
        return None, None
    target, _, desc = line.partition(':')
    return target, desc


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


def replace_cmd(line):
    chars = []
    opened = False
    for ch in line:
        if ch == '`':
            chars.extend(codes['fore']['reset'])
            chars.extend(codes['style'][CDesc.style or 'reset_all'])
            chars.append(ch)
            if opened:
                chars.extend(CDesc.codes())
                opened = False
            else:
                chars.extend(CCmd.codes())
                opened = True
        else:
            chars.append(ch)
    return ''.join(chars)


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
    except (EOFError, KeyboardInterrupt):
        print_err('\nUser cancelled.\n')
        mainret = 2
    except BrokenPipeError:
        print_err('\nBroken pipe, input/output was interrupted.\n')
        mainret = 3
    sys.exit(mainret)
