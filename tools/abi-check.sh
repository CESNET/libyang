#!/bin/sh -e
# Dump the current ABI
abi-dumper ./build/libyang.so -o ./build/libyang.dump -lver "$(PKG_CONFIG_PATH=./build pkg-config --modversion libyang)"
# Compare the current ABI with previous ABI
abi-compliance-checker -l libyang.so -old ./libyang.dump -new ./build/libyang.dump -s
