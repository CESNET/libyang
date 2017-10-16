__author__ = "Mislav Novakovic <mislav.novakovic@sartura.hr>"
__copyright__ = "Copyright 2017, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

import libyang as ly

try:
    ctx = ly.Context("/etc/sysrepo2/yang")
except Exception as e:
    print(e)
    err = ly.Error()
    print("err: %d" % err.err())
    print("vecode: %d" % err.vecode())
    print("errmsg: "+err.errmsg())
    print("errpath:"+err.errpath())
    print("errapptag:"+err.errapptag())

try:
    ctx = ly.Context("/etc/sysrepo/yang")
except Exception as e:
    print(e)

folders = ctx.get_searchdirs()
for folder in folders:
    print(folder)
print("\n")

module = ctx.get_module("ietf-interfaces")
if module is not None:
    print(module.name())
else:
    module = ctx.load_module("ietf-interfaces")
    if module is not None:
        print(module.name())

modules = ctx.get_module_iter()
for module in modules:
    print("module: " + module.name() + " prefix: " + module.prefix() + " type: " + str(module.type()))
