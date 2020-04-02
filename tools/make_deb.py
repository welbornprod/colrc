#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" make_deb_dir.py
    ColrC Debian directory generator.
    This is used by make_dist.sh to generate debian packages for the colr tool
    and libcolr.

    -Christopher Welborn 03-25-2020
"""

import gzip
import os
import shutil
import subprocess
import sys
from collections import UserList
from functools import partial

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

CONTROL = f"""
Package: {{pkgname}}
Version: {COLRC_VERSION}
Architecture: {COLRC_ARCH}
Maintainer: Christopher Welborn (cjwelborn@live.com)
Homepage: https://welbornprod.com/colrc
Vcs-Browser: https://github.com/welbornprod/colrc
Vcs-Git: git://github.com/welbornprod/colrc.git
Priority: optional
License: MIT
Description: ColrC is a C library and a command-line tool for terminal colors
             on Linux.
""".lstrip()

USAGESTR = f"""{VERSIONSTR} for ColrC {COLRC_VERSION} ({COLRC_ARCH})

    Creates debian packages for ColrC

    Usage:
        {SCRIPT} -h | -v
        {SCRIPT} [-D] [-a | -l] [-L | -F] [-q] [-d dir]

    Options:
        -a,--all          : Make all packages (colr and libcolr).
        -D,--debug        : Show more info while running.
        -d dir,--dir dir  : Destination directory for packages.
        -F,--listfiles    : Like -L, but also lists files that come from
        -h,--help         : Show this help message.
        -L,--list         : List package files, don't create anything.
                            whole-directory copies.
        -l,--lib          : Create the libcolr package, instead of colr.
        -q,--quiet        : Don't print status messages.
                            Only print the resulting package names.
        -v,--version      : Show version.
