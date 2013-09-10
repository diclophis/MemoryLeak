#!/bin/sh

#rm assets/.DS_Store
#rm assets/models/.DS_Store
#rm assets/textures/.DS_Store

#cd platforms/android

#~/android-ndk-r4-crystax/ndk-build clean && ~/android-ndk-r4-crystax/ndk-build && \
#ant debug && ant uninstall && ant install

#~/android-ndk-r4-crystax/ndk-build
#ant debug && ant uninstall && ant install

cd platforms/android

CRYSTAX=~/android-ndk-r8-crystax-1

#$CRYSTAX/ndk-build clean && 
$CRYSTAX/ndk-build && ant debug && ant debug uninstall && ant debug install
