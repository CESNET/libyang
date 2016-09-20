# libyang

[![BSD license](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://secure.travis-ci.org/CESNET/libyang.png?branch=master)](http://travis-ci.org/CESNET/libyang/branches)
[![codecov.io](https://codecov.io/github/CESNET/libyang/coverage.svg?branch=master)](https://codecov.io/github/CESNET/libyang?branch=master)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5259/badge.svg)](https://scan.coverity.com/projects/5259)
[![Ohloh Project Status](https://www.openhub.net/p/libyang/widgets/project_thin_badge.gif)](https://www.openhub.net/p/libyang)

libyang is YANG data modelling language parser and toolkit written (and
providing API) in C. The library is used e.g. in [libnetconf2](https://github.com/CESNET/libnetconf2),
[Netopeer2](https://github.com/CESNET/Netopeer2) or [sysrepo](https://github.com/sysrepo/sysrepo) projects.

## Provided Features

* Parsing (and validating) schemas in YANG format.
* Parsing (and validating) schemas in YIN format.
* Parsing, validating and printing instance data in XML format.
* Parsing, validating and printing instance data in JSON format
  ([RFC 7951](https://tools.ietf.org/html/rfc7951)).
* Manipulation with the instance data.
* Support for default values in the instance data ([RFC 6243](https://tools.ietf.org/html/rfc6243)).

Current implementation covers YANG 1.0 ([RFC 6020](https://tools.ietf.org/html/rfc6020))
as well as YANG 1.1 ([RFC 7950](https://tools.ietf.org/html/rfc7950)).

## Planned Features

* YANG Metadata ([RFC 7952](https://tools.ietf.org/html/rfc7952)).
* [yanglint](#yanglint) as a full-fledged YANG tool.

## Requirements

* cmake >= 2.8.9
* libpcre
* cmocka >= 1.0.0 (for tests only, see [Tests](#Tests))

## Building

```
$ mkdir build; cd build
$ cmake ..
$ make 
# make install
```

### Documentation

The library documentation can be generated directly from the source codes using
Doxygen tool:
```
$ make doc
$ google-chrome ../doc/html/index.html
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

## Usage

All libyang functions are available via the main header:
```
#include <libyang/libyang.h>
```

To compile your program with libyang, it is necessary to link it with libyang using the
following linker parameters:
```
-lyang
```

Note, that it may be necessary to call `ldconfig(8)` after library installation and if the
library was installed into a non-standard path, the path to libyang must be specified to the
linker. To help with setting all the compiler's options, there is `libyang.pc` file for
`pkg-config(1)` available in the source tree. The file is installed with the library.

If you are using `cmake` in you project, it is also possible to use the provided
`FindLibYANG.cmake` file to detect presence of the libyang library in the system.


## Tests

libyang includes several tests built with [cmocka](https://cmocka.org/). The tests
can be found in `tests` subdirectory and they are designed for checking library
functionality after code changes.

The tests are by default built in the `Debug` build mode by running
```
$ make
```

In case of the `Release` mode, the tests are not built by default (it requires
additional dependency), but they can be enabled via cmake option:
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

## Bindings

We provide bindings for high-level languages using [SWIG](http://www.swig.org/)
generator. The bindings are optional and to enable building of the specific
binding, the appropriate cmake option must be enabled, for example:
```
$ cmake -DJAVASCRIPT_BINDING=ON ..
```

More information about the specific binding can be found in their README files.

Currently supported bindings are:

* JavaScript
 * cmake option: `JAVASCRIPT_BINDING`
 * [README](./swig/javascript/README.md)

## yanglint

libyang source codes include a simple example tool to demanstrate how an application
can work with the libyang library. The tool is called `yanglint`. It is not being
installed together with the library itself, but it is built together with the library
binary and you can find it in [`tools/lint`](./tools/lint) directory.

There is also [README](./tools/lint/examples/README.md) describing some examples of
using `yanglint`.


