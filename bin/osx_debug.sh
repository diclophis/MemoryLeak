#!/bin/sh

touch assets/models/.DS_Store && rm assets/models/.DS_Store
touch assets/textures/.DS_Store && rm assets/textures/.DS_Store

cd platforms/osx/

make clean
make

cd ../../

./platforms/osx/build/raptor_island $1
