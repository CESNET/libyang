# Copyright (c) 2020 6WIND S.A.
# SPDX-License-Identifier: BSD-3-Clause

import logging

from _libyang import ffi
from _libyang import lib

from .schema import Module
from .schema import SContainer
from .schema import SLeaf
from .schema import SLeafList
from .schema import SList
from .schema import SNode
from .schema import SRpc
from .schema import Type
from .util import LibyangError
from .util import c2str
from .util import str2c


LOG = logging.getLogger(__name__)


#------------------------------------------------------------------------------
def printer_flags(with_siblings=False, pretty=False, keep_empty_containers=False,
                  trim_default_values=False, include_implicit_defaults=False):
    flags = 0
    if with_siblings:
        flags |= lib.LYP_WITHSIBLINGS
    if pretty:
        flags |= lib.LYP_FORMAT
    if keep_empty_containers:
        flags |= lib.LYP_KEEPEMPTYCONT
    if trim_default_values:
        flags |= lib.LYP_WD_TRIM
    if include_implicit_defaults:
        flags |= lib.LYP_WD_ALL
    return flags


#------------------------------------------------------------------------------
def data_format(fmt_string):
    if fmt_string == 'json':
        return lib.LYD_JSON
    if fmt_string == 'xml':
        return lib.LYD_XML
    if fmt_string == 'lyb':
        return lib.LYD_LYB
    raise ValueError('unknown data format: %r' % fmt_string)


#------------------------------------------------------------------------------
def path_flags(update=False, rpc_output=False, no_parent_ret=False):
    flags = 0
    if update:
        flags |= lib.LYD_PATH_OPT_UPDATE
    if rpc_output:
        flags |= lib.LYD_PATH_OPT_OUTPUT
    if no_parent_ret:
        flags |= lib.LYD_PATH_OPT_NOPARENTRET
    return flags


#------------------------------------------------------------------------------
def parser_flags(data=False, config=False, get=False, strict=False,
                 trusted=False, no_yanglib=False, rpc=False, rpcreply=False,
                 destruct=False, no_siblings=False, explicit=False):
    flags = 0
    if data:
        flags |= lib.LYD_OPT_DATA
    if config:
        flags |= lib.LYD_OPT_CONFIG
    if get:
        flags |= lib.LYD_OPT_GET
    if strict:
        flags |= lib.LYD_OPT_STRICT
    if trusted:
        flags |= lib.LYD_OPT_TRUSTED
    if no_yanglib:
        flags |= lib.LYD_OPT_DATA_NO_YANGLIB
    if rpc:
        flags |= lib.LYD_OPT_RPC
    if rpcreply:
        flags |= lib.LYD_OPT_RPCREPLY
    if destruct:
        flags |= lib.LYD_OPT_DESTRUCT
    if no_siblings:
        flags |= lib.LYD_OPT_NOSIBLINGS
    if explicit:
        flags |= lib.LYD_OPT_EXPLICIT
    return flags


