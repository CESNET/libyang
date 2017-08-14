## Requirements

* cmake >= 2.8.12
* swig
* node >= 0.10.x
* npm

## Install

```
$ cmake -DJAVASCRIPT_BINDING=ON ..
$ make
$ make install #necessary for plugin's
$ cd javascript
$ npm install --unsafe-perm
```

## node 7.0.x

The latest SWIG library does not generate correct node 7.0.x code. To correct that at the moment you need to do two things:

1) Use the latest SWIG master branch with the pull request [Add Node 7.x aka V8 5.2+ support](https://github.com/swig/swig/pull/968).
2) In the generated C++ source file libyang_javascript_wrap.cxx replace 'lys_ext_instance_complex()' with 'lys_ext_instance_complex'.

You can build the JavaScript bindings inside docker, for that you can use the [dockerfile](./Dockerfile), it is based on the [dockerfile](https://github.com/sysrepo/sysrepo/blob/master/deploy/docker/sysrepo-netopeer2/platforms/Dockerfile.arch.devel).

More details on SWIG's JavaScript bindings can be found at [SWIG](http://www.swig.org/Doc3.0/Javascript.html#Javascript_node_extensions).

## Usage

To include the node bindings simply use it with.

```
var yang = require("./index")
```
