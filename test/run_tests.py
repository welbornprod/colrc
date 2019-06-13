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
from printdebug import DebugColrPrinter

from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import Terminal256Formatter

debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug

pyg_lexer = get_lexer_by_name('c')
pyg_fmter = Terminal256Formatter(bg='dark', style='monokai')

# Environment variable to change CMocka output style.
# Need 'XML' to parse, but can be set to any other valid value to just print
# the output.
cmocka_var = 'CMOCKA_MESSAGE_OUTPUT'
cmocka_file_var = 'CMOCKA_XML_FILE'

colr_auto_disable()

NAME = 'Colr Test Runner'
VERSION = '0.0.1'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

DEFAULT_FILE_FMT = 'cm_%g.xml'
DEFAULT_TEST_EXE = os.path.join(SCRIPTDIR, 'test_colr')

USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-p | -S | -s | -t | -x] [TEST_EXE] [-D]
        {SCRIPT} -X [FMT] [TEST_EXE] [-D]
        {SCRIPT} -f FILE... [-p] [-D]

    Options:
        FILE               : One or more XML files to parse.
                             If '-' is used, stdin will be read.
        FMT                : File name to generate with -f.
                             '%g' will be replaced with the test group name
                             if groups were used.
                             Default: {DEFAULT_FILE_FMT!r}
        TEST_EXE           : Cmocka-based test executable to run.
                             Default: {DEFAULT_TEST_EXE}
        -D,--debug         : Print some more info to stderr while running.
        -f,--file          : Treat arguments as CMocka XML files to parse.
        -h,--help          : Show this help message.
        -S,--stdout        : Show normal CMocka stdout-style output.
                             This is the same as running the test exe itself.
        -p,--pyrepr        : Show python repr() for the TestSuites object.
        -s,--subunit       : Show subunit-style CMocka output.
        -t,--tap           : Show tap-style CMocka output.
        -v,--version       : Show version.
        -X,--xmlfile       : Use {cmocka_file_var} to generate XML files,
                             instead of printing the test results.
                             See FMT.
        -x,--xml           : Show raw XML from the test executable.

    The {cmocka_var} environment variable will be honored. For pure XML output,
    you must use the --xml flag.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    style = OutputStyle.from_argd(argd)
    debug(f'Arg output style: {C(style)}')
    if argd['--file']:
        return parse_files(argd['FILE'], style=style)
    return run_test_exe(
        argd['TEST_EXE'] or DEFAULT_TEST_EXE,
        file_fmt=(argd['FMT'] or DEFAULT_FILE_FMT),
        style=style,
    )


def highlight_c(s):
    """ Syntax highlight some C code. """
    return highlight(s, pyg_lexer, pyg_fmter).strip()


def parse_files(filepaths, style=None):
    debug('Parsing xml files...')
    errs = 0
    for filepath in filepaths:
        if filepath == '-':
            # Do stdin.
            errs += print_output(read_stdin(), style=style)
            continue
        # Actual file path.
        try:
            with open(filepath, 'r') as f:
                errs += print_output(f.read(), style=style)
        except FileNotFoundError:
            print_err(C(': ').join(
                C('File not found', 'red'),
                C(filepath, 'blue')
            ))
            errs += 1
        except EnvironmentError as ex:
            print_err(C('\n').join(
                C(': ').join(
                    C('Unable to read file', 'red'),
                    C(filepath, 'blue'),
                ),
                f'    {ex}'
            ))
            errs += 1
    return errs


def print_output(output, style=None):
    """ Parse and print XML output in the preferred style. """
    suites = TestSuites.from_output(output)
    # Pick output formatter and print the result.
    if style is None:
        style = OutputStyle.color
    formatter = style.formatter()
    print(formatter(suites))
    return suites.failures


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


def read_stdin():
    if read_stdin.already_done:
        raise InvalidArg('already read from stdin once.')
    if sys.stdin.isatty() and sys.stdout.isatty():
        print('\nReading from stdin until end of file (Ctrl + D)...\n')
    data = sys.stdin.read()
    read_stdin.already_done = True
    return data


# Set when the function successfully reads from stdin the first time.
read_stdin.already_done = False