#------------------------------------------------------------------------------
class DNode:
    """
    Data tree node.
    """
    def __init__(self, context, node_p):
        """
        :arg Context context:
            The libyang.Context python object.
        :arg struct lyd_node * node_p:
            The pointer to the C structure allocated by libyang.so.
        """
        self.context = context
        self._node = ffi.cast('struct lyd_node *', node_p)

    def name(self):
        return c2str(self._node.schema.name)

    def module(self):
        mod = lib.lyd_node_module(self._node)
        if not mod:
            raise self.context.error('cannot get module')
        return Module(self.context, mod)

    def schema(self):
        return SNode.new(self.context, self._node.schema)

    def parent(self):
        if not self._node.parent:
            return None
        return self.new(self.context, self._node.parent)

    def root(self):
        node = self
        while node.parent() is not None:
            node = node.parent()
        return node

    def first_sibling(self):
        n = lib.lyd_first_sibling(self._node)
        if n == self._node:
            return self
        return self.new(self.context, n)

    def siblings(self, include_self=True):
        n = lib.lyd_first_sibling(self._node)
        while n:
            if n == self._node:
                if include_self:
                    yield self
            else:
                yield self.new(self.context, n)
            n = n.next

    def find_one(self, xpath):
        try:
            return next(self.find_all(xpath))
        except StopIteration:
            return None

    def find_all(self, xpath):
        node_set = lib.lyd_find_path(self._node, str2c(xpath))
        if not node_set:
            raise self.context.error('cannot find path')
        try:
            for i in range(node_set.number):
                yield DNode.new(self.context, node_set.d[i])
        finally:
            lib.ly_set_free(node_set)

    def path(self):
        path = lib.lyd_path(self._node)
        try:
            return c2str(path)
        finally:
            lib.free(path)

    def validate(self, data=False, config=False, get=False, rpc=False,
                 rpcreply=False, no_yanglib=False):
        flags = parser_flags(
            data=data, config=config, get=get, rpc=rpc,
            rpcreply=rpcreply, no_yanglib=no_yanglib)
        node_p = ffi.new('struct lyd_node **')
        node_p[0] = self._node
        ret = lib.lyd_validate(node_p, flags, ffi.NULL)
        if ret != 0:
            self.context.error('validation failed')

    def merge(self, source, destruct=False, no_siblings=False, explicit=False):
        flags = parser_flags(destruct=destruct, no_siblings=no_siblings,
                             explicit=explicit)
        ret = lib.lyd_merge(self._node, source._node, flags)
        if ret != 0:
            raise self.context.error('merge failed')

    def print_mem(self, fmt,
                  with_siblings=False,
                  pretty=False,
                  include_implicit_defaults=False,
                  trim_default_values=False,
                  keep_empty_containers=False):
        flags = printer_flags(
            with_siblings=with_siblings, pretty=pretty,
            include_implicit_defaults=include_implicit_defaults,
            trim_default_values=trim_default_values,
            keep_empty_containers=keep_empty_containers)
        buf = ffi.new('char **')
        fmt = data_format(fmt)
        ret = lib.lyd_print_mem(buf, self._node, fmt, flags)
        if ret != 0:
            raise self.context.error('cannot print node')
        try:
            if fmt == lib.LYD_LYB:
                # binary format, do not convert to unicode
                return c2str(buf[0], decode=False)
            return c2str(buf[0], decode=True)
        finally:
            lib.free(buf[0])

    def print_file(self, fileobj, fmt,
                   with_siblings=False,
                   pretty=False,
                   include_implicit_defaults=False,
                   trim_default_values=False,
                   keep_empty_containers=False):
        flags = printer_flags(
            with_siblings=with_siblings, pretty=pretty,
            include_implicit_defaults=include_implicit_defaults,
            trim_default_values=trim_default_values,
            keep_empty_containers=keep_empty_containers)
        fmt = data_format(fmt)
        ret = lib.lyd_print_fd(fileobj.fileno(), self._node, fmt, flags)
        if ret != 0:
            raise self.context.error('cannot print node')

    def print_dict(self, strip_prefixes=True, absolute=True,
                   with_siblings=False, include_implicit_defaults=False,
                   trim_default_values=False, keep_empty_containers=False):
        """
        Convert a DNode object to a python dictionary.

        :arg DNode dnode:
            The data node to convert.
        :arg bool strip_prefixes:
            If True (the default), module prefixes are stripped from dictionary
            keys. If False, dictionary keys are in the form ``<module>:<name>``.
        :arg bool absolute:
            If True (the default), always return a dictionary containing the
            complete tree starting from the root.
        :arg bool with_siblings:
            If True, include the node's siblings.
        :arg bool include_implicit_defaults:
            Include implicit default nodes.
        :arg bool trim_default_values:
            Exclude nodes with the value equal to their default value.
        :arg bool keep_empty_containers:
            Preserve empty non-presence containers.
        """
        flags = printer_flags(
            include_implicit_defaults=include_implicit_defaults,
            trim_default_values=trim_default_values,
            keep_empty_containers=keep_empty_containers)

        def _to_dict(node, parent_dic):
            if not lib.lyd_node_should_print(node._node, flags):
                return
            if strip_prefixes:
                name = node.name()
            else:
                name = '%s:%s' % (node.module().name(), node.name())
            if isinstance(node, DList):
                list_element = {}
                for child in node:
                    _to_dict(child, list_element)
                parent_dic.setdefault(name, []).append(list_element)
            elif isinstance(node, (DContainer, DRpc)):
                container = {}
                for child in node:
                    _to_dict(child, container)
                parent_dic[name] = container
            elif isinstance(node, DLeafList):
                parent_dic.setdefault(name, []).append(node.value())
            elif isinstance(node, DLeaf):
                parent_dic[name] = node.value()

        dic = {}
        dnode = self
        if absolute:
            dnode = dnode.root()
        if with_siblings:
            for sib in dnode.siblings():
                _to_dict(sib, dic)
        else:
            _to_dict(dnode, dic)
        return dic

    def merge_data_dict(self, dic, rpc=False, rpcreply=False, strict=False,
                        data=False, config=False, no_yanglib=False):
        """
        Merge a python dictionary into this node. The returned value is the
        first created node.

        :arg dict dic:
            The python dictionary to convert.
        :arg bool rpc:
            Data represents RPC or action input parameters.
        :arg bool rpcreply:
            Data represents RPC or action output parameters.
        :arg bool strict:
            Instead of ignoring (with a warning message) data without schema
            definition, raise an error.
        :arg bool data:
            Complete datastore content with configuration as well as state
            data. To handle possibly missing (but by default required)
            ietf-yang-library data, use no_yanglib=True.
        :arg bool config:
            Complete datastore without state data.
        :arg bool no_yanglib:
            Ignore (possibly) missing ietf-yang-library data. Applicable only
            with data=True.
        """
        return dict_to_dnode(dic, self.module(), parent=self,
                             rpc=rpc, rpcreply=rpcreply, strict=strict,
                             data=data, config=config, no_yanglib=no_yanglib)

    def free(self, with_siblings=True):
        try:
            if with_siblings:
                lib.lyd_free_withsiblings(self._node)
            else:
                lib.lyd_free(self._node)
        finally:
            self._node = None

    def __repr__(self):
        cls = self.__class__
        return '<%s.%s: %s>' % (cls.__module__, cls.__name__, str(self))

    def __str__(self):
        return self.name()

    NODETYPE_CLASS = {}

    @classmethod
    def register(cls, *nodetypes):
        def _decorator(nodeclass):
            for t in nodetypes:
                cls.NODETYPE_CLASS[t] = nodeclass
            return nodeclass
        return _decorator

    @classmethod
    def new(cls, context, node_p):
        node_p = ffi.cast('struct lyd_node *', node_p)
        nodecls = cls.NODETYPE_CLASS.get(node_p.schema.nodetype, DNode)
        return nodecls(context, node_p)


