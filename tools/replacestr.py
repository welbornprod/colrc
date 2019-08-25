#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" replacestr.py
    Replaces strings in files.
    -Christopher Welborn 02-04-2019
"""

import os
import re
import sys

from colr import (
    ColrControl as C,
    Preset,
    auto_disable as colr_auto_disable,
    docopt,
)
from colr.controls import EraseMethod

from printdebug import DebugColrPrinter

debugprinter = DebugColrPrinter()
debug = debugprinter.debug
debug_err = debugprinter.debug_err
colr_auto_disable()

NAME = 'Replace Strings'
VERSION = '0.1.2'
VERSIONSTR = '{} v. {}'.format(NAME, VERSION)
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

SOURCE_EXTS = {
    '.asm',
    '.bash',
    '.c',
    '.coffee',
    '.cpp',
    '.cs',
    '.cson',
    '.css',
    '.h',
    '.hpp',
    '.hs',
    '.htm',
    '.html',
    '.js',
    '.json',
    '.log',
    '.md',
    '.pl',
    '.py',
    '.rs',
    '.rst',
    '.scss',
    '.sh',
    '.tex',
    '.toml',
    '.txt',
    '.xml',
}

USAGESTR = f"""{VERSIONSTR}
    Like `sed s/TARGET/REPL/`, but simpler.

    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} TARGET REPL [-i] [-D] [-V | -c] [-f file] [-o file]
        {SCRIPT} TARGET REPL [-i] [-D] [-E pat] [-I pat] [-V] -r FILE...
        {SCRIPT} TARGET REPL [-i] [-D] [-e exts] [-E pat] [-I pat] -R [FILE...]
        {SCRIPT} -l [-e exts] [-E pat] [-I pat] [FILE...]

    Options:
        FILE                  : Target file or files for replacement when
                                rewriting/refactoring in place.
        REPL                  : Replacement string.
        TARGET                : Regex pattern to replace.
        -c,--color            : Highlight replaced text.
        -D,--debug            : Debug mode, show more info while running.
        -E pat,--exclude pat  : Regex pattern for file paths to exclude.
        -e exts,--exts exts   : A comma or space-separated list of extensions
                                to include instead of the default extensions.
        -f file,--file file   : Target file for replacements.
                                * stdin is used if no -f flag is given.
        -h,--help             : Show this help message.
        -I pat,--include pat  : Regex pattern for file paths to include.
        -i,--ignorecase       : Make target case-insensitive.
        -l,--listfiles        : List files that would be searched during a
                                refactor, with optional include/exclude
                                patterns.
        -R,--refactor         : Use --rewrite and --view.
                                When no files are given, they are collected
                                recursively from the current directory.
                                Common source code file extensions
                                are matched. Binary files are not touched.
        -r,--rewrite          : Rewrite the input file/s, in place.
        -o file,--out file    : Target file for output.
                                * stdout is used if no -o flag is given.
        -v,--version          : Show version.
        -V,--view             : View the replaced lines before writing.
                                There is a chance to cancel the write.
