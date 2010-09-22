#!/bin/sh

make clean && make && valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --track-origins=yes ../build/osx/raptor_island
