#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" make_deb_dir.py
    ColrC Debian directory generator.
    This is used by make_dist.sh to generate debian packages for the colr tool
    and libcolr.

    -Christopher Welborn 03-25-2020
"""

import os
import shutil
import subprocess
import sys
from collections import UserList

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

NAME = 'ColrC Deb Generator'
VERSION = '0.0.1'
VERSIONSTR = f'{NAME} v. {VERSION}'
SCRIPT = os.path.split(os.path.abspath(sys.argv[0]))[1]
SCRIPTDIR = os.path.abspath(sys.path[0])

COLRC_SRC = os.path.abspath(os.path.join(SCRIPTDIR, '..'))
COLRC_GET_VER = os.path.join(COLRC_SRC, 'tools/get_version.sh')
COLRC_VERSION = subprocess.check_output(
    ['bash', COLRC_GET_VER]
).decode().strip()
COLRC_ARCH = subprocess.check_output(
    ['dpkg', '--print-architecture']
).decode().strip()

USAGESTR = f"""{VERSIONSTR} for ColrC {COLRC_VERSION} ({COLRC_ARCH})
    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-D] [-a | -l]

    Options:
        -a,--all      : Make all packages (colr and libcolr).
        -D,--debug    : Show more info while running.
        -h,--help     : Show this help message.
        -l,--lib      : Make a directory for libcolr, instead of the colr tool.
        -v,--version  : Show version.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """

    if argd['--all']:
        pkgs = list(deb_pkgs)
    else:
        pkgs = ['libcolr' if argd['--lib'] else 'colr']
    for pkgname in pkgs:
        debpkg = deb_pkgs[pkgname]
        print(f'\n{C(debpkg)}')

    return 0


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


def try_cd(dirpath):
    try:
        os.chdir(dirpath)
    except EnvironmentError as ex:
        raise FatalError(
            '\n'.join((
                f'Unable to change dir: {dirpath}',
                f'\nError: {ex}',
            ))
        ) from None


def try_cp(src, dest):
    try:
        shutil.copy2(src, dest)
    except EnvironmentError as ex:
        raise FatalError(
            '\n'.join((
                'Unable to copy file:',
                f'        {src}',
                f'    to: {dest}',
                f'\nError: {ex}',
            ))
        ) from None


def try_ln(symlink, target):
    try:
        os.symlink(symlink, target)
    except EnvironmentError as ex:
        raise FatalError(
            '\n'.join((
                'Unable to symlink file:',
                f'        {target}',
                f'    to: {symlink}',
                f'\nError: {ex}',
            ))
        ) from None


def try_makedirs(dirpath):
    try:
        os.makedirs(dirpath)
    except EnvironmentError as ex:
        raise FatalError(
            f'Unable to create directory: {dirpath}\n{ex}'
        ) from None


class ChangeDir(object):
    """ Context manager to switch directories for some code. """
    def __init__(self, destdir):
        self.cwd = os.getcwd()
        self.destdir = destdir

    def __enter__(self):
        try:
            os.chdir(self.destdir)
        except EnvironmentError as ex:
            raise FatalError(f'Unable to cd back to: {self.cwd}\nError: {ex}')

    def __exit__(self, exc_type, exc, tb):
        try:
            os.chdir(self.cwd)
        except EnvironmentError as ex:
            raise FatalError(f'Unable to cd back to: {self.cwd}\nError: {ex}')
        return None


class DebFile(object):
    def __init__(
            self, *,
            pkgname=None, srcpath=None, destdir=None, linkdir=None):
        if not all((pkgname, srcpath, destdir, linkdir)):
            raise FatalError('Missing arguments for DebFile()!')

        self.pkgname = pkgname
        self.srcpath = srcpath
        self.destdir = destdir.lstrip('/')
        self.linkdir = linkdir.lstrip('/')
        self.filename = os.path.split(self.srcpath)[-1]
        self.debdir = f'{pkgname}_{COLRC_VERSION}'
        self.debiandir = os.path.join(self.debdir, 'DEBIAN')
        self.destdir_deb = os.path.join(self.debiandir, self.destdir)
        self.destpath = os.path.join(self.destdir_deb, self.filename)
        self.linkdir_deb = os.path.join(self.debiandir, self.linkdir)
        self.linkrel = self._get_rel_link()
        self.linkpath = os.path.join(self.linkrel, self.filename)

    def __colr__(self):
        linkdest = f'{self.linkdir_deb}/{self.filename}'
        return C('\n').join((
            C(f'{self.srcpath}'),
            f'⮞ {C(self.destpath, "blue")}',
            f'   ⮞ {C(linkdest, "blue", style="italic")}',
        ))

    def __hash__(self):
        return hash(self.srcpath)

    def __repr__(self):
        return '\n'.join((
            f'Source file: {self.srcpath}',
            f' Debian dir: {self.destdir_deb}',
            f'Debian file: {self.destpath}',
            f'   Link dir: {self.linkdir_deb}',
            f'   Link rel: {self.linkpath}',
        ))

    def __str__(self):
        return '\n'.join((
            f'{self.srcpath}',
            f'⮞ {self.destpath}',
            f'   ⮞ {self.linkdir_deb}/{self.filename}',
        ))

    def _get_rel_link(self):
        top = self._get_top()
        s = self.destdir_deb
        # Collect relative cd's.
        ld = self.linkdir_deb
        lnks = []
        while not s.startswith(ld):
            ld = os.path.split(ld)[0]
            lnks.append('..')
        # Collect required path ending.
        lnkend = []
        while s != top:
            s, end = os.path.split(s)
            lnkend.append(end)

        lnk = os.path.join(*lnks, *reversed(lnkend))
        return lnk

    def _get_top(self):
        s = self.linkdir_deb
        while not self.destpath.startswith(s):
            s = os.path.split(s)[0]
        return s

    def create(self):
        self.create_dirs()
        self.create_file()
        self.create_link()

    def create_dirs(self):
        try_makedirs(self.destdir_deb)
        try_makedirs(self.linkdir_deb)

    def create_file(self):
        try_cp(self.srcpath, self.destpath)

    def create_link(self):
        if not os.path.isdir(self.linkdir_deb):
            raise FatalError(f'Directory not created: {self.linkdir_deb}')
        with ChangeDir(f'{self.linkdir_deb}'):
            try_ln(self.linkpath, self.filename)


class DebPackage(UserList):
    def __init__(self, pkgname, debfiles=None):
        super().__init__(debfiles or [])
        self.pkgname = pkgname
        self.debdir = f'{pkgname}_{COLRC_VERSION}'

    def __colr__(self):
        files = C('\n    ').join(
            C(df).replace('\n', '\n    ')
            for df in self
        )
        pkgname = C(self.pkgname, fore='blue', style='bright')
        nofiles = C('no files', 'red').join('<', '>')
        return C(f'{pkgname}:\n    {files or nofiles}')

    def __str__(self):
        files = '\n    '.join(
            str(df).replace('\n', '\n    ')
            for df in self
        )
        return f'{self.pkgname}:\n    {files or "<no files>"}'

    def create_files(self):
        if os.path.exists(self.debdir):
            print(f'Removing existing dir: {self.debdir}')
            shutil.rmtree(self.debdir)
        for df in self:
            print(f'Creating strucure for: {df.filename}')
            df.create()


class FatalError(ValueError):
    """ Raised when the program should exit on error. """
    def __init__(self, msg):
        self.msg = msg or 'Fatal error (not specified)!'

    def __str__(self):
        return self.msg


class InvalidArg(ValueError):
    """ Raised when the user has used an invalid argument. """
    def __init__(self, msg=None):
        self.msg = msg or ''

    def __str__(self):
        if self.msg:
            return f'Invalid argument, {self.msg}'
        return 'Invalid argument!'


deb_pkgs = {
    'colr': DebPackage(
        'colr',
        [
            DebFile(
                pkgname='colr',
                srcpath=os.path.join(COLRC_SRC, 'colrc'),
                destdir='/usr/share/colr/bin',
                linkdir='/usr/bin',
            ),
        ],
    ),
    'libcolr': DebPackage(
        'libcolr',
        [
            DebFile(
                pkgname='libcolr',
                srcpath=os.path.join(COLRC_SRC, 'libcolr.so'),
                destdir='/usr/share/colr/lib',
                linkdir='/usr/lib',
            ),
            DebFile(
                pkgname='libcolr',
                srcpath=os.path.join(COLRC_SRC, 'colr.h'),
                destdir='/usr/share/colr/include',
                linkdir='/usr/include',
            ),
        ],
    ),
}


if __name__ == '__main__':
    try:
        mainret = main(docopt(USAGESTR, version=VERSIONSTR, script=SCRIPT))
    except (FatalError, InvalidArg) as ex:
        print_err(ex)
        mainret = 1
    except (EOFError, KeyboardInterrupt):
        print_err('\nUser cancelled.\n')
        mainret = 2
    except BrokenPipeError:
        print_err('\nBroken pipe, input/output was interrupted.\n')
        mainret = 3
    sys.exit(mainret)
