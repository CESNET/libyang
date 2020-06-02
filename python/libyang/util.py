# Copyright (c) 2018-2019 Robin Jarry
# SPDX-License-Identifier: BSD-3-Clause

from _libyang import ffi


#------------------------------------------------------------------------------
class LibyangError(Exception):
    pass


#------------------------------------------------------------------------------
def str2c(s, encode=True):
    if s is None:
        return ffi.NULL
    if hasattr(s, 'encode'):
        s = s.encode('utf-8')
    return ffi.new('char []', s)


#------------------------------------------------------------------------------
def c2str(c, decode=True):
    if c == ffi.NULL:
        return None
    s = ffi.string(c)
    if hasattr(s, 'decode'):
        s = s.decode('utf-8')
    return s
