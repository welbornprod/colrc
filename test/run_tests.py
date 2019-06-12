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
        {script} [-h | -v]

    Options:
        -h,--help     : Show this help message.
        -v,--version  : Show version.
""".format(script=SCRIPT, versionstr=VERSIONSTR)


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    return run_test_exe()


def highlight_c(s):
    """ Syntax highlight some C code. """
    return highlight(s, pyg_lexer, pyg_fmter).strip()


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


def run_test_exe():
    """ Run the test executable and parse it's output. """
    if not os.path.exists(DEFAULT_TEST_EXE):
        print_err(f'Test executable does not exist: {DEFAULT_TEST_EXE}')
        return 1
    env = {'CMOCKA_MESSAGE_OUTPUT': 'XML'}
    proc = subprocess.Popen(
        DEFAULT_TEST_EXE,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()
    if stderr:
        print_err(f'Test runner error: {stderr.decode()}')

    if not stdout:
        if not stderr:
            print_err(f'No output from the test exe!')
        return 1

    root = ElementTree.fromstring(stdout.decode())
    for suite in root.findall('testsuite'):
        print('{}:'.format(C(suite.attrib['name'], 'blue')))
        for testcase in suite.findall('testcase'):
            name = testcase.attrib['name']
            failure = testcase.find('failure')
            if failure is None:
                print('    {}'.format(C(name, 'green')))
                continue
            print('    {}'.format(C(name, 'red')))
            code, lineinfo = failure.text.split('\n')
            print('        {}'.format(highlight_c(code)))
            file, line, msg = lineinfo.split(':', 2)
            msg = msg.strip()
            print('        {}'.format(
                C(':').join(
                    C(file, 'cyan'),
                    C(line, 'lightblue'),
                    ' {}'.format(C(msg, 'red')),
                )
            ))
    return 0


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