"""

# Colr presets.
CNum = Preset('blue', style='bright')


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    debugprinter.enable(argd['--debug'])
    if argd['--listfiles'] or argd['--refactor']:
        argd['--rewrite'] = True
        argd['--view'] = True
        if not argd['FILE']:
            argd['FILE'] = list(iter_source_files(
                exts=parse_exts(argd['--exts']),
            ))

    if not argd['--file']:
        argd['FILE'] = list(filter_files(
            argd['FILE'],
            include_str=argd['--include'],
            exclude_str=argd['--exclude'],
        ))

    if argd['--listfiles']:
        return list_files(argd['FILE'])

    if not (argd['FILE'] or argd['--file']):
        raise InvalidArg('no files to work with.')

    pat = parse_repat(
        argd['TARGET'],
        ignore_case=argd['--ignorecase'],
        pat_for='target',
    )

    if argd['--rewrite']:
        return rewrite_files(
            argd['FILE'],
            pat,
            argd['REPL'],
            view_first=argd['--view'],
            silent_empty=argd['--refactor'],
            diff=argd['--refactor'],
        )

    # Single in/out file mode:
    infile = parse_file_arg(
        argd['--file'],
        default=sys.stdin,
        mode='r',
        viewmode=argd['--view'],
    )
    outfile = parse_file_arg(
        argd['--out'],
        default=sys.stdout,
        mode='w',
        viewmode=argd['--view'],
    )

    rf = ReplaceFile(
        infile,
        pat,
        argd['REPL'],
    )
    if argd['--view'] and (not rf.confirm_lines()):
        return 1

    rf.write_to(outfile, force_color=argd['--color'])

    return 0 if rf.line_cnt else 1


def confirm(s, default=False):
    """ Confirm a yes/no question. """
    if confirm.accept_all:
        return True

    if default:
        defaultstr = C('/', style='bright').join(
            C('Y', 'green'),
            C('n', 'red'),
            C('q', 'blue'),
            C('a', 'yellow'),
        )
    else:
        defaultstr = C('/', style='bright').join(
            C('y', 'green'),
            C('N', 'red'),
            C('q', 'blue'),
            C('a', 'yellow'),
        )
    s = '\n'.join((
        str(C(', ').join(
            C(' Q ', 'blue', style='bright').join(
                '\nType',
                'to cancel everything',
            ),
            C(' A ', 'blue', style='bright').join(
                'or',
                C(' all ', 'green').join('to accept', 'changes.'),
            ),
        )),
        '{} ({}): '.format(C(s, 'cyan'), defaultstr),
    ))
    try:
        answer = input(s).strip().lower()
    except EOFError:
        raise CancelledAll()
    else:
        if answer.startswith('q'):
            raise CancelledAll()
        elif answer.startswith('a'):
            confirm.accept_all = True
            return True

    # Erase the prompt.
    C().move_column(0).move_up().erase_line(EraseMethod.END).write()
    if s.startswith('\n'):
        C().move_up().erase_line(EraseMethod.END).write()
    # Erase the 'Type Q to quit, or A to accept all changes.' msg.
    C().move_up().erase_line(EraseMethod.END).move_up().write()
    if answer:
        return answer.startswith('y')

    # no answer, return the default.
    return default


# If this is set to a truthy value, confirm() simply returns True.
# It's used to implement the 'Confirm All' option.
confirm.accept_all = False


def filter_files(filepaths, include_str=None, exclude_str=None):
    include_pat = parse_repat(
        include_str,
        ignore_case=False,
        allow_empty=True,
        pat_for='include',
    )
    exclude_pat = parse_repat(
        exclude_str,
        ignore_case=False,
        allow_empty=True,
        pat_for='exclude',
    )

    for filepath in filepaths:
        if (
                (include_pat is not None) and
                (include_pat.search(filepath) is None)):
            continue
        if (
                (exclude_pat is not None) and
                (exclude_pat.search(filepath) is not None)):
            continue
        yield filepath


def format_file_changes(rf, errmsg=None):
    """ Format a ReplaceFile's changed lines count for printing. """
    namefmt = C(getattr(rf, 'name', rf), 'blue').rjust(25)
    width = 3
    if errmsg:
        return '{name}: {changes:>{width}} changes ({errmsg})'.format(
            name=namefmt,
            changes=C('0', 'cyan'),
            width=width,
            errmsg=C(errmsg, 'red'),
        )
    return '{name}: {changes:>{width}} {plural}'.format(
        name=namefmt,
        changes=C(rf.replace_cnt, 'cyan'),
        width=width,
        plural='change' if rf.replace_cnt == 1 else 'changes',
    )


def format_file_name(s):
    """ Format a file name for printing. """
    return str(C('').join(
        '\n',
        C(s, 'blue', style='bright'),
        ':',
    ))


def format_file_err(msg, indent=4):
    return '{}{}'.format(
        ' ' * indent,
        C(msg, 'red', style='bright'),
    )


def format_lbl(lbl, val, rjust=0, indent=0):
    """ Format a label/value pair string. """
    return C('').join(
        ' ' * indent,
        C(': ').join(
            C(str(lbl).rjust(rjust), 'cyan'),
            C(val, 'blue', style='bright'),
        )
    )


def format_lbls(lblinfo, rjust=0, indent=0):
    """ Format lines of label/value pairs.
        Arguments:
            lblinfo  : A list of tuples [('lbl1', 'val1'), ('lbl2', 'val2')]
            rjust    : rjust value for label.
    """
    return C('\n').join(
        format_lbl(l, v, rjust=rjust, indent=indent)
        for l, v in lblinfo
    )


def iter_source_files(exts=None):
    """ Walks CWD, and yields each file path that looks like a source code file.
    """
    cwd = os.getcwd()
    use_exts = exts or SOURCE_EXTS
    for root, dirs, files in os.walk(cwd):
        for filename in files:
            ext = os.path.splitext(filename)[-1]
            if ext not in use_exts:
                continue
            fullpath = os.path.join(root, filename)
            yield fullpath


def list_files(filepaths):
    """ Print a list of file paths. """
    if not filepaths:
        print_err('No files found.')
        return 1

    filelen = len(filepaths)
    plural = 'file' if filelen == 1 else 'files'
    print('Found {} {}:'.format(
        C(filelen, 'blue', style='bright'),
        plural,
    ))
    for filepath in sorted(filepaths):
        print('    {}'.format(filepath))
    return 0


