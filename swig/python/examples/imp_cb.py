__author__ = "Mislav Novakovic <mislav.novakovic@sartura.hr>"
__copyright__ = "Copyright 2017, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

import yang as ly

def module_imp_clb(mod_name, mod_rev, submod_name, sub_rev, user_data):
    print("module_imp_cb called")
    print("mod_name: ", mod_name, " mod_rev: ", mod_rev, " submod_name: ", submod_name, " sub_rev: ", sub_rev)
    return (ly.LYS_IN_YIN, "")


ctx = None
try:
    ctx = ly.Context("/etc/sysrepo/yang")
except Exception as e:
    print(e)

data = "user data"

ctx.set_module_imp_clb(module_imp_clb, data)

ctx.set_searchdir("/tmp/yang")
module = ctx.load_module("data-imp-per-B")
