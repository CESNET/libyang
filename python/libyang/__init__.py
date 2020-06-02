# Copyright (c) 2018-2019 Robin Jarry
# Copyright (c) 2020 6WIND S.A.
# SPDX-License-Identifier: BSD-3-Clause

import os  # isort:skip

# Important: the following *must* remain *before* the import of _libyang
HERE = os.path.dirname(__file__)
LIBDIR = os.path.join(HERE, '_lib')
INCLUDEDIR = os.path.join(HERE, '_include')
if os.path.isdir(LIBDIR):
    os.environ.setdefault(
        'LIBYANG_EXTENSIONS_PLUGINS_DIR', os.path.join(LIBDIR, 'extensions'))
    os.environ.setdefault(
        'LIBYANG_USER_TYPES_PLUGINS_DIR', os.path.join(LIBDIR, 'user_types'))

import logging

from _libyang import ffi
from _libyang import lib

from .data import DNode
from .data import data_format
from .data import parser_flags
from .data import path_flags
from .schema import Module
from .schema import SNode
from .schema import schema_in_format
from .util import LibyangError
from .util import c2str
from .util import str2c


LOG = logging.getLogger(__name__)
LOG.addHandler(logging.NullHandler())


#------------------------------------------------------------------------------
class Context:

    def __init__(self, search_path=None, disable_searchdir_cwd=True, pointer=None):
        if pointer is not None:
            self._ctx = ffi.cast('struct ly_ctx *', pointer)
            return  # already initialized

        options = 0
        if disable_searchdir_cwd:
            options |= lib.LY_CTX_DISABLE_SEARCHDIR_CWD

        self._ctx = lib.ly_ctx_new(ffi.NULL, options)
        if not self._ctx:
            raise self.error('cannot create context')

        search_dirs = []
        if 'YANGPATH' in os.environ:
            search_dirs.extend(
                os.environ['YANGPATH'].strip(': \t\r\n\'"').split(':'))
        elif 'YANG_MODPATH' in os.environ:
            search_dirs.extend(
                os.environ['YANG_MODPATH'].strip(': \t\r\n\'"').split(':'))
        if search_path:
            search_dirs.extend(search_path.strip(': \t\r\n\'"').split(':'))

        for path in search_dirs:
            if not os.path.isdir(path):
                continue
            if lib.ly_ctx_set_searchdir(self._ctx, str2c(path)) != 0:
                raise self.error('cannot set search dir')

    def destroy(self):
        if self._ctx is not None:
            lib.ly_ctx_destroy(self._ctx, ffi.NULL)
            self._ctx = None

    def __enter__(self):
        return self

    def __exit__(self, *args, **kwargs):
        self.destroy()

    def error(self, msg, *args):
        msg %= args

        if self._ctx:
            err = lib.ly_err_first(self._ctx)
            while err:
                if err.msg:
                    msg += ': %s' % c2str(err.msg)
                if err.path:
                    msg += ': %s' % c2str(err.path)
                err = err.next
            lib.ly_err_clean(self._ctx, ffi.NULL)

        return LibyangError(msg)

    def parse_module_file(self, fileobj, fmt='yang'):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        fmt = schema_in_format(fmt)
        mod = lib.lys_parse_fd(self._ctx, fileobj.fileno(), fmt)
        if not mod:
            raise self.error('cannot parse module')

        return Module(self, mod)

    def parse_module_str(self, s, fmt='yang'):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        fmt = schema_in_format(fmt)
        mod = lib.lys_parse_mem(self._ctx, str2c(s), fmt)
        if not mod:
            raise self.error('cannot parse module')

        return Module(self, mod)

    def load_module(self, name):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        mod = lib.ly_ctx_load_module(self._ctx, str2c(name), ffi.NULL)
        if not mod:
            raise self.error('cannot load module')

        return Module(self, mod)

    def get_module(self, name):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        mod = lib.ly_ctx_get_module(self._ctx, str2c(name), ffi.NULL, False)
        if not mod:
            raise self.error('cannot get module')

        return Module(self, mod)

    def find_path(self, path):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        node_set = lib.ly_ctx_find_path(self._ctx, str2c(path))
        if not node_set:
            raise self.error('cannot find path')
        try:
            for i in range(node_set.number):
                yield SNode.new(self, node_set.set.s[i])
        finally:
            lib.ly_set_free(node_set)

    def create_data_path(self, path, parent=None, value=None,
                         update=True, no_parent_ret=True, rpc_output=False,
                         force_return_value=True):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        lib.lypy_set_errno(0)
        if value is not None:
            if isinstance(value, bool):
                value = str(value).lower()
            elif not isinstance(value, str):
                value = str(value)
        flags = path_flags(
            update=update, no_parent_ret=no_parent_ret, rpc_output=rpc_output)
        dnode = lib.lyd_new_path(
            parent._node if parent else ffi.NULL,
            self._ctx, str2c(path), str2c(value), 0, flags)
        if lib.lypy_get_errno() != lib.LY_SUCCESS:
            if lib.ly_vecode(self._ctx) != lib.LYVE_PATH_EXISTS:
                raise self.error('cannot create data path: %s', path)
            lib.ly_err_clean(self._ctx, ffi.NULL)
            lib.lypy_set_errno(0)
        if not dnode and not force_return_value:
            return None

        if not dnode and parent:
            # This can happen when path points to an already created leaf and
            # its value does not change.
            # In that case, lookup the existing leaf and return it.
            node_set = lib.lyd_find_path(parent._node, str2c(path))
            try:
                if not node_set or not node_set.number:
                    raise self.error('cannot find path: %s', path)
                dnode = node_set.set.s[0]
            finally:
                lib.ly_set_free(node_set)

        if not dnode:
            raise self.error('cannot find created path')

        return DNode.new(self, dnode)

    def parse_data_mem(self, d, fmt, data=False, config=False, strict=False,
                       trusted=False, no_yanglib=False, rpc=False):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        flags = parser_flags(
            data=data, config=config, strict=strict, trusted=trusted,
            no_yanglib=no_yanglib, rpc=rpc)
        fmt = data_format(fmt)
        if fmt == lib.LYD_LYB:
            d = str2c(d, encode=False)
        else:
            d = str2c(d, encode=True)
        args = []
        if rpc:
            args.append(ffi.cast('struct lyd_node *', ffi.NULL))
        dnode = lib.lyd_parse_mem(self._ctx, d, fmt, flags, *args)
        if not dnode:
            raise self.error('failed to parse data tree')
        return DNode.new(self, dnode)

    def parse_data_file(self, fileobj, fmt, data=False, config=False,
                        strict=False, trusted=False, no_yanglib=False, rpc=False):
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        flags = parser_flags(
            data=data, config=config, strict=strict, trusted=trusted,
            no_yanglib=no_yanglib, rpc=rpc)
        fmt = data_format(fmt)
        args = []
        if rpc:
            args.append(ffi.cast('struct lyd_node *', ffi.NULL))
        dnode = lib.lyd_parse_fd(self._ctx, fileobj.fileno(), fmt, flags, *args)
        if not dnode:
            raise self.error('failed to parse data tree')
        return DNode.new(self, dnode)

    def __iter__(self):
        """
        Return an iterator that yields all implemented modules from the context
        """
        if self._ctx is None:
            raise RuntimeError('context already destroyed')
        idx = ffi.new('uint32_t *')
        mod = lib.ly_ctx_get_module_iter(self._ctx, idx)
        while mod:
            yield Module(self, mod)
            mod = lib.ly_ctx_get_module_iter(self._ctx, idx)


