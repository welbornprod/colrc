#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" run_tests.py
    Run `test_colr` and print the output in a prettier format.
    -Christopher Welborn 06-11-2019
"""

import os
import subprocess
import sys
from collections import UserList
from enum import Enum
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

# Environment variable to change CMocka output style.
# Need 'XML' to parse, but can be set to any other valid value to just print
# the output.
cmocka_var = 'CMOCKA_MESSAGE_OUTPUT'

colr_auto_disable()

NAME = 'Colr Test Runner'
VERSION = '0.0.1'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

DEFAULT_TEST_EXE = os.path.join(SCRIPTDIR, 'test_colr')

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-n | -p | -s | -t | -x]

    Options:
        -h,--help     : Show this help message.
        -n,--normal   : Show normal CMocka stdout-style output.
        -p,--pyrepr   : Show python repr() for the TestSuites object.
        -s,--subunit  : Show subunit-style CMocka output.
        -t,--tap      : Show tap-style CMocka output.
        -v,--version  : Show version.
        -x,--xml      : Show raw XML from the test executable.

    The {cmocka_var} environment variable will be honored. For pure XML output,
    you must use the --xml flag.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    style = OutputStyle.from_argd(argd)
    return run_test_exe(style=style)


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


def run_test_exe(style=None):
    """ Run the test executable and parse it's output. """
    if not os.path.exists(DEFAULT_TEST_EXE):
        print_err(f'Test executable does not exist: {DEFAULT_TEST_EXE}')
        return 1
    if style is None:
        style = OutputStyle.color

    # Set cmock env var if not already set.
    cmocka_style = os.environ.get(
        cmocka_var,
        style.cmocka_value()
    ).strip().upper()
    os.environ[cmocka_var] = cmocka_style

    proc = subprocess.Popen(
        DEFAULT_TEST_EXE,
        env=os.environ,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()
    exitcode = proc.wait()
    if not stdout:
        if not stderr:
            print_err(f'No output from the test exe!')
        return 1

    # Check to see if we need to parse the resulting XML, just print it if not.
    if (os.environ[cmocka_var] != 'XML') or (style == OutputStyle.xml):
        # No use parsing anything, it's not xml.
        print(stdout.decode())
        if stderr:
            print(stderr.decode())
        return 0

    suites = TestSuites.from_output(stdout.decode())

    # Pick output formatter and print the result.
    formatter = style.formatter()
    print(formatter(suites))
    return exitcode


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return f'Invalid argument, {self.msg}'
        return 'Invalid argument!'


class InvalidXML(InvalidArg):
    """ Raised when the test runner emits non-xml data. """
    def __str__(self):
        if self.msg:
            return f'Test executable output was invalid XML: {self.msg}'
        return 'Test executable output was invalid XML!'


class FailureLineInfo(object):
    """ A formatted failure string. """
    indent = 8

    def __init__(self, lineinfo, code=None):
        self.code = code
        self.file, self.line, self.msg = lineinfo.split(':', 2)
        self.msg = self.msg.strip()

    def __bool__(self):
        # Being explicit, even though this is the default python behavior.
        return True

    def __colr__(self):
        """ Default Colr instance, when wrapped in a plain `Colr()` call. """
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

    def __repr__(self):
        tname = type(self).__name__
        return ''.join((
            f'{tname}(code={self.code!r}',
            f', file={self.file!r}',
            f', msg={self.msg!r}',
            ')',
        ))

    @classmethod
    def from_elem(cls, failelem):
        """ Build a FailureLineInfo from a <failure> ElementTree element. """
        try:
            code, lineinfo = failelem.text.split('\n')
        except ValueError:
            # Just line info.
            lineinfo = failelem.text
            code = None
        return cls(lineinfo, code=code)


class OutputStyle(Enum):
    """ Enum for the different output styles this runner supports. """
    normal = 0
    color = 1
    xml = 2
    pyrepr = 3
    subunit = 4
    tap = 5

    def __colr__(self):
        return C(str(self), 'blue', style='bold')

    def __str__(self):
        """ Enhanced representation for console. """
        return {
            OutputStyle.normal.value: 'normal',
            OutputStyle.color.value: 'color',
            OutputStyle.xml.value: 'xml',
            OutputStyle.pyrepr.value: 'pyrepr',
            OutputStyle.subunit.value: 'subunit',
            OutputStyle.tap.value: 'tap',
        }.get(self.value, 'unknown')

    def cmocka_value(self):
        """ Return the proper CMOCKA_MESSAGE_OUTPUT setting for this value. """
        return {
            'normal': 'STDOUT',
            'color': 'XML',
            'xml': 'XML',
            'pyrepr': 'XML',
            'subunit': 'SUBUNIT',
            'tap': 'TAP',
        }.get(str(self), 'XML')

    def formatter(self):
        """ Return the proper formatting function for this value. """
        return {
            'normal': str,
            'color': C,
            'xml': str,
            'pyrepr': repr,
            'subunit': str,
            'tap': str,
        }.get(str(self), C)

    @classmethod
    def from_argd(cls, argd):
        flagattrs = (
            #  'color', There is no flag for color, it is the default.
            'normal',
            'xml',
            'pyrepr',
            'subunit',
            'tap',
        )
        for flag in flagattrs:
            if argd[f'--{flag}']:
                return getattr(cls, flag)
        return cls.color


class TestCase(object):
    """ Holds info about a single <testcase> element. """
    indent = 4

    def __init__(self, name, failure=None):
        self.name = name
        self.failure = failure

    def __colr__(self):
        """ Default Colr instance, when wrapped in a plain `Colr()` call. """
        pcs = [
            '{}{}'.format(
                ' ' * self.indent,
                C(self.name, 'red' if self.failure else 'green'),
            ),
        ]
        if self.failure:
            pcs.append(C(self.failure))
        return C('\n').join(pcs)

    def __repr__(self):
        tname = type(self).__name__
        return '\n'.join((
            f'{tname}(',
            f'    name={self.name!r},',
            f'    failure={self.failure!r}',
            ')',
        ))

    @classmethod
    def from_elem(cls, caseelem):
        """ Build a TestCase from a <testcase> ElementTree element. """
        name = caseelem.attrib['name']
        failelem = caseelem.find('failure')
        if failelem is None:
            return cls(name)
        failure = FailureLineInfo.from_elem(failelem)
        return cls(name, failure=failure)


class TestSuite(UserList):
    """ Holds info about a single <testsuite> element, which holds info
        about <testcase> elements.
    """
    indent = 0

    def __init__(self, name, cases=None, time=None, errors=None, skipped=None):
        if cases:
            super().__init__(cases)
        self.name = name
        self.tests = len(self.data)
        self.time = float(time or 0.0)
        self.failures = len([c for c in self if c.failure])
        self.errors = int(errors or 0)
        self.skipped = int(skipped or 0)

    def __colr__(self):
        """ Default Colr instance, when wrapped in a plain `Colr()` call. """
        return C('\n').join(
            C(' ').join(
                C(f'\n{self.name.ljust(32)}', 'blue'),
                self.colr_counts(),
            ),
            C('\n').join(C(case) for case in self),
        )

    def __repr__(self):
        tname = type(self).__name__
        cases = '\n        '.join(
            '{},'.format('\n        '.join(l for l in repr(x).split('\n')))
            for x in self
        )
        return '\n    '.join((
            f'{tname}(',
            f'name={self.name!r},',
            f'tests={self.tests!r},',
            f'time={self.time!r},',
            f'failures={self.failures!r},',
            f'errors={self.errors!r},',
            f'skipped={self.skipped!r},',
            f'data=[\n        {cases}\n    ]\n)',
        ))

    @property
    def cases(self):
        return self.data

    def colr_counts(self):
        """ Return a string containing only the formatted counts. """
        testcolr = 'blue' if (self.errors or self.failures) else 'green'
        pcs = [
            C(': ').join(C('Tests', 'cyan'), C(self.tests, testcolr))
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
    def from_elem(cls, suiteelem):
        """ Build a TestSuite from a <testsuite> ElementTree element. """
        cases = [
            TestCase.from_elem(e)
            for e in suiteelem.findall('testcase')
        ]
        return cls(
            suiteelem.attrib['name'],
            cases=cases,
            time=suiteelem.attrib['time'],
            errors=suiteelem.attrib['errors'],
            skipped=suiteelem.attrib['skipped'],
        )


class TestSuites(UserList):
    """ Holds info about a single <testsuites>, which holds info about
        <testsuite> elements.
    """
    def __init__(self, suites=None):
        if suites:
            super().__init__(suites)
        self.time = 0.0
        self.tests = 0
        self.failures = 0
        self.errors = 0
        self.skipped = 0
        self.update_counts()

    def __add__(self, other):
        """ Add another TestSuites elements to this one. """
        if not isinstance(other, self.__class__):
            raise TypeError(
                f'Expecting {type(self).__name__}, got {type(other).__name__}.'
            )
        self.data.extend(other.data)
        self.time += other.time
        self.tests += other.tests
        self.failures += other.failures
        self.errors += other.errors
        self.skipped += other.skipped
        return self

    def __colr__(self):
        """ Default Colr instance, when wrapped in a plain `Colr()` call. """
        return C('\n').join(
            C('\n').join(C(st) for st in self),
            C(' ').join(
                C('\nAll', 'blue', style='bold'),
                self.colr_counts(),
            ),
        )

    def __iadd__(self, other):
        # __add__ already does an in-place addition.
        return self.__add__(other)

    def __repr__(self):
        tname = type(self).__name__
        indent = ' ' * 4
        suiteindent = indent * 2
        suites = f'\n{suiteindent}'.join(
            '{},'.format(
                f'\n{suiteindent}'.join(l for l in repr(x).split('\n'))
            )
            for x in self
        )
        return '\n    '.join((
            f'{tname}(',
            f'tests={self.tests!r},',
            f'time={self.time!r},',
            f'failures={self.failures!r},',
            f'errors={self.errors!r},',
            f'skipped={self.skipped!r},',
            f'data=[\n{suiteindent}{suites}\n{indent}]\n)',
        ))

    def colr_counts(self):
        """ Return a string containing only the formatted counts. """
        testcolr = 'blue' if (self.errors or self.failures) else 'green'
        pcs = [
            C(': ').join(C('Tests', 'cyan'), C(self.tests, testcolr))
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
    def from_output(cls, output):
        """ Parse the test runner's XML output and build a TestSuites object,
            even if the output contains multiple <testsuites> elements.
        """
        suites = None
        for suite_str in cls.parse_runner_xml(output):
            if suites is None:
                suites = cls.from_xml(suite_str)
                continue
            suites += cls.from_xml(suite_str)
        return suites

    @classmethod
    def from_xml(cls, xml):
        """ Parse a <testsuites> element string into an actual TestSuite
            with TestCase elements.
        """
        try:
            root = ElementTree.fromstring(xml)
        except ElementTree.ParseError as ex:
            raise InvalidXML(str(ex))

        suiteslist = []
        for suiteelem in root.findall('testsuite'):
            suite = TestSuite.from_elem(suiteelem)
            suiteslist.append(suite)
        return cls(suites=suiteslist)

    @staticmethod
    def parse_runner_xml(s):
        """ Parse an XML doc with multiple <testsuites> elements,
            and yield each individual <testsuites> element.
        """
        lines = []
        for line in s.split('\n'):
            lines.append(line)
            if '</testsuites>' in line:
                yield '\n'.join(lines)
                lines = []

    @property
    def suites(self):
        return self.data

    def update_counts(self):
        """ Update the `time`, `tests`, `failures`, `errors`, and `skipped`
            attributes based on each TestSuite in self.data.
        """
        self.time = 0.0
        self.tests = 0
        self.failures = 0
        self.errors = 0
        self.skipped = 0
        for suite in self:
            self.time += suite.time
            self.tests += suite.tests
            self.failures += suite.failures
            self.errors += suite.errors
            self.skipped += suite.skipped
        return self


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
