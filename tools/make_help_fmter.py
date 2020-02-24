#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" make_help_fmter.py
    Colorizes `make help/targets` output, because.
    -Christopher Welborn 08-11-2019
"""

import os
import sys

from colr import (
    codes,
    Colr as C,
    Preset as ColrPreset,
    auto_disable as colr_auto_disable,
    docopt,
)
from printdebug import DebugColrPrinter

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug
debug_err = debugprinter.debug_err

colr_auto_disable()

NAME = 'Make Help Formatter'
VERSION = '0.0.1'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-h | -v]

    Options:
        -h,--help     : Show this help message.
        -v,--version  : Show version.
"""

# Some colr presets.
CTarget = ColrPreset(fore='blue')
CDesc = ColrPreset(fore='reset')
CCmd = ColrPreset(fore='cyan')


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    if not sys.stdout.isatty():
        for line in sys.stdin:
            print(line, end='')
        return 0
    format_stdin()
    return 0


def format_stdin():
    in_continue = False

    for line in sys.stdin:
        try:
            line.index(':')
            in_continue = False
        except ValueError:
            in_continue = True

        if in_continue:
            line = str(CDesc(line))
        elif not line.startswith((' ', '\t')):
            # Header line.
            print(line, end='')
            continue
        else:
            target, _, desc = line.partition(':')
            line = str(C(':').join(CTarget(target), CDesc(desc)))
        line = replace_cmd(line)

        print(line, end='')


def get_preset_codes(preset):
    # This needs to be in Colr. I just now needed it.
    # It could be Preset.codes().
    # Update: It is in Colr 0.9.3, just haven't updated the package yet.
    #         When it is updated, this whole function becomes: `preset.codes()`
    #         and it comes with color/style validating and calculated colors
    #         (like RGB/hex values that would crash this function).
    pcs = []
    if preset.fore:
        pcs.append(codes['fore'][preset.fore])
    if preset.back:
        pcs.append(codes['back'][preset.back])
    if preset.style:
        pcs.append(codes['style'][preset.style])
    return ''.join(pcs)


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
                chars.extend(get_preset_codes(CDesc))
                opened = False
            else:
                chars.extend(get_preset_codes(CCmd))
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
