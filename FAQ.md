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
   when running cmake:
```
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
```
   or add the libyang's location to the linker paths in `/etc/ld.so.conf.d` and
   then run `ldconfig` to rebuild the linker cache.