#------------------------------------------------------------------------------
@DNode.register(SNode.CONTAINER)
class DContainer(DNode):

    def create_path(self, path, value=None, rpc_output=False):
        return self.context.create_data_path(
            path, parent=self, value=value, rpc_output=rpc_output)

    def children(self):
        child = self._node.child
        while child:
            yield DNode.new(self.context, child)
            child = child.next

    def __iter__(self):
        return self.children()


#------------------------------------------------------------------------------
@DNode.register(SNode.RPC)
class DRpc(DContainer):
    pass


#------------------------------------------------------------------------------
@DNode.register(SNode.LIST)
class DList(DContainer):
    pass


#------------------------------------------------------------------------------
@DNode.register(SNode.LEAF)
class DLeaf(DNode):

    def __init__(self, context, node_p):
        super().__init__(context, node_p)
        self._leaf = ffi.cast('struct lyd_node_leaf_list *', node_p)

    def value(self):
        if self._leaf.value_type == Type.EMPTY:
            return None
        if self._leaf.value_type in Type.NUM_TYPES:
            return int(c2str(self._leaf.value_str))
        if self._leaf.value_type in (
                Type.STRING, Type.BINARY, Type.ENUM, Type.IDENT, Type.BITS):
            return c2str(self._leaf.value_str)
        if self._leaf.value_type == Type.DEC64:
            return lib.lyd_dec64_to_double(self._node)
        if self._leaf.value_type == Type.LEAFREF:
            referenced = DNode.new(self.context, self._leaf.value.leafref)
            return referenced.value()
        if self._leaf.value_type == Type.BOOL:
            return bool(self._leaf.value.bln)
        return None


#------------------------------------------------------------------------------
@DNode.register(SNode.LEAFLIST)
class DLeafList(DLeaf):
    pass


