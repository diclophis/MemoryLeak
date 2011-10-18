#!/bin/sh

cd platforms/linux_glut
rm -R ./build
make -j && gdb ./build/raptor_island
