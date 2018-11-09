Debian packaging for libyang
============================

This repository contains Debian package files for
[libyang](https://github.com/CESNET/libyang).

Building a release with Debian patches
--------------------------------------

```
git clone https://github.com/CESNET/libyang.git
git clone https://github.com/opensourcerouting/libyang-debian.git
cd libyang
git checkout -b debian-0.16.52 v0.16-r2
cp -r ../libyang-debian/debian .
dpkg-buildpackage -uc -us
```

Note the debian version cannot be symlinked in this case because
dpkg-buildpackage will check the source for changes and this fails with
symlinks.  Also, the git tag must match the release the Debian patches
were made for.

Building libyang HEAD without Debian patches
--------------------------------------------

```
git clone https://github.com/CESNET/libyang.git
git clone https://github.com/opensourcerouting/libyang-debian.git
cd libyang
ln -s ../libyang-debian/debian .
dpkg-buildpackage -uc -us -nc
```

In this case due to the `-nc` option, Debian patches are not applied.
You can manually apply some of them if neccessary.  Something like this
will probably work for newer libyang versions than the one the packaging
is for.

Building a proper Debian source and package
-------------------------------------------

```
wget -O libyang_0.16.52.orig.tar.gz 'https://github.com/CESNET/libyang/archive/v0.16-r2.tar.gz'
git clone https://github.com/opensourcerouting/libyang-debian.git
dpkg-source -b libyang-debian
dpkg-source -x *.dsc
cd libyang-0*
debuild
```

The `dpkg-source -b` step will print a lot of warnings about removed files,
you can ignore these.  You can also use `cowbuilder` or `reprotest` on the
dsc file directly.


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
