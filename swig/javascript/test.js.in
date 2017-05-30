var yang = require("./index")

var ctx = yang.ly_ctx_new("./files");
var module = yang.lys_parse_path(ctx, "./files/b.yang", yang.LYS_IN_YANG);

console.log(module.name);
