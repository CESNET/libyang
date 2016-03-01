## Requirements

* swig
* node
* node-gyp

## Istall

```
$ cmake -DJAVASCRIPT_BINDING=ON ..
$ make
$ make npmpkg
$ cd javascript
$ node-gyp configure build
```

More details on swig's javascript bindings can be found at [SWIG](http://www.swig.org/Doc3.0/Javascript.html#Javascript_node_extensions).

## Usage

To include the node bindings simply use it with.

```
yang = require("<path to build>/javascript/build/Release/yang")
```

