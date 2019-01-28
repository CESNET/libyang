Debian packaging for libyang
============================

Compatibility notes
-------------------

This package won't build on older Debian (jessie, stretch) due to missing
swig 3.0.12 and debhelper 11.  However, the resulting binary packages can
be installed on jessie and stretch without issues - except for the python
packages.  I.e. the following packages:

- libyang0.16
- libyang-dev
- libyang-cpp0.16
- libyang-cpp-dev
- yang-tools

will work on jessie and stretch.  The following:

- python3-yang

will NOT work on jessie and stretch (because they have older python
versions.)

There are not plans to make this package "backwards compatible" in some
way.  You can either build on buster, or install swig 3.0.12 and debhelper
11.

Where to file issues
--------------------

Please file issues on the Debian BTS as usual.  You could also open issues
on github, but if it's something about the Debian packaging it's better to
stick with the proper Debian ways.  The Debian BTS is where other people
involved with Debian go look for bugs regarding a package, so that's where
they should be.

Building straight off git
-------------------------

Just the normal:

```
git clone https://github.com/CESNET/libyang -b debian
cd libyang
dpkg-buildpackage
```

Building a Debian .dsc
----------------------

Again, pretty much the normal:

```
git clone https://github.com/CESNET/libyang -b debian
wget -Olibyang_0.16.105.orig.tar.gz https://github.com/CESNET/libyang/archive/v0.16-r3.tar.gz
cd libyang
dpkg-source -b .
```

(Note the diverging release numbering though.)


Maintainer Notes
================

* the project version number is actually the SO ABI version.  The release
  point numbers (0.16-r3) isn't used for Debian.

* it's intentional that the SONAME is libyang.so.0.16 and not libyang.so.0.
  ABI compatibility is indicated by the first two numbers being equal;
  the third number is incremented for compatible changes.  cf.
  CESNET/libyang#656

* the watch file doesn't work yet but the libyang people agreed to make
  future release tags the same as the internal version number.  At that point
  the watch file will work.

* python3-yang-dbg follows the recommendations from
  https://wiki.debian.org/Python/LibraryStyleGuide#Building_python_-dbg_packages
