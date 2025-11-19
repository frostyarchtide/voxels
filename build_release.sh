#!/bin/bash

if [ ! -d "build/release/" ]; then
    meson setup build/release/ -Ddefault_library=static
fi

meson compile -C build/release/

if [ ! -d "bin/release/" ]; then
    mkdir -p bin/release/
fi

cp build/release/voxels bin/release/
./bin/release/voxels
