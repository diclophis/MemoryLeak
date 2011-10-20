#!/bin/sh

touch assets/models/.DS_Store && rm assets/models/.DS_Store
touch assets/textures/.DS_Store && rm assets/textures/.DS_Store

cd platforms/osx_glut/

#make clean
make && cd ../../ && ./platforms/osx_glut/build/raptor_island $1
