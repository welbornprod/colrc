#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" cppcheck_errors.py
    Colorized `cppcheck --errorlist`.
    -Christopher Welborn 09-21-2019
"""

import os
import subprocess
import re
import sys
from collections import UserList
from xml.etree import ElementTree as ET

from colr import (
    Colr as C,
    auto_disable as colr_auto_disable,
    docopt,
    get_terminal_size,
)
from fmtblock import FormatBlock
from printdebug import DebugColrPrinter
TERMWIDTH = get_terminal_size()[0]

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug
debug_err = debugprinter.debug_err

colr_auto_disable()

NAME = 'CPPCheck - Error List'
VERSION = '0.0.1'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

SEVERITIES = (
    'warning',
    'performance',
    'portability',
    'error',
    'style',
)

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} [-h | -v]
        {SCRIPT} [-D] [-s type...] [PATTERN]

    Options:
        PATTERN                  : Only show errors matching this regex pattern.
        -D,--debug               : Show more info while running.
        -h,--help                : Show this help message.
        -s type,--severity type  : Only show errors with this severity level.
                                   Can be one of:
                                       {", ".join(SEVERITIES)}
        -v,--version             : Show version.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    errs = CppCheckErrors.from_cppcheck()
    if not errs:
        print_err('No errors found in cppcheck output!')
        return 1
    pat = try_repat(argd['PATTERN'])
    if pat:
        errs = errs.filter_pattern(pat)
    if argd['--severity']:
        errs = errs.filter_severity(argd['--severity'])
    print(C(errs))
    return 0 if errs else 1


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


class CppCheckError(object):
    def __init__(self, name, cwe=None, severity=None, msg=None, verbose=None):
        self.name = name
        self.cwe = cwe or None
        self.severity = severity
        self.msg = msg
        self.verbose = verbose

    def __colr__(self):
        return C('\n').join(
            C(' ').join(
                C(f'{self.name:<40}', 'cyan'),
                C(f'{self.cwe or "":>4}', 'blue', style='bright'),
                C(self.severity, 'yellow'),
            ),
            C(FormatBlock(self.verbose).format(
                width=TERMWIDTH - 4,
                prepend='    ',
                newlines=True,
            ))
        )

    def __str__(self):
        return str(self.__colr__())

    @classmethod
    def from_elem(cls, elem):
        name = elem.attrib['id']
        cwe = int(elem.attrib.get('cwe', 0)) or None
        msg = elem.attrib['msg']
        sev = elem.attrib['severity']
        verb = elem.attrib['verbose'].replace('\\012', '\n')
        return cls(name, cwe=cwe, severity=sev, msg=msg, verbose=verb)


class CppCheckErrors(UserList):
    severities = SEVERITIES

    def __colr__(self):
        return C('\n\n').join(C(e) for e in self)

    def __str__(self):
        return '\n\n'.join(str(e) for e in self)

    def filter_pattern(self, pat):
        return self.__class__(
            e
            for e in self
            if pat.search(e.name) is not None
        )

    def filter_severity(self, sev):
        if isinstance(sev, str):
            sev = [sev]
        sev = [s.lower().strip() for s in sev]
        for s in sev:
            if s not in self.severities:
                raise InvalidArg('\n'.join((
                    f'not a valid "severity": {s}',
                    f'Expecting one of: {", ".join(self.severities)}'
                )))
        return self.__class__(
            e
            for e in self
            if e.severity in sev
        )

    @classmethod
    def from_cppcheck(cls):
        cmd = ['cppcheck', '--errorlist']
        lines = list(
            subprocess.check_output(
                cmd,
                stderr=subprocess.STDOUT
            ).decode().splitlines()
        )
        if not lines:
            print(C('No output from cppcheck!', 'red'), file=sys.stderr)
            sys.exit(1)
        while lines[0].lstrip().startswith('<?'):
            lines.pop(0)
        xml = '\n'.join(lines)
        return cls.from_str(xml)

    @classmethod
    def from_str(cls, s):
        results = ET.fromstring(s)
        errorselem = results.find('errors')
        return cls(
            sorted(
                [
                    CppCheckError.from_elem(e)
                    for e in errorselem.iterfind('error')
                ],
                key=lambda e: e.name
            )
        )


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
