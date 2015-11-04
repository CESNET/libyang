# libyang

Travis CI:
[![Build Status](https://secure.travis-ci.org/CESNET/libyang.png?branch=master)](http://travis-ci.org/CESNET/libyang)

Coverity Scan:
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5259/badge.svg)](https://scan.coverity.com/projects/5259)

libyang is YANG data modelling language parser and toolkit written (and
providing API) in C.


## Provided Features

* parsing (and validating) data models in YIN format
* reading instance data in XML format and validating
* reading instance data in JSON format and validating
* providing instance data in XML format
* providing instance data in JSON format


## Planned Features

* parsing (and validating) data models in YANG format


## Requirements

* cmake >= 2.6
* libpcre
* cmocka >= 1.0.0 (for tests only, see [Tests](#Tests))

## Building

```
$ mkdir build; cd build
$ cmake ..
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
$ CC=/usr/bin/clang cmake ..
```

### Tests

libyang includes several tests built with [cmocka](https://cmocka.org/). The tests
can be found in `tests` subdirectory and they are designed for checking library
functionality after code changes.

The tests are by default built in the `Debug` build mode by running
```
$ make
```

In case of the `Release` mode, the tests are not built by default (it requires
additional dependency), but it can be enabled via cmake option:
```
$ cmake -DENABLE_BUILD_TESTS=ON ..
```

Tests can be run by the make's `test` target:
```
$ make test
```


### CMake Notes

Note that, with CMake, if you want to change the compiler or its options after
you already ran CMake, you need to clear its cache first. For your convenience,
this action is available as `cclean` target of the Makefile:
```
$ make cclean
$ CC=gcc CFLAGS='-fstack-protector-strong -Wa,--noexecstack' cmake ..
```

