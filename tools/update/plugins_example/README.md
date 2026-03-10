# Update Example

## Compilation

To try this example, *libyang* needs to be compiled with a `cmake` variable
```
-DYANGUPDATE_PLUGINS_DIR=libyang/tools/update/plugins_example
```
to compile the *mod* YANG plugin file `mod.c`. It implements the update of *mod* data first from revision 2025-01-01 to
2025-03-01 and then to 2025-06-01.

## Updating Data

Using the tool `yangupdate` the provided example data can be updated from their original revision 2025-01-01 to
2025-06-01. Use the following command to perform the update and then print into JSON format:
```
yangupdate -m libyang/tools/update/plugins_example/mod@2025-01-01.yang -s libyang/tools/update/plugins_example/ -d libyang/tools/update/plugins_example/data@2025-01-01.xml
```
