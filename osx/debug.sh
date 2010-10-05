#!/bin/sh

rm ../assets/models/.DS_Store
rm ../assets/textures/.DS_Store

make && ../build/osx/raptor_island $1
