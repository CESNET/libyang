#!/bin/sh

set -e

src_dir="$1"
build_dir="$2"
install_dir="$3"

mkdir -p "$build_dir"
cd "$build_dir"

cmake -DCMAKE_BUILD_TYPE=release \
    -DENABLE_BUILD_TESTS=OFF \
    -DENABLE_VALGRIND_TESTS=OFF \
    -DENABLE_CALGRIND_TESTS=OFF \
    -DENABLE_BUILD_FUZZ_TARGETS=OFF \
    -DCMAKE_INSTALL_PREFIX=$install_dir \
    -DCMAKE_INSTALL_LIBDIR=$install_dir/_lib \
    -DCMAKE_INSTALL_INCLUDEDIR=$install_dir/_include \
    -DPLUGINS_DIR=$install_dir/_lib \
    -DGEN_LANGUAGE_BINDINGS=0 \
    "$src_dir"

make -s -j$(nproc)
make -s install
rm -rf -- $install_dir/bin $install_dir/share $install_dir/_lib/pkgconfig