def parse_exts(extstr):
    """ Parse a comma or space-separated list of file extensions. """
    if not extstr:
        return None
    exts = []
    for word in extstr.split():
        exts.extend((s for s in word.split(',') if s))
    return set((
        s if s.startswith('.') else '.{}'.format(s)
        for s in exts
    ))


def parse_file_arg(s, default=None, mode='r', viewmode=False):
    """ Parse an argument as a file name, and return a file object.
        Returns `default` if no `s` is given.
        Raises InvalidArg on error.
    """
    if not s:
        return default
    if os.path.isdir(s):
        if viewmode:
            print('\n'.join((
                format_file_name(s),
                format_file_err('Ignored directory.'),
            )))
        else:
            print(format_file_changes(s, errmsg='ignoring directory'))
        return default
    try:
        f = open(s, mode)
    except FileNotFoundError:
        raise InvalidArg('file doesn\'t exist: {}'.format(s))
    except EnvironmentError as ex:
        raise InvalidArg('cannot open file: {}\n{}'.format(s, ex))
    return f


def parse_repat(s, ignore_case=True, allow_empty=False, pat_for=None):
    """ Try compiling a regex pattern.
        On errors, InvaligArg is raised.
        On success, a compiled regex pattern is returned.
    """
    if not s:
        if allow_empty:
            return None
        raise InvalidArg('no pattern given.')
    try:
        p = re.compile(s, flags=re.IGNORECASE if ignore_case else 0)
    except re.error as ex:
        raise InvalidArg(
            'bad regex pattern{patfor}: {pat}\n{err}'.format(
                patfor='' if pat_for is None else ' for {}'.format(pat_for),
                pat=s,
                err=ex,
            ))
    return p


def print_err(*args, **kwargs):
    """ A wrapper for print() that uses stderr by default. """
    if kwargs.get('file', None) is None:
        kwargs['file'] = sys.stderr
    print(*args, **kwargs)


def rewrite_files(
        filepaths, pat, repl, view_first=False, silent_empty=False, diff=False):
    """ Rewrite multiple files, replacing `pat` matches with `repl` str.
    """
    skipped = 0
    totalchanges = 0
    confirm_opts = {'silent': silent_empty, 'diff': diff}
    for filepath in filepaths:
        infile = parse_file_arg(filepath, mode='r', viewmode=view_first)
        if infile is None:
            return 1
        rf = ReplaceFile(infile, pat, repl)
        if view_first and (not rf.confirm_lines(**confirm_opts)):
            skipped += 1
            continue
        totalchanges += rf.replace_cnt
        rf.rewrite()
        print(format_file_changes(rf))

    statuspcs = [
        C(': ').join('Files', CNum(len(filepaths))),
        C(': ').join('Changes', CNum(totalchanges)),
    ]
    if skipped:
        statuspcs.append(C(': ').join('Skipped', CNum(skipped)))
    print('\n{}'.format(C(', ').join(statuspcs)))
    return 0 if totalchanges else 1


class CancelledAll(KeyboardInterrupt):
    pass


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            try:
                lbl, val = self.msg.split(':')
            except ValueError:
                return 'Invalid argument, {}'.format(self.msg)
            msg = C('Invalid argument, {}'.format(lbl), 'red', style='bright')
            return str(
                C(':').join(msg, C(val, 'blue', style='bright'))
            )
        return 'Invalid argument!'


