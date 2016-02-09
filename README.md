# libyang

[![BSD license](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://secure.travis-ci.org/CESNET/libyang.png?branch=master)](http://travis-ci.org/CESNET/libyang)
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

### Useful CMake Options

#### Changing Compiler

Set `CC` variable:

```
$ CC=/usr/bin/clang cmake ..
```

#### Changing Install Path

To change the prefix where the library, headers and any other files are installed,
set `CMAKE_INSTALL_PREFIX` variable:
```
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
```

Default prefix is `/usr/local`.

#### Build Modes

There are two build modes:
* Release.
  This generates library for the production use without any debug information.
* Debug.
  This generates library with the debug information and disables optimization
  of the code.

The `Debug` mode is currently used as the default one. to switch to the
`Release` mode, enter at the command line:
```
$ cmake -D CMAKE_BUILD_TYPE:String="Release" ..
```
### CMake Notes

Note that, with CMake, if you want to change the compiler or its options after
you already ran CMake, you need to clear its cache first - the most simple way
to do it is to remove all content from the 'build' directory.

## Tests

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

Note that if the necessary [cmocka](https://cmocka.org/) headers are not present
in the system include paths, tests are not available despite the build mode or
cmake's options.

Tests can be run by the make's `test` target:
```
$ make test
```

## yanglint

libyang source codes include a simple example tool to demanstrate how an application
can work with the libyang library. The tool is called `yanglint`. It is not being
installed together with the library itself, but it is built together with the library
binary and you can find it in [`tools/lint`](./tools/lint) directory.

There is also [README](./tools/lint/examples/README.md) describing some examples of
using `yanglint`.


