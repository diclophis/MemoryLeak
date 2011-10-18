#!/bin/sh

cd platforms/linux_glut
#rm -R ./build
make && ./build/raptor_island
