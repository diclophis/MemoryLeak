#!/bin/sh

cd platforms/emscripten
rm -R ./build
~/emscripten/emmake make
#if [ -f ./build/raptor_island.html]; then
test -f ./build/raptor_island.html && firefox ./build/raptor_island.html
#fi
