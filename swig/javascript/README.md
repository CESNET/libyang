## Requirements

* swig
* node
* npm

## Install

```
$ cmake -DJAVASCRIPT_BINDING=ON ..
$ make
$ cd javascript
$ npm install
```

More details on swig's javascript bindings can be found at [SWIG](http://www.swig.org/Doc3.0/Javascript.html#Javascript_node_extensions).

## Usage

To include the node bindings simply use it with.

```
yang = require("<path to build>/javascript/build/Release/yang")
```

