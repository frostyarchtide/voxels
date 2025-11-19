#!/bin/bash

if [ ! -d "build/debug/" ]; then
    meson setup build/debug/ -Ddefault_library=static
fi

meson compile -C build/debug/

if [ ! -d "bin/debug/" ]; then
    mkdir -p bin/debug/
fi

cp build/debug/voxels bin/debug/
./bin/debug/voxels
