#!/bin/sh

touch assets/models/.DS_Store && rm assets/models/.DS_Store
touch assets/textures/.DS_Store && rm assets/textures/.DS_Store

cd platforms/osx_glut/

#make clean
make && cd ../../

trap "kill 0" SIGINT SIGTERM EXIT

./platforms/osx_glut/build/raptor_island $1 &
./platforms/osx_glut/build/raptor_island $1 &

sleep 9999