class ReplaceFile(object):

    def __init__(self, fileobj, pat, repl):
        self.fileobj = fileobj
        self.name = fileobj.name
        self.pat = pat
        self.repl = repl
        self.line_cnt = 0
        self.replace_cnt = 0
        self.lines = []
        self._replace()

    def __str__(self):
        return ''.join(str(rl) for rl in self.lines)

    def _replace(self):
        debug('Reading: {}'.format(self.name))
        try:
            for i, line in enumerate(self.fileobj):
                rl = ReplaceLine(line, i + 1, self.pat, self.repl)
                self.line_cnt += 1 if rl.modified else 0
                self.replace_cnt += rl.replace_cnt
                self.lines.append(rl)
        except UnicodeDecodeError:
            debug('Skipping non utf-8 file: {}'.format(self.name))
            return
        debug('Lines replaced: {}'.format(self.line_cnt))
        debug('Patterns found: {}'.format(self.replace_cnt))

    def confirm_lines(
            self, silent=False, only_matches=True, line_nums=True, diff=False):
        """ View lines in stdout, and confirm whether the output is okay.
            Returns True or False (okay, or not okay).
        """
        if silent and self.replace_cnt == 0:
            return False

        print(format_file_name(self.name))
        if self.replace_cnt == 0:
            print(format_file_err('No changes were made.'))
            return False

        sys.stdout.writelines(
            self.highlighted(
                only_matches=only_matches,
                line_nums=line_nums,
                indent=4,
                matched=True,
                diff=diff,
            )
        )
        print('\n{}'.format(
            format_lbls(
                (
                    ('Lines modified', self.line_cnt),
                    ('Patterns found', self.replace_cnt),
                ),
                indent=4,
            )
        ))
        shortname = os.path.split(self.name)[-1]
        return confirm('\nWrite the file ({})?'.format(shortname))

    def highlighted(
            self, only_matches=False, line_nums=False,
            indent=0, matched=False, diff=False):
        """ Return a list of ReplaceLines, with highlighted matches.
            If `matched` is True, the text that matched is highlighted,
            otherwise the replacement is highlighted.
        """
        if only_matches:
            return [
                rl.highlighted(
                    line_nums=line_nums,
                    indent=indent,
                    matched=matched,
                    diff=diff,
                )
                for rl in self.lines
                if rl.modified
            ]
        return [
            str(rl.highlighted(
                line_nums=line_nums,
                indent=indent,
                matched=matched,
                diff=diff,
            ))
            for rl in self.lines
        ]

    def modified_lines(self):
        """ Return a list of ReplaceLines that were modified. """
        return [
            rl
            for rl in self.lines
            if rl.modified
        ]

    def rewrite(self):
        """ Rewrite this file with all replacements. """
        try:
            self.fileobj.close()
        except OSError:
            pass
        with open(self.name, 'w') as f:
            self.write_to(f)

    def write_to(self, fileobj, force_color=False):
        """ Write all replacements to `fileobj`. """
        debug('{} to: {}'.format(
            'Rewriting' if self.name == fileobj.name else 'Writing',
            self.name,
        ))
        fileobj.writelines(
            str(rl)
            for rl in (self.highlighted() if force_color else self.lines)
        )


class ReplaceLine(object):
    def __init__(self, line, lineno, pat, repl):
        self.line = line
        self.lineno = lineno
        self.pat = pat
        self.repl = repl
        self.replaced = None
        self._highlighted = None
        self._highlighted_match = None
        self.replace_cnt = 0
        self.modified = False
        self._replace()

    def __str__(self):
        return self.replaced or self.line

    def _highlight_match(self, match):
        return str(C(match.group(), 'red', style='bright'))

    def _highlight_repl(self, match):
        return str(C(self.repl, 'green', style='bright'))

    def _replace(self):
        self.replaced = self.pat.sub(self.repl, self.line)
        self._highlighted = self.pat.sub(self._highlight_repl, self.line)
        self._highlighted_match = self.pat.sub(self._highlight_match, self.line)
        self.replace_cnt = len(list(self.pat.finditer(self.line)))
        self.modified = bool(self.replace_cnt)
        if self.modified:
            debug('Replaced line:\n  {}\n  {}'.format(self.line, self.replaced))

    def highlighted(self, line_nums=False, indent=0, matched=False, diff=False):
        """ If matched is True, the text that matched is highlighted,
            otherwise the replacement is highlighted.
        """
        if diff:
            pcs = [
                [C('-', 'red'), self._highlighted_match.rstrip()],
                [C('+', 'green'), self._highlighted.rstrip()],
            ]
            if line_nums:
                pcs = [
                    [
                        xs[0],
                        C('').join(
                            C(self.lineno, 'red' if i == 0 else 'green'),
                            ':'
                        ),
                        xs[1]
                    ]
                    for i, xs in enumerate(pcs)
                ]
            return '{}\n'.format(C('\n').join(C(' ').join(pc) for pc in pcs))

        line = self._highlighted_match if matched else self._highlighted
        if line_nums:
            s = str(C(': ').join(C(self.lineno, 'blue'), line))
        else:
            s = line
        return '{}\n'.format(''.join((' ' * indent, s.rstrip())))


if __name__ == '__main__':
    try:
        mainret = main(docopt(USAGESTR, version=VERSIONSTR))
    except InvalidArg as ex:
        print_err('\n{}'.format(ex))
        mainret = 1
    except (EOFError, KeyboardInterrupt):
        print_err(C('\nUser cancelled.\n', 'red'))
        mainret = 2
    except BrokenPipeError:
        print_err(C('\nBroken pipe, input/output was interrupted.\n', 'red'))
        mainret = 3
    sys.exit(mainret)
