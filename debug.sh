#!/bin/sh

rm assets/.DS_Store
rm assets/models/.DS_Store
rm assets/textures/.DS_Store

~/android-ndk-r4-crystax/ndk-build && \
ant debug && ant uninstall && ant install

#~/android-ndk-r4-crystax/ndk-build
#ant debug && ant uninstall && ant install
