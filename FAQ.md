# Frequently Asked Questions

__Q: error while loading shared libraries__

__A:__ libyang is installed into the directory detected by CMake's GNUInstallDirs
   function. However, when it is connected with the installation prefix, the
   target directory is not necessary the path used by the system linker. Check
   the linker's paths in `/etc/ld.so.conf.d/`. If the path where libyang is 
   installed is already present, just make `ldconfig` to rebuild its cache:
```
# ldconfig
```
   If the path is not present, you can change the libyang installation prefix
   when running cmake, so the complete compilation and installation sequence is:
```
$ mkdir build; cd build
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
$ make 
# make install
```
   or add the libyang's location to the linker paths in `/etc/ld.so.conf.d` and
   then run `ldconfig` to rebuild the linker cache.

__Q: yanglint(1) does not start and, but prints the following error messages:__
```
./yanglint
libyang[0]: Invalid keyword "type" as a child to "annotation". (path: /)
libyang[0]: Module "yang" parsing failed.
Failed to create context.
```

__A:__ To handle complex YANG extensions, libyang (and therefore yanglint(1))
   needs plugins. By default, the plugins are installed into the system path
   (next to the libyang library into the separate `libyang` subdirectory). If
   libyang was not installed, yanglint cannot find these plugins and it fails.
   If you do not want to install libyang, it is possible to specify path to the
   plugins via environment variable. The plugins can be found in the libyang
   build directory in `src/extensions/` subdirectory. So running yanglint(1)
   then can be made this way:
```
$ LIBYANG_EXTENSIONS_PLUGINS_DIR=`pwd`/src/extensions ./yanglint
```

