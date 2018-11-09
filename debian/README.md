Debian packaging for libyang
============================

This repository contains Debian package files for
[libyang](https://github.com/CESNET/libyang).

Building a release with Debian patches
--------------------------------------

```
git clone https://github.com/opensourcerouting/libyang-debian.git
cd libyang-debian
dpkg-buildpackage -uc -us
```

Building a proper Debian source and package
-------------------------------------------

```
git clone https://github.com/opensourcerouting/libyang-debian.git
dpkg-source -b libyang-debian
dpkg-source -x *.dsc
cd libyang-0*
debuild
```

You can also use `cowbuilder` or `reprotest` on the dsc file directly.


Maintainer Notes
================

* the project version number is actually the SO ABI version.

* it's intentional that the SONAME is libyang.so.0.16 and not libyang.so.0.
  ABI compatibility is indicated by the first two numbers being equal;
  the third number is incremented for compatible changes.  cf.
  CESNET/libyang#656

* the watch file doesn't work yet but the libyang people agreed to make
  future release tags the same as the internal version number.  At that point
  the watch file will work.

* python3-yang-dbg follows the recommendations from
  https://wiki.debian.org/Python/LibraryStyleGuide#Building_python_-dbg_packages
