# libyang Repository Details

This document provides a summary of the libyang repository, including its purpose, structure, dependencies, and build/test procedures.

## Repository Purpose

Libyang is a C library for parsing, validating, and manipulating YANG data models. It serves as a toolkit for applications that need to work with YANG, providing functionalities such as:

*   Parsing and validating schemas in YANG and YIN formats.
*   Parsing, validating, and printing instance data in XML and JSON formats.
*   Manipulation of instance data.
*   Support for default values, YANG extensions, YANG Metadata, YANG Schema Mount, and YANG Structure.
*   It includes command-line tools like `yanglint` for validating YANG modules and `yangre` for testing regular expressions.

The library is used in projects like libnetconf2, Netopeer2, and sysrepo.

## Repository Structure

The repository is organized into several main directories:

*   **`src/`**: Contains the C source code for the libyang library.
*   **`tests/`**: Houses various tests:
    *   `utests/`: Unit tests (using CMocka).
    *   `fuzz/`: Fuzzing targets and related files.
    *   `perf/`: Performance measurement tools.
    *   `yanglint/`: Tests for the `yanglint` tool.
    *   `yangre/`: Tests for the `yangre` tool.
*   **`tools/`**: Contains source code for command-line tools:
    *   `lint/`: Source for `yanglint`. The `tools/lint/examples/README.md` provides usage examples.
    *   `re/`: Source for `yangre`.
*   **`doc/`**: Contains documentation files, including a "Transition Manual" for migrating from older versions.
*   **`models/`**: Contains standard YANG models (e.g., IETF models).
*   **`distro/`**: Files and scripts for creating binary packages (RPM, DEB).
*   **`CMakeModules/`**: Custom CMake scripts for the build process.
*   **`compat/`**: Compatibility code, possibly for different OS or C library versions.
*   **`.github/`**: GitHub-specific files, primarily for CI/CD workflows.

## Dependencies

### Unix Build Requirements (Required)

*   **C compiler** (e.g., GCC, Clang)
*   **cmake >= 2.8.12**
*   **libpcre2 >= 10.21** (including development package, e.g., `libpcre2-dev`)

### Unix Build Requirements (Optional)

*   **xxhash**: For faster hashing.
*   **doxygen**: For generating API documentation.
*   **cmocka >= 1.0.1**: Unit testing framework (for tests).
*   **valgrind**: For memory debugging and profiling (for enhanced testing).
*   **gcov**: Code coverage analysis tool.
*   **lcov**: Frontend for gcov.
*   **genhtml**: Tool to convert LCOV coverage data to HTML.

### Unix Runtime Requirements (Required)

*   **libpcre2 >= 10.21**

### Windows Build Requirements

*   **Visual Studio 17 (2022)**
*   **cmake >= 3.22.0**
*   **libpcre2**
*   **`pthreads-win32`**
*   **`dirent`**
*   **`dlfcn-win32`**
*   **`getopt-win32`**

(Windows version has limitations: no plugin support, `yanglint` is non-interactive only).

## Build Process (Unix-like systems)

The build process uses CMake:

1.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```
2.  **Run CMake to configure:**
    ```bash
    cmake ..
    ```
3.  **Compile:**
    ```bash
    make
    ```
4.  **Install (optional):**
    ```bash
    sudo make install
    ```

### CMake Customization Options:

*   **Compiler:** `CC=/usr/bin/clang cmake ..`
*   **Install Path:** `cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..` (default: `/usr/local`)
*   **Build Mode:**
    *   Debug (default)
    *   Release: `cmake -D CMAKE_BUILD_TYPE:String="Release" ..`
*   **Extensions Plugins Directory:** `cmake -DPLUGINS_DIR:PATH="/custom/path/" ..`
*   **Disable Latest Revisions Cache:** `cmake -DENABLE_LATEST_REVISIONS=OFF ..`

If CMake options are changed, clear the `build` directory and rerun `cmake`.

## Testing Process

*   Tests use the **CMocka** framework and are located in `tests/`.
*   Includes unit tests, regression tests (from fuzzing inputs).

### Building Tests:

*   **Debug Mode (default):** Built automatically with `make`.
*   **Release Mode:** Enable with `cmake -DENABLE_TESTS=ON ..` then `make`.
*   Requires CMocka headers to be available.

### Running Tests:

*   From the `build` directory:
    ```bash
    make test
    ```

### Performance Tests:

*   Enable: `cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PERF_TESTS=ON ..`
*   Run: `make && ctest -V -R ly_perf`

### Code Coverage:

*   Enable: `cmake -DENABLE_COVERAGE=ON ..`
*   Generate: `make && make coverage`

### Fuzzing:

*   Targets and instructions are in `tests/fuzz/`.
*   Supports LibFuzzer and AFL.
---
This file summarizes key information about the libyang library, its structure, how to build it, and how to run its tests.

## `src/` Directory Overview

This section provides a high-level overview of the key files and their functionalities within the `src/` directory, which contains the core source code of the libyang library.

**1. Core Functionality:**

*   **`libyang.h`**: The main public header file for libyang. It's the primary entry point for users, including other necessary headers and defining the core API.
*   **`context.c`, `context.h`**: Manage the libyang context (`ly_ctx`), a central structure holding loaded YANG modules, configurations, string dictionaries, and error information. Most operations require a context.
*   **`ly_common.c`, `ly_common.h`**: Contain common utility functions, macros, and definitions used throughout the library (e.g., memory management, error handling).
*   **`log.c`, `log.h`**: Implement the logging subsystem, allowing the library to output messages of varying severity.
*   **`dict.c`, `dict.h`**: Manage a string dictionary for interning, optimizing memory usage and string comparisons.
*   **`hash_table.c`, `hash_table.h`, `hash_table_internal.h`**: Provide a generic hash table implementation used for efficient storage and lookup.

**2. Schema Handling:**

This group is responsible for parsing, compiling, and validating YANG schema files.

*   **Parsers (`parser_yang.c`, `parser_yin.c`)**:
    *   `parser_yang.c`: Parses schemas in YANG format.
    *   `parser_yin.c`: Parses schemas in YIN (XML-based) format.
*   **Common Schema Parsing Logic (`parser_common.c`, `parser_schema.h`, `parser_internal.h`)**:
    *   `parser_schema.h`: Defines common structures and functions for schema parsing.
    *   `parser_common.c`, `parser_internal.h`: Contain shared utilities and internal structures for schema parsers.
*   **Schema Compilation (`schema_compile.c`, `schema_compile.h`, `resolve.c`, `schema_features.c`)**:
    *   `schema_compile.h`: Defines structures for the schema compilation process, including the compilation context and handling of unresolved items.
    *   `schema_compile.c`: Implements the core logic for compiling parsed schemas (resolving references, processing typedefs, groupings, etc.).
    *   `resolve.c`: Handles resolution of cross-references within and between modules.
    *   `schema_features.c`: Manages the evaluation and application of YANG features and if-feature statements.
*   **Schema Tree Representation (`tree_schema.c`, `tree_schema.h`, `tree_schema_internal.h`)**:
    *   `tree_schema.h`: Defines C structures for the compiled YANG schema in memory (modules, nodes like containers, leafs, lists, types, identities).
    *   `tree_schema.c`: Implements functions for working with the compiled schema tree.
*   **XPath Handling (`xpath.c`, `xpath.h`)**: Implements parsing and evaluation of XPath expressions used in YANG.
*   **Validation (`validation.c`, `validation.h`, `validate_value.c`)**:
    *   `validation.c`, `validation.h`: Contain logic for validating data trees against schemas and for schema self-validation.
    *   `validate_value.c`: Focuses on validating values of terminal nodes.

**3. Data Handling:**

This group deals with parsing, representing, printing, and comparing YANG instance data.

*   **Data Parsers (`parser_json.c`, `parser_xml.c`, `parser_lyb.c`)**:
    *   `parser_json.c`: Parses instance data from JSON format.
    *   `parser_xml.c`: Parses instance data from XML format.
    *   `parser_lyb.c`: Parses instance data from LYB (libyang binary) format.
*   **Common Data Parsing Logic (`parser_data.h`)**:
    *   `parser_data.h`: Defines common interfaces and structures for data parsing.
*   **Data Printers (`printer_json.c`, `printer_xml.c`, `printer_lyb.c`)**:
    *   `printer_json.c`: Serializes data trees into JSON format.
    *   `printer_xml.c`: Serializes data trees into XML format.
    *   `printer_lyb.c`: Serializes data trees into LYB format.
*   **Common Data Printing Logic (`printer_data.c`, `printer_data.h`, `printer_internal.h`)**:
    *   `printer_data.h`: Defines common structures and functions for printing data trees.
*   **Data Tree Representation (`tree_data.c`, `tree_data.h`, `tree_data_internal.h`)**:
    *   `tree_data.h`: Defines C structures for in-memory YANG instance data (generic nodes, inner nodes, terminal nodes, anydata/anyxml, metadata, values).
    *   `tree_data.c`: Implements functions for creating, modifying, and freeing data nodes.
*   **Data Tree Comparison (`diff.c`, `diff.h`)**: Implements functionality to compare two data trees and generate a diff.
*   **Input/Output Handling (`in.c`, `in.h`, `in_internal.h` & `out.c`, `out.h`, `out_internal.h`)**:
    *   `in.h`, `out.h`: Define abstractions for input sources and output destinations.
    *   `in.c`, `out.c`: Implement these I/O abstractions.
*   **LYB Format Specifics (`lyb.c`, `lyb.h`)**: Contains code specific to the LYB binary format.

**4. Plugins and Extensions:**

Libyang uses a plugin system for custom YANG types and extensions.

*   **Plugin System Core (`plugins.c`, `plugins.h`, `plugins_internal.h`)**:
    *   `plugins.h`: Defines the API for managing and interacting with plugins.
    *   `plugins.c`: Implements the core logic for plugin discovery and management.
*   **Extension Plugins (`plugins_exts/` directory, `plugins_exts.c`, `plugins_exts.h`)**:
    *   `plugins_exts/`: Contains internal plugins for specific YANG extensions (e.g., `metadata.c`, `nacm.c`, `schema_mount.c`).
    *   `plugins_exts.c`, `plugins_exts.h`: Common infrastructure for extension plugins.
*   **Type Plugins (`plugins_types/` directory, `plugins_types.c`, `plugins_types.h`)**:
    *   `plugins_types/`: Contains internal plugins for YANG data types (e.g., `binary.c`, `bits.c`, `integer.c`, `string.c`).
    *   `plugins_types.c`, `plugins_types.h`: Common infrastructure for type plugins.

**5. Utilities & Other:**

*   **Path Utilities (`path.c`, `path.h`)**: Provides functions for working with YANG data paths.
*   **Set Data Structure (`set.c`, `set.h`)**: Implements a set data structure for unique collections.
*   **XML Utilities (`xml.c`, `xml.h`, `xml_internal.h`)**: Utility functions for handling XML.
*   **JSON Utilities (`json.c`, `json.h`)**: Utility functions for handling JSON.
*   **Build-time Configuration (`ly_config.h.in` -> `ly_config.h`)**: Generates a header with build-time configuration (version, features).