def run_file_mode(exe, file_fmt=None):
    """ Run the test executable in CMocka's file mode.
        Returns an exit status code.
    """
    # Set the file format before running the exe.
    os.environ[cmocka_file_var] = os.environ.get(
        cmocka_file_var,
        file_fmt or DEFAULT_FILE_FMT,
    )
    debug('Running in file-mode...', end=' ')
    debug(f'Format: {C(os.environ[cmocka_file_var]):[blue]}')
    proc = subprocess.Popen(
        exe,
        env=os.environ,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()
    exitcode = proc.wait()

    print(C(' ').join(C(exe, 'blue'), 'completed, '), end='')
    if exitcode:
        print(C('there were errors', 'red'), '.', sep='')
    else:
        print(C('no errors', 'green'), '.', sep='')
    if stderr:
        print_err(stderr.decode())
    return exitcode


def run_no_parse(exe):
    """ Run the test executable, and print it's output. """
    debug('Running in no-parse mode...')
    proc = subprocess.Popen(
        exe,
        env=os.environ,
    )
    exitcode = proc.wait()
    return exitcode


def run_parse(exe, style=None):
    """ Run the test executable, parse it's output, and format it. """
    debug('Running in parse-mode...')
    proc = subprocess.Popen(
        exe,
        env=os.environ,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()
    exitcode = proc.wait()
    if not stdout:
        if stderr:
            print_err(stderr.decode())
        else:
            print_err(f'No output from the test exe!')
        return 1

    print_output(stdout.decode(), style=style)
    return exitcode


def run_test_exe(exe, file_fmt=None, style=None):
    """ Run the test executable and parse it's output. """
    if not os.path.exists(exe):
        print_err(f'Test executable does not exist: {exe}')
        return 1
    if style is None:
        style = OutputStyle.color
        debug(f'Set default style: {C(style):[blue]}')
    if os.environ.get(cmocka_file_var, None):
        # Detected CMOCKA_XML_FILE, set file mode.
        style = OutputStyle.xmlfile
        debug(f'Set style for {cmocka_file_var}: {C(style):[blue]}')
    # Set cmocka output var if not already set.
    cmocka_style = os.environ.get(
        cmocka_var,
        style.cmocka_value()
    ).strip().upper()
    os.environ[cmocka_var] = cmocka_style
    debug('Set style var:', end=' ')
    debug(f'{C(os.environ[cmocka_var]):[blue]}')
    if style == OutputStyle.xmlfile:
        return run_file_mode(exe, file_fmt=file_fmt)
    elif (not style.needs_parsing()) or (os.environ[cmocka_var] != 'XML'):
        return run_no_parse(exe)
    # The style and output mode says we need to parse the XML.
    return run_parse(exe, style=style)


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
    color = 1
    pyrepr = 2
    stdout = 3
    subunit = 4
    tap = 5
    xml = 6
    xmlfile = 7

    def __colr__(self):
        return C(str(self), 'blue', style='bold')

    def __str__(self):
        """ Enhanced representation for console. """
        return {
            OutputStyle.color.value: 'color',
            OutputStyle.pyrepr.value: 'pyrepr',
            OutputStyle.stdout.value: 'stdout',
            OutputStyle.subunit.value: 'subunit',
            OutputStyle.tap.value: 'tap',
            OutputStyle.xml.value: 'xml',
            OutputStyle.xmlfile.value: 'xmlfile',
        }.get(self.value, 'unknown')

    def cmocka_value(self):
        """ Return the proper CMOCKA_MESSAGE_OUTPUT setting for this value. """
        return self.outputs().get(str(self), 'XML')

    def formatter(self):
        """ Return the proper formatting function for this value. """
        # Some of these values require no XML parsing, so they don't need
        # a formatter.
        return {
            'color': C,
            'pyrepr': repr,
        }.get(str(self), str)

    @classmethod
    def from_argd(cls, argd):
        # color has no flag, it's the default output method.
        for flag in cls.names(ignore=('color', )):
            if argd[f'--{flag}']:
                return getattr(cls, flag)
        return cls.color

    @classmethod
    def names(cls, ignore=None):
        ignored = set(ignore or '')
        return [
            name
            for name in cls.outputs()
            if name not in ignored
        ]

    def needs_parsing(self):
        """ Return True if this output value requires parsing the XML output.
        """
        return str(self) in ('color', 'pyrepr')

    @classmethod
    def outputs(cls):
        """ Return a map of {name: cmocka_output_style} for all values.
        """
        return {
            'color': 'XML',
            'pyrepr': 'XML',
            'stdout': 'STDOUT',
            'subunit': 'SUBUNIT',
            'tap': 'TAP',
            'xml': 'XML',
            'xmlfile': 'XML',
        }


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
