# Copyright (c) 2018-2019 Robin Jarry
# SPDX-License-Identifier: BSD-3-Clause

import os
import shlex

import cffi


HERE = os.path.dirname(__file__)

BUILDER = cffi.FFI()
with open(os.path.join(HERE, 'cdefs.h')) as f:
    BUILDER.cdef(f.read())

HEADERS = []
if 'LIBYANG_HEADERS' in os.environ:
    HEADERS.append(os.environ['LIBYANG_HEADERS'])
LIBRARIES = []
if 'LIBYANG_LIBRARIES' in os.environ:
    LIBRARIES.append(os.environ['LIBYANG_LIBRARIES'])
EXTRA_CFLAGS = ['-Werror', '-std=c99']
EXTRA_CFLAGS += shlex.split(os.environ.get('LIBYANG_EXTRA_CFLAGS', ''))
EXTRA_LDFLAGS = shlex.split(os.environ.get('LIBYANG_EXTRA_LDFLAGS', ''))

with open(os.path.join(HERE, 'source.c')) as f:
    BUILDER.set_source('_libyang', f.read(), libraries=['yang'],
                       extra_compile_args=EXTRA_CFLAGS,
                       extra_link_args=EXTRA_LDFLAGS,
                       include_dirs=HEADERS,
                       library_dirs=LIBRARIES,
                       py_limited_api=False)

if __name__ == '__main__':
    BUILDER.compile()
