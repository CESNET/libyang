# YANG Update

This tool, developed together with [RACOM](https://www.racom.eu), provides a framework for updating configuration YANG data
in one revision to a newer revision of the corresponding YANG module. This is a necessary task when there is a deployed
device using YANG configuration that needs to be updated including its YANG module.

The user needs to define exact rules for transforming every YANG node from one revision to another. These rules are then
applied in succession to get full configuration data tree for the newest YANG module revision.

## Plugins

The transformation rules are specified in the form of plugins. They are source files in a specific directory that needs to
be set during compilation via a `cmake` variable `YANGUPDATE_PLUGINS_DIR`. In these files there are structures with
all the update rules that will be supported. All the details and an example template plugin source file can be found in
`plugin_template.c`.

## Usage

From all the plugin files a header file is generated that is included in the main source file. These are then compiled
into a static library `libyang_update`, which can be linked into the device image, for example, for it to support
updating its configuration YANG data. The file `yang_update.h` is the main library header with all the definitions and
the main update function. Also, a `yangupdate` binary tool is created that adds a simple interface for this library so
that it can be used directly from a CLI.

## Example

In `plugins_example` directory there are all the files required for trying out updating some YANG data with another
README with more details.
