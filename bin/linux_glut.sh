#!/bin/sh

cd platforms/linux_glut
rm -R ./build
make -j && ./build/raptor_island
