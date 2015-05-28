# libyang

libyang is YANG data modelling language parser and toolkit written (and
providing API) in C.


## Planned Features

* parsing (and validating) data models in YANG format
* parsing (and validating) data models in YIN format
* reading instance data in XML format and validating
* reading instance data in JSON format and validating
* storing instance data in XML format
* storing instance data in JSON format


## Requirements

* cmake >= 2.6


## Building

```
$ cmake .
$ make
# make install
```

### Build Modes

There are two build modes:
* Release.
  This generates library for the production use without any debug information.
* Debug.
  This generates library with the debug information and disables optimization
  of the code.

The `Debug` mode is currently used as the default one. to switch to the
`Release` mode, enter at the command line:
```
$ cmake -D CMAKE_BUILD_TYPE:String="Release" .
```

### Changing Compiler

Set CC variable:

```
$ CC=/usr/bin/clang cmake .
```

### CMake Notes

Note that, with CMake, if you want to change the compiler or its options after
you already ran CMake, you need to clear its cache first. For your convenience,
this action is available as `cclean` target of the Makefile:
```
$ make cclean
$ CC=gcc CFLAGS='-fstack-protector-strong -Wa,--noexecstack' cmake .
```

