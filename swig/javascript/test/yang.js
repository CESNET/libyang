
var path = "@CMAKE_BINARY_DIR@/javascript"

var yang = require(path + "/build/Release/yang.node")

var ctx = yang.ly_ctx_new(path + "/test/files");
var module = yang.lys_parse_path(ctx, path + "/test/files/a.yin", yang.LYS_IN_YIN);
var node = yang.lyd_parse_path(ctx, path + "/test/files/a.xml", yang.LYD_XML, 0);

console.log(module.name);
console.log(node.schema.child.name)
