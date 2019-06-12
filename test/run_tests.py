#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" run_tests.py
    Run `test_colr` and print the output in a prettier format.
    -Christopher Welborn 06-11-2019
"""

import os
import subprocess
import sys
from xml.etree import ElementTree

from colr import (
    Colr as C,
    auto_disable as colr_auto_disable,
    docopt,
)
from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import Terminal256Formatter

pyg_lexer = get_lexer_by_name('c')
pyg_fmter = Terminal256Formatter(bg='dark', style='monokai')

colr_auto_disable()

NAME = 'Colr Test Runner'
VERSION = '0.0.1'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

DEFAULT_TEST_EXE = os.path.join(SCRIPTDIR, 'test_colr')

USAGESTR = """{versionstr}
    Usage:
        {script} -h | -v
        {script} [-n | -x]

    Options:
        -h,--help     : Show this help message.
        -n,--normal   : Show normal CMocka output, with no color.
        -v,--version  : Show version.
        -x,--xml      : Show raw XML from the test executable.
""".format(script=SCRIPT, versionstr=VERSIONSTR)


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    style = None if argd['--normal'] else 'XML'
    return run_test_exe(style=style, raw=argd['--xml'])


def highlight_c(s):
    """ Syntax highlight some C code. """
    return highlight(s, pyg_lexer, pyg_fmter).strip()


def parse_failure_elem(failure, indent=0):
    """ Parse a <failure> element and return either a FailureLineInfo, or
        a FailureMessage.
    """
    try:
        code, lineinfo = failure.text.split('\n')
    except ValueError:
        # Just line info.
        return FailureLineInfo(failure.text, code=None, indent=indent)
    return FailureLineInfo(lineinfo, code=code, indent=indent)


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


def run_test_exe(style='XML', raw=False):
    """ Run the test executable and parse it's output. """
    if not os.path.exists(DEFAULT_TEST_EXE):
        print_err(f'Test executable does not exist: {DEFAULT_TEST_EXE}')
        return 1
    env = {'CMOCKA_MESSAGE_OUTPUT': style} if style else None
    proc = subprocess.Popen(
        DEFAULT_TEST_EXE,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()

    if not stdout:
        if not stderr:
            print_err(f'No output from the test exe!')
        return 1
    if raw or (style != 'XML'):
        # No use parsing anything, it's not xml.
        print(stdout.decode())
        if stderr:
            print(stderr.decode())
        return 0
    root = ElementTree.fromstring(stdout.decode())
    allcounts = SuiteCounts('All')
    for suite in root.findall('testsuite'):
        counts = SuiteCounts.from_suite_elem(suite)
        allcounts += counts
        print(C(counts))
        for testcase in suite.findall('testcase'):
            name = testcase.attrib['name']
            failure = testcase.find('failure')
            if failure is None:
                print('    {}'.format(C(name, 'green')))
                continue
            print('    {}'.format(C(name, 'red')))
            print(C(parse_failure_elem(failure, indent=8)))

    print(f'\n{C(allcounts)}')
    if stderr:
        print_err(f'\n{stderr.decode()}')

    return 0


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return 'Invalid argument, {}'.format(self.msg)
        return 'Invalid argument!'


class FailureLineInfo(object):
    """ A formatted failure string. """
    def __init__(self, lineinfo, code=None, indent=0):
        self.code = code
        self.file, self.line, self.msg = lineinfo.split(':', 2)
        self.msg = self.msg.strip()
        self.indent = indent or 0

    def __colr__(self):
        spaces = ' ' * self.indent

        # Append line info.
        pcs = [
            C(':').join(
                C(self.file, 'cyan'),
                C(self.line, 'lightblue'),
                ' {}'.format(C(self.msg, 'red')),
            )
        ]
        # Use highlighted code if available.
        if self.code:
            pcs.append(highlight_c(self.code))
        lines = C(f'\n{spaces}').join(pcs)
        return C(f'{spaces}{lines}')


class SuiteCounts(object):
    """ Holds test/failure/error/skipped counts from a <testsuite> element.
    """
    def __init__(self, name, tests=0, failures=0, errors=0, skipped=0):
        self.name = name or ''
        self.tests = tests or 0
        self.failures = failures or 0
        self.errors = errors or 0
        self.skipped = skipped or 0
        self.testcolr = 'blue' if (self.failures or self.errors) else 'green'

    def __add__(self, other):
        """ Add one SuiteCounts totals to another, without changing this one's
            name.
        """
        if not isinstance(other, self.__class__):
            raise TypeError(
                f'Expecting {type(self).__name__}, got: {type(other).__name__}'
            )
        self.tests += other.tests
        self.failures += other.failures
        self.errors += other.errors
        self.skipped += other.skipped
        return self

    def __colr__(self):
        pcs = [
            f'{C(self.name):[blue]}',
            self.colr_counts(),
        ]
        return C(' ').join(pcs)

    def colr_counts(self):
        """ Return a string containing only the formatted counts. """
        pcs = [
            C(': ').join(C('Tests', 'cyan'), C(self.tests, self.testcolr))
        ]
        if self.errors:
            pcs.append(
                C(': ').join(C('Errors', 'cyan'), C(self.errors, 'red'))
            )
        if self.failures:
            pcs.append(
                C(': ').join(C('Failed', 'cyan'), C(self.failures, 'red'))
            )
        if self.skipped:
            pcs.append(
                C(': ').join(C('Skipped', 'cyan'), C(self.skipped, 'blue'))
            )
        return C(' ').join(pcs)

    @classmethod
    def from_suite_elem(cls, suiteelem):
        return cls(
            suiteelem.attrib['name'],
            tests=int(suiteelem.attrib['tests']),
            failures=int(suiteelem.attrib['failures']),
            errors=int(suiteelem.attrib['errors']),
            skipped=int(suiteelem.attrib['skipped']),
        )


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
