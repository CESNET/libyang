## Requirements

* cmake >= 2.8.12
* swig
* node >= 0.10.x <7.0.x
* npm

## Install

```
$ cmake -DJAVASCRIPT_BINDING=ON ..
$ make
$ cd javascript
$ npm install --unsafe-perm
```

More details on swig's javascript bindings can be found at [SWIG](http://www.swig.org/Doc3.0/Javascript.html#Javascript_node_extensions).

## Usage

To include the node bindings simply use it with.

```
var yang = require("./index")
```
