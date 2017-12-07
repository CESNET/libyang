__author__ = "Mislav Novakovic <mislav.novakovic@sartura.hr>"
__copyright__ = "Copyright 2017, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

import libyang as ly

try:
    ctx = ly.Context("/etc/sysrepo/yang")
except Exception as e:
    print(e)
    sys.exit()

module = ctx.load_module("turing-machine", None)
if module is None:
    print("module not loaded")
    sys.exit()

try:
    node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", ly.LYD_XML, ly.LYD_OPT_CONFIG)
except Exception as e:
    print(e)
    sys.exit()

node_set = node.find_path("/turing-machine:turing-machine/transition-function/delta[label='left summand']/*")
if node_set is None:
    print("could not find data for xpath")
    sys.exit()

for data_set in node_set.data():
    schema = data_set.schema()
    print("name: " + schema.name() + " type: " + str(schema.nodetype()) + " path: " + data_set.path())