"""


def main(argd):
    """ Main entry point, expects docopt arg dict as argd. """
    global status
    if argd['--quiet']:
        status = noop

    if argd['--all']:
        pkgs = list(deb_pkgs)
    else:
        pkgs = ['libcolr' if argd['--lib'] else 'colr']
    for pkgname in pkgs:
        debpkg = deb_pkgs[pkgname]
        debpkg.dest_dir = argd['--dir']
        if argd['--list'] or argd['--listfiles']:
            print(f'\n{debpkg.as_colr(list_files=argd["--listfiles"])}')
        else:
            debpkg.create(name_only=argd['--quiet'])

    return 0


def compress_file(filepath, dest_dir=None, in_place=False, prefix=None):
    """ GZips a file and returns f'{filepath}.gz' on success.
        Arguments:
            dest_dir  : Directory to place the gzipped file in.
            in_place  : Whether the original file is removed on success.
            prefix    : Either a string prefix to prepend, or a callable
                        that takes the file name as an argument and returns
                        a file name to use.
                        def my_prefix(filename):
                            return f'my-{filename}'
    """
    try:
        with open(filepath, 'rb') as f:
            compressed = gzip.compress(f.read())
    except EnvironmentError as ex:
        raise FatalError(
            f'Unable to read file: {filepath}\nError: {ex}'
        ) from None

    newpath = f'{filepath}.gz'
    origdir, filename = os.path.split(newpath)
    if callable(prefix):
        filename = prefix(filename)
    elif prefix:
        filename = f'{prefix}{filename}'
    if dest_dir:
        newpath = os.path.join(dest_dir, filename)
    else:
        newpath = os.path.join(origdir, filename)
    try:
        with open(newpath, 'wb') as g:
            g.write(compressed)
    except EnvironmentError as ex:
        raise FatalError(
            f'Unable to write compressed file: {newpath}\nError: {ex}'
        ) from None
    if in_place:
        try:
            os.remove(filepath)
        except EnvironmentError as ex:
            raise FatalError(
                f'Unable to remove original file: {filepath}\nError: {ex}'
            )
    return newpath


def ensure_colr_prefix(filename):
    if filename.lower().startswith('colr'):
        return filename
    return f'colr-{filename}'


def noop(*args, **kwargs):
    """ This function cab replace any other function to disable things. """
    return None


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


def status(*args, **kwargs):
    print(*args, **kwargs)


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
        os.makedirs(dirpath, exist_ok=True)
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


class DebDirFiles(UserList):
    trans_char = '✪ '

    def __init__(
            self, *,
            pkgname=None, srcdir=None, destdir=None, linkdir=None,
            exts=None, transform=None):
        if not all((pkgname, srcdir, destdir, linkdir)):
            raise FatalError('Missing arguments for DebDirFiles()!')
        try:
            srcfiles = os.listdir(srcdir)
        except EnvironmentError as ex:
            raise FatalError(f'Unable to list files in: {srcdir}\nError: {ex}')
        if exts:
            srcfiles = [s for s in srcfiles if s.endswith(exts)]
        self.transform = transform or None
        super().__init__([
            DebFile(
                pkgname=pkgname,
                srcpath=os.path.join(srcdir, s),
                destdir=destdir,
                linkdir=linkdir,
                transform=transform,
            )
            for s in srcfiles
        ])
        self.pkgname = pkgname
        self.srcdir = srcdir
        extmarker = '*'
        if len(exts or []) == 1:
            extmarker = f'*{exts[0]}'
        else:
            extstr = ','.join(exts)
            extmarker = f'*{{{extstr}}}' if extstr else '*'
        self.srcdir_str = f'{self.srcdir}/{extmarker}'
        self.destdir = destdir
        self.linkdir = linkdir

    def __bool__(self):
        return bool(self.data)

    def __colr__(self):
        return self.as_colr(list_files=False)

    def __str__(self):
        return '\n'.join((
            f'{self.srcdir_str}',
            f'⮞ {self.destdir}',
            f'   ⮞ {self.linkdir}',
        ))

    def as_colr(self, list_files=False):
        if self.transform:
            func = getattr(self.transform, 'func', self.transform)
            funcname = getattr(func, '__name__', '??')
            transname = C('').join(
                self.trans_char,
                C(funcname, fore='yellow'),
                C('filepath', 'cyan').join('(', ')'),
            )
        else:
            transname = None
        pcs = []
        if list_files:
            if transname:
                transname = transname.replace(
                    self.trans_char,
                    f'      {self.trans_char}'
                )
                pcs.extend(
                    C('\n').join(
                        C(df),
                        transname,
                    )
                    for df in self
                )
            else:
                pcs.extend(C(df) for df in self)
        else:
            pcs.append(C(f'{self.srcdir_str}'))
            if transname:
                pcs.append(transname)
            pcs.extend((
                f'⮞ {C(self.destdir, "blue")}',
                f'   ⮞ {C(self.linkdir, "blue", style="italic")}',
            ))
        return C('\n').join(pcs)

    def create(self):
        status(f'Creating structure for: {self.srcdir_str}')
        for df in self:
            df.create(quiet=True)


class DebFile(object):
    def __init__(
            self, *,
            pkgname=None, srcpath=None, destdir=None, linkdir=None,
            transform=None):
        if not all((pkgname, srcpath, destdir, linkdir)):
            raise FatalError('Missing arguments for DebFile()!')

        self.pkgname = pkgname
        self.srcpath = srcpath
        self.destdir = destdir.lstrip('/')
        self.linkdir = linkdir.lstrip('/')
        # Should be a function that accepts the srcfile, and returns another
        # (changed) file path. The changed file path will be will be the new
        # destination file (the original will not be included in the package).
        # It should accept a `dest_dir` kwarg (`self.destdir_deb`).
        self.transform = transform or None
        # Calculated attributes.
        self.debdir = f'{self.pkgname}_{COLRC_VERSION}'
        self.debiandir = os.path.join(self.debdir, 'DEBIAN')
        self.destdir_deb = os.path.join(self.debdir, self.destdir)
        self.linkdir_deb = os.path.join(self.debdir, self.linkdir)
        self.filename = None
        self.destpath = None
        self.linkrel = None
        self.linkpath = None
        self._calc_paths()

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

    def _calc_paths(self, filename=None):
        """ Calculate destination/link paths based on `self.srcpath`.
        """
        self.filename = filename or os.path.split(self.srcpath)[-1]
        self.destpath = os.path.join(self.destdir_deb, self.filename)
        self.linkrel = self._get_rel_link()
        self.linkpath = os.path.join(self.linkrel, self.filename)

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

    def as_colr(self, list_files=False):
        """ This is exactly like C(self).
            `list_files` is for compatibility with DebDirFiles.
            It is not used.
        """
        return C(self)

    def create(self, quiet=False):
        if not quiet:
            status(f'Creating structure for: {self.filename}')
        self.create_dirs()
        self.create_file()
        self.create_link()

    def create_dirs(self):
        try_makedirs(self.destdir_deb)
        try_makedirs(self.linkdir_deb)

    def create_file(self):
        if self.transform:
            self.destpath = self.transform(
                self.srcpath,
                dest_dir=self.destdir_deb
            )
            self._calc_paths(filename=os.path.split(self.destpath)[-1])
        else:
            try_cp(self.srcpath, self.destpath)

    def create_link(self):
        if not os.path.isdir(self.linkdir_deb):
            raise FatalError(f'Directory not created: {self.linkdir_deb}')
        with ChangeDir(f'{self.linkdir_deb}'):
            try_ln(self.linkpath, self.filename)


class DebPackage(UserList):
    def __init__(self, pkgname, debfiles=None, dest_dir=None, triggers=None):
        super().__init__(debfiles or [])
        self.pkgname = pkgname
        self.debdir = f'{pkgname}_{COLRC_VERSION}'
        self.debname = f'{self.debdir}_{COLRC_ARCH}.deb'
        self.control_dir = os.path.join(self.debdir, 'DEBIAN')
        self.build_cmd = [
            'dpkg-deb',
            '--build',
            '--root-owner-group',
            self.debdir,
        ]
        self.dest_dir = dest_dir
        self.triggers = triggers or []

    def __colr__(self):
        return self.as_colr(list_files=False)

    def __str__(self):
        files = '\n    '.join(
            str(df).replace('\n', '\n    ')
            for df in self
        )
        return f'{self.pkgname}:\n    {files or "<no files>"}'

    def as_colr(self, list_files=False):
        files = C('\n    ').join(
            df.as_colr(list_files=list_files).replace('\n', '\n    ')
            for df in self
        )
        pkgname = C(self.pkgname, fore='blue', style='bright')
        nofiles = C('no files', 'red').join('<', '>')
        return C(f'{pkgname}:\n    {files or nofiles}')

    def build(self):
        cmd = self.build_cmd[:]
        if self.dest_dir:
            debfile = os.path.join(self.dest_dir, self.debname)
        else:
            debfile = self.debname
        cmd.append(debfile)
        try:
            output = subprocess.check_output(cmd)
        except subprocess.CalledProcessError as ex:
            if ex.output:
                print_err(ex.output.decode())
            raise FatalError(
                f'Failed to build package, dpkg-deb returned: {ex.returncode}'
            )
        else:
            status(output.decode())
        return debfile

    def clean(self):
        if os.path.exists(self.debdir):
            status(f'Cleaning dir: {self.debdir}')
            os.system(f'sudo rm -r {self.debdir}')

    def create(self, name_only=False):
        self.clean()
        self.create_files()
        self.create_control()
        self.create_triggers()
        self.create_perms()
        debfile = self.build()
        self.clean()
        if name_only:
            print(debfile)
        else:
            status(f'Created package: {debfile}')

    def create_control(self):
        self.ensure_control_dir()
        control_file = os.path.join(self.control_dir, 'control')
        try:
            with open(control_file, 'w') as f:
                f.write(CONTROL.format(pkgname=self.pkgname))
        except EnvironmentError as ex:
            raise FatalError(
                f'Unable to write control file: {control_file}\nError: {ex}'
            ) from None

    def create_files(self):
        self.create_control()
        for df in self:
            df.create()

    def create_perms(self):
        os.system(f'sudo chown -R root:root {self.debdir}')

    def create_triggers(self):
        """ This is not used right now. """
        if not self.triggers:
            return None
        self.ensure_control_dir()
        triggers_file = os.path.join(self.control_dir, 'triggers')
        try:
            with open(triggers_file, 'w') as f:
                f.write('\n'.join(self.triggers))
        except EnvironmentError as ex:
            raise FatalError(
                f'Unable to write triggers file: {triggers_file}\nError: {ex}'
            ) from None

    def ensure_control_dir(self):
        if not os.path.exists(self.control_dir):
            try_makedirs(self.control_dir)


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
        'libcolr-dev',
        [
            DebFile(
                pkgname='libcolr-dev',
                srcpath=os.path.join(COLRC_SRC, 'libcolr.so'),
                destdir='/usr/share/colr/lib',
                linkdir='/usr/lib',
            ),
            DebFile(
                pkgname='libcolr-dev',
                srcpath=os.path.join(COLRC_SRC, 'colr.h'),
                destdir='/usr/share/colr/include',
                linkdir='/usr/include',
            ),
            DebDirFiles(
                pkgname='libcolr-dev',
                srcdir=os.path.join(COLRC_SRC, 'docs/man/man3'),
                destdir='/usr/share/colr/man3',
                linkdir='/usr/share/man/man3',
                exts=('.3', ),
                transform=partial(compress_file, prefix=ensure_colr_prefix),
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