#------------------------------------------------------------------------------
def dict_to_dnode(dic, module, parent=None, rpc=False, rpcreply=False,
                  strict=False, data=False, config=False, no_yanglib=False):
    """
    Convert a python dictionary to a DNode object given a YANG module object.
    The return value is the first created node. If parent is not set, a
    top-level node is returned.

    :arg dict dic:
        The python dictionary to convert.
    :arg Module module:
        The libyang Module object associated with the dictionary.
    :arg DNode parent:
        Optional parent to update. If not specified a new top-level DNode will
        be created.
    :arg bool rpc:
        Data represents RPC or action input parameters.
    :arg bool rpcreply:
        Data represents RPC or action output parameters.
    :arg bool strict:
        Instead of ignoring (with a warning message) data without schema
        definition, raise an error.
    :arg bool data:
        Complete datastore content with configuration as well as state
        data. To handle possibly missing (but by default required)
        ietf-yang-library data, use no_yanglib=True.
    :arg bool config:
        Complete datastore without state data.
    :arg bool no_yanglib:
        Ignore (possibly) missing ietf-yang-library data. Applicable only
        with data=True.
    """
    if not dic:
        return None

    if not isinstance(dic, dict):
        raise TypeError('dic argument must be a python dict')
    if not isinstance(module, Module):
        raise TypeError('module argument must be a Module object')
    if parent is not None and not isinstance(parent, DNode):
        raise TypeError('parent argument must be a DNode object or None')

    created = []

    def _create_leaf(_parent, module, name, value, in_rpc_output=False):
        if value is not None:
            if isinstance(value, bool):
                value = str(value).lower()
            elif not isinstance(value, str):
                value = str(value)
        if in_rpc_output:
            n = lib.lyd_new_output_leaf(
                _parent, module._module, str2c(name), str2c(value))
        else:
            n = lib.lyd_new_leaf(
                _parent, module._module, str2c(name), str2c(value))
        if not n:
            if _parent:
                parent_path = repr(DNode.new(module.context, _parent).path())
            else:
                parent_path = 'module %r' % module.name()
            raise module.context.error(
                'failed to create leaf %r as a child of %s', name, parent_path)
        created.append(n)

    def _create_container(_parent, module, name, in_rpc_output=False):
        if in_rpc_output:
            n = lib.lyd_new_output(_parent, module._module, str2c(name))
        else:
            n = lib.lyd_new(_parent, module._module, str2c(name))
        if not n:
            if _parent:
                parent_path = repr(DNode.new(module.context, _parent).path())
            else:
                parent_path = 'module %r' % module.name()
            raise module.context.error(
                'failed to create container/list/rpc %r as a child of %s',
                name, parent_path)
        created.append(n)
        return n

    schema_cache = {}

    def _find_schema(schema_parent, name, prefix):
        if isinstance(schema_parent, Module):
            cache_key = (schema_parent._module, name, prefix)
        else:
            cache_key = (schema_parent._node, name, prefix)
        snode, module = schema_cache.get(cache_key, (None, None))
        if snode is not None:
            return snode, module
        if isinstance(schema_parent, SRpc):
            if rpc:
                schema_parent = schema_parent.input()
            elif rpcreply:
                schema_parent = schema_parent.output()
            else:
                raise ValueError('rpc or rpcreply must be specified')
            if schema_parent is None:
                # there may not be any input or any output node in the rpc
                return None, None
        for s in schema_parent:
            if s.name() != name:
                continue
            mod = s.module()
            if prefix is not None and mod.name() != prefix:
                continue
            snode = s
            module = mod
            break
        schema_cache[cache_key] = (snode, module)
        return snode, module

    def _to_dnode(_dic, _schema, _parent=ffi.NULL, in_rpc_output=False):
        for key, value in _dic.items():
            if ':' in key:
                prefix, name = name.split(':')
            else:
                prefix, name = None, key

            s, module = _find_schema(_schema, name, prefix)
            if not s:
                if isinstance(_schema, Module):
                    path = _schema.name()
                elif isinstance(_schema, SNode):
                    path = _schema.schema_path()
                else:
                    path = str(_schema)
                if strict:
                    raise LibyangError('%s: unknown element %r' % (path, key))
                LOG.warning('%s: skipping unknown element %r', path, key)
                continue

            if isinstance(s, SLeaf):
                _create_leaf(_parent, module, name, value, in_rpc_output)

            elif isinstance(s, SLeafList):
                if not isinstance(value, (list, tuple)):
                    raise TypeError('%s: python value is not a list/tuple: %r'
                                    % (s.schema_path(), value))
                for v in value:
                    _create_leaf(_parent, module, name, v, in_rpc_output)

            elif isinstance(s, SRpc):
                n = _create_container(_parent, module, name, in_rpc_output)
                _to_dnode(value, s, n, rpcreply)

            elif isinstance(s, SContainer):
                n = _create_container(_parent, module, name, in_rpc_output)
                _to_dnode(value, s, n, in_rpc_output)

            elif isinstance(s, SList):
                if not isinstance(value, (list, tuple)):
                    raise TypeError('%s: python value is not a list/tuple: %r'
                                    % (s.schema_path(), value))
                for v in value:
                    if not isinstance(v, dict):
                        raise TypeError('%s: list element is not a dict: %r'
                                        % (_schema.schema_path(), v))
                    n = _create_container(_parent, module, name, in_rpc_output)
                    _to_dnode(v, s, n, in_rpc_output)

    result = None

    try:
        if parent is not None:
            _parent = parent._node
            _schema_parent = parent.schema()
        else:
            _parent = ffi.NULL
            _schema_parent = module
        _to_dnode(dic, _schema_parent, _parent,
                  in_rpc_output=rpcreply and isinstance(parent, DRpc))
        if created:
            result = DNode.new(module.context, created[0])
            result.validate(rpc=rpc, rpcreply=rpc,
                            data=data, config=config, no_yanglib=no_yanglib)
    except:
        for c in reversed(created):
            lib.lyd_free(c)
        raise

    return result
