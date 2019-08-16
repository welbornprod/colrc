#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" undoxy_markdown.py
    Converts doxygen-style markdown files to github/normal-style markdown files.
    -Christopher Welborn 08-11-2019
"""

import os
import re
import sys

from colr import (
    Colr as C,
    auto_disable as colr_auto_disable,
    docopt,
)

from printdebug import DebugColrPrinter
debugprinter = DebugColrPrinter()
debugprinter.enable(('-D' in sys.argv) or ('--debug' in sys.argv))
debug = debugprinter.debug
debug_err = debugprinter.debug_err

colr_auto_disable()

NAME = 'ColrC - Markdown Generator'
VERSION = '0.0.1'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])
COLRDIR = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
USAGESTR = f"""{VERSIONSTR}
    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-D] [-t title] FILE [OUTPUT]
        {SCRIPT} [-D] -t title [-H line...] FILE [OUTPUT]

    Options:
        FILE                   : The file to parse. Use - for stdin.
        OUTPUT                 : File path for output.
                                 Default: stdout
        -D,--debug             : Print some more info while running.
        -H line,--header line  : Add these lines after the replaced h1 header.
                                 They will be the body of the --title header.
        -h,--help              : Show this help message.
        -t txt,--title txt     : Replace the first non-doxygen h1 header with
                                 this header text. The old header is converted
                                 into an h2 header.
        -v,--version           : Show version.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    debugprinter.enable(argd['--debug'])
    if not argd['FILE']:
        # Possible future options. This doesn't even fire right now.
        argd['FILE'] = '-'

    if not input_file_exists(argd['FILE']):
        raise InvalidArg(f'file doesn\'t exist: {argd["FILE"]}')

    repl_header = [argd['--title']] if argd['--title'] else []
    if argd['--header'] and repl_header:
        repl_header.extend(argd['--header'])

    write_file(argd['FILE'], output=argd['OUTPUT'], replace_header=repl_header)
    return 0


def input_file_exists(filepath):
    """ Return True if the file path exists, or is the stdin marker. """
    return (filepath == '-') or os.path.exists(filepath)


def parse_file(filepath, replace_header=None):
    """ Parse a doxygen-style markdown file and yield the "fixed" lines. """
    if filepath == '-':
        yield from parse_lines(sys.stdin, replace_header=replace_header)
    else:
        with open(filepath, 'r') as f:
            yield from parse_lines(f, replace_header=replace_header)


def parse_lines(iterable, replace_header=None):
    """ Yield fixed markdown lines. """
    did_header = False
    did_blank = False
    did_replace = False if replace_header else True
    for line in iterable:
        stripped = line.strip()
        if (not did_header) and line.startswith('# '):
            did_header = True
            debug('Skipping Doxygen header line.')
            continue
        elif (not stripped) and (not did_blank):
            # Skip blank lines following the header.
            debug('Skipping blank header line.')
            continue
        elif (not did_replace) and line.startswith('# '):
            if isinstance(replace_header, str):
                yield f'# {replace_header}\n\n'
            else:
                yield f'# {replace_header[0]}\n\n'
                headerbody = replace_header[1:]
                if headerbody:
                    yield from (f'{s}\n' for s in replace_header[1:])
                    yield '\n'
            yield f'#{line}'
            did_replace = True
            debug('Replaced old header.')
            continue
        elif stripped.startswith('\\anchor '):
            # Skip single-line anchors.
            debug(f'Skipped anchor: {line}')
            continue
        elif not did_blank:
            did_blank = True
            debug('Finished with header stuff.')
        replaced = replace_func(line)
        if replaced:
            yield replaced
        else:
            yield line


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


def replace_func(line):
    """ Returns a list of replacement lines if the line contains one of the
        patterns in REPLACE_PATS.
        If there is no match, None is returned.
    """
    for pat, func in REPLACE_PATS.items():
        if pat.search(line) is not None:
            return pat.sub(func, line)
    return []


def replace_includesrc(match):
    """ Parse an \\includesrc{<filepath>} line, and return the string needed to
        replace it, in a Github-style fenced block.
    """
    line = match.group()
    filepath = line.partition('{')[-1].strip().rstrip('}')
    if not filepath:
        print_err(
            f'Missing filepath in \\includesrc{{<filepath>}} line: {line!r}'
        )
        return []
    ext = os.path.splitext(filepath)[-1]
    lang = {
        '.bash': 'bash',
        '.c': 'c',
        '.cpp': 'cpp',  # <- Never tried this on github. Is it valid?
        '.js': 'javascript',
        '.json': 'json',
        '.py': 'python',
        '.sh': 'bash',
        '.zsh': 'zsh',
    }.get(ext, '')
    if not ext:
        debug_err(f'Missing language name for extension: {ext!r}')
    lines = [f'```{lang}\n']
    if not os.path.exists(filepath):
        trypath = os.path.join(COLRDIR, filepath)
        if not os.path.exists(trypath):
            print_err(f'Can\'t find source file: {filepath}')
            return []
        filepath = trypath
    debug(f'Using source file: {filepath}')
    try:
        with open(filepath, 'r') as f:
            lines.extend(f)
    except EnvironmentError as ex:
        print_err(f'Error reading source file: {filepath}\n{ex}')
        return []
    lines.append('```\n')
    return ''.join(lines)


def replace_ref(match):
    ref, title = match.groups()
    if ref.endswith('\n'):
        end = '\n'
        ref = ref.rstrip()
    else:
        end = ''
    # Not sure how to fix these. Doxygen's references don't use a
    # predictable linking scheme.
    if title:
        return f'[{title}](#{ref}){end}'
    # Do the right thing for example references.
    if ref.endswith('_example.c'):
        return f'[{ref}](examples/{ref}){end}'
    return f'{ref}{end}'


def write_file(filepath, output=None, replace_header=None):
    out_file = sys.stdout
    if output:
        debug(f'Opening output file: {output}')
        out_file = open(output, 'w')

    for line in parse_file(filepath, replace_header=replace_header):
        out_file.write(line)

    if out_file.name != '<stdout>':
        out_file.close()


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return f'Invalid argument, {self.msg}'
        return 'Invalid argument!'


# If the key pattern is found in the line, the result of this function is used
# instead.
# It is called with the line that triggered the match: function(line)
REPLACE_PATS = {
    r'\\includesrc\{.+\}': replace_includesrc,
    r'\\ref ([\w_\-\. ]+) ?("[\w\. ]+")?': replace_ref,
}
REPLACE_PATS = {re.compile(k): v for k, v in REPLACE_PATS.items()}

if __name__ == '__main__':
    try:
        mainret = main(docopt(USAGESTR, version=VERSIONSTR, script=SCRIPT))
    except InvalidArg as ex:
        print_err(ex)
        mainret = 1
    except EnvironmentError as ex:
        print_err(f'File error: {ex.filename}\n  {ex}')
        mainret = 1
    except (EOFError, KeyboardInterrupt):
        print_err('\nUser cancelled.\n')
        mainret = 2
    except BrokenPipeError:
        print_err('\nBroken pipe, input/output was interrupted.\n')
        mainret = 3
    sys.exit(mainret)