#------------------------------------------------------------------------------
LOG_LEVELS = {
    lib.LY_LLERR: logging.ERROR,
    lib.LY_LLWRN: logging.WARNING,
    lib.LY_LLVRB: logging.INFO,
    lib.LY_LLDBG: logging.DEBUG,
}


@ffi.def_extern(name='lypy_log_cb')
def libyang_c_logging_callback(level, msg, path):
    args = [c2str(msg)]
    if path:
        fmt = '%s: %s'
        args.append(c2str(path))
    else:
        fmt = '%s'
    LOG.log(LOG_LEVELS.get(level, logging.NOTSET), fmt, *args)


def configure_logging(enable_py_logger, level=logging.ERROR):
    """
    Configure libyang logging behaviour.

    :arg bool enable_py_logger:
        If False, configure libyang to store the errors in the context until
        they are consumed when Context.error() is called. This is the default
        behaviour.

        If True, libyang log messages will be sent to the python 'libyang'
        logger and will be processed according to the python logging
        configuration. Note that by default, the 'libyang' python logger is
        created with a NullHandler() which means that all messages are lost
        until another handler is configured for that logger.
    :arg int level:
        Python logging level. By default only ERROR messages are stored/logged.
    """
    for ly_lvl, py_lvl in LOG_LEVELS.items():
        if py_lvl == level:
            lib.ly_verb(ly_lvl)
            break
    if enable_py_logger:
        lib.ly_log_options(lib.LY_LOLOG |lib.LY_LOSTORE)
        lib.ly_set_log_clb(lib.lypy_log_cb, True)
    else:
        lib.ly_log_options(lib.LY_LOSTORE)
        lib.ly_set_log_clb(ffi.NULL, False)


configure_logging(False, logging.ERROR)


#------------------------------------------------------------------------------
def lib_dirs():
    dirs = []
    if os.path.isdir(LIBDIR):
        dirs.append(LIBDIR)
    return dirs


#------------------------------------------------------------------------------
def include_dirs():
    dirs = []
    if os.path.isdir(INCLUDEDIR):
        dirs.append(INCLUDEDIR)
    return dirs
