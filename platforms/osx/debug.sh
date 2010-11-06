#!/bin/sh

touch ../assets/models/.DS_Store && rm ../assets/models/.DS_Store
touch ../assets/textures/.DS_Store && rm ../assets/textures/.DS_Store

make && echo "done" && ../build/osx/raptor_island $1
