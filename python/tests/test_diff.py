# Copyright (c) 2019 Robin Jarry
# SPDX-License-Identifier: BSD-3-Clause

import os
import unittest

from libyang import Context
from libyang.diff import BaseTypeAdded
from libyang.diff import BaseTypeRemoved
from libyang.diff import NodeTypeAdded
from libyang.diff import NodeTypeRemoved
from libyang.diff import SNodeAdded
from libyang.diff import SNodeRemoved
from libyang.diff import StatusAdded
from libyang.diff import StatusRemoved
from libyang.diff import schema_diff


OLD_YANG_DIR = os.path.join(os.path.dirname(__file__), 'yang-old')
NEW_YANG_DIR = os.path.join(os.path.dirname(__file__), 'yang')


#------------------------------------------------------------------------------
class DiffTest(unittest.TestCase):

    expected_diffs = frozenset((
        (BaseTypeAdded, '/yolo-system:conf/yolo-system:speed'),
        (BaseTypeAdded, '/yolo-system:state/yolo-system:speed'),
        (BaseTypeRemoved, '/yolo-system:conf/yolo-system:speed'),
        (BaseTypeRemoved, '/yolo-system:state/yolo-system:speed'),
        (NodeTypeAdded, '/yolo-system:conf/yolo-system:number'),
        (NodeTypeAdded, '/yolo-system:state/yolo-system:number'),
        (NodeTypeRemoved, '/yolo-system:conf/yolo-system:number'),
        (NodeTypeRemoved, '/yolo-system:state/yolo-system:number'),
        (SNodeAdded, '/yolo-system:conf/yolo-system:url/yolo-system:enabled'),
        (SNodeAdded, '/yolo-system:state/yolo-system:url/yolo-system:enabled'),
        (StatusAdded, '/yolo-system:conf/yolo-system:deprecated-leaf'),
        (StatusAdded, '/yolo-system:conf/yolo-system:obsolete-leaf'),
        (StatusAdded, '/yolo-system:state/yolo-system:deprecated-leaf'),
        (StatusAdded, '/yolo-system:state/yolo-system:obsolete-leaf'),
        (StatusRemoved, '/yolo-system:conf/yolo-system:deprecated-leaf'),
        (StatusRemoved, '/yolo-system:conf/yolo-system:obsolete-leaf'),
        (StatusRemoved, '/yolo-system:state/yolo-system:deprecated-leaf'),
        (StatusRemoved, '/yolo-system:state/yolo-system:obsolete-leaf'),
    ))

    def test_diff(self):
        with Context(OLD_YANG_DIR) as ctx_old, Context(NEW_YANG_DIR) as ctx_new:
            mod = ctx_old.load_module('yolo-system')
            mod.feature_enable_all()
            mod = ctx_new.load_module('yolo-system')
            mod.feature_enable_all()
            diffs = []
            for d in schema_diff(ctx_old, ctx_new):
                if isinstance(d, (SNodeAdded, SNodeRemoved)):
                    diffs.append((d.__class__, d.node.schema_path()))
                else:
                    diffs.append((d.__class__, d.new.schema_path()))
            self.assertEqual(frozenset(diffs), self.expected_diffs)
