#!/usr/bin/env python3
# Copyright (c) 2018-2020 Robin Jarry
# SPDX-License-Identifier: BSD-3-Clause

from distutils import dir_util
from distutils import log
from distutils.command.build_clib import build_clib
import os
import re
import subprocess
import sys

import setuptools
from setuptools.command.build_ext import build_ext
from setuptools.command.sdist import sdist


INSTALL_REQS = []
SETUP_REQS = []
if '_cffi_backend' not in sys.builtin_module_names:
    INSTALL_REQS.append('cffi')
    SETUP_REQS.append('cffi')
HERE = os.path.abspath(os.path.dirname(__file__))


class SDist(sdist):

    def make_distribution(self):
        base_name = self.distribution.get_fullname()
        base_dir = os.path.join(self.dist_dir, base_name)
        self.make_release_tree(base_dir, self.filelist.files)
        archive_files = []
        if 'tar' in self.formats:
            self.formats.append(self.formats.pop(self.formats.index('tar')))
        for fmt in self.formats:
            file = self.make_archive(base_dir, fmt, base_dir=base_name,
                                     root_dir=self.dist_dir,
                                     owner=self.owner, group=self.group)
            archive_files.append(file)
            self.distribution.dist_files.append(('sdist', '', file))
        self.archive_files = archive_files
        if not self.keep_temp:
            dir_util.remove_tree(base_dir, dry_run=self.dry_run)


class BuildCLib(build_clib):

    def run(self):
        if not self.libraries:
            return
        log.info('Building libyang C library ...')
        tmp = os.path.abspath(self.build_temp)
        staging = os.path.join(tmp, 'staging')
        cmd = [
            os.path.join(HERE, 'build-so.sh'),
            os.path.join(HERE, 'clib'),
            tmp,
            staging,
        ]
        log.info('+ %s' % ' '.join(cmd))
        subprocess.check_call(cmd)


class BuildExt(build_ext):

    def run(self):
        if self.distribution.has_c_libraries():
            if 'build_clib' not in self.distribution.have_run or \
                    not self.distribution.have_run['build_clib']:
                self.run_command('build_clib')
            tmp = os.path.abspath(
                self.get_finalized_command('build_clib').build_temp)
            self.include_dirs.append(os.path.join(tmp, 'staging/_include'))
            self.library_dirs.append(os.path.join(tmp, 'staging/_lib'))
            self.rpath.append('$ORIGIN/libyang/_lib')

        build_ext.run(self)

        if self.distribution.has_c_libraries():
            if self.inplace:
                build_py = self.get_finalized_command('build_py')
                dest = build_py.get_package_dir('libyang')
            else:
                dest = os.path.join(self.build_lib, 'libyang')
            if os.path.isdir(os.path.join(dest, '_lib')):
                # Work around dir_util.copy_tree() that fails when a symlink
                # already exists.
                for f in os.listdir(os.path.join(dest, '_lib')):
                    if os.path.islink(os.path.join(dest, '_lib', f)):
                        os.unlink(os.path.join(dest, '_lib', f))
            tmp = self.get_finalized_command('build_clib').build_temp
            dir_util.copy_tree(
                os.path.join(tmp, 'staging'), dest,
                preserve_symlinks=True, update=True)


LIBRARIES = []
if os.environ.get('LIBYANG_INSTALL', 'system') == 'embed':
    LIBRARIES.append(('yang', {'sources': ['clib']}))


def _version():
    forced_version = os.getenv('LIBYANG_PYTHON_VERSION')
    if forced_version is not None:
        return forced_version

    suffix = os.getenv('LIBYANG_PYTHON_VERSION_SUFFIX', '')

    try:
        with open('clib/CMakeLists.txt', 'rb') as f:
            buf = f.read().decode('utf-8')
        flags = re.MULTILINE
        major = int(re.search(
            r'^set\(LIBYANG_MAJOR_VERSION\s+(\d+)\)$', buf, flags).group(1))
        minor = int(re.search(
            r'^set\(LIBYANG_MINOR_VERSION\s+(\d+)\)$', buf, flags).group(1))
        micro = int(re.search(
            r'^set\(LIBYANG_MICRO_VERSION\s+(\d+)\)$', buf, flags).group(1))
        return '%d.%d.%d%s' % (major, minor, micro, suffix)
    except Exception:
        return '0.dev0' + suffix


with open('README.md', 'rb') as f:
    LONG_DESC = f.read().decode('utf-8')


setuptools.setup(
    name='libyang',
    version=_version(),
    description='CFFI bindings to libyang',
    long_description=LONG_DESC,
    long_description_content_type='text/markdown',
    url='https://github.com/CESNET/libyang',
    license='BSD 3 clause',
    author='Robin Jarry',
    author_email='robin@jarry.cc',
    keywords=['libyang', 'cffi'],
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Operating System :: Unix',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Topic :: Software Development :: Libraries',
    ],
    packages=['libyang'],
    zip_safe=False,
    include_package_data=True,
    python_requires='>=3.5',
    setup_requires=SETUP_REQS,
    install_requires=INSTALL_REQS,
    cffi_modules=['cffi/build.py:BUILDER'],
    libraries=LIBRARIES,
    cmdclass={
        'build_clib': BuildCLib,
        'build_ext': BuildExt,
        'sdist': SDist,
    },
)
