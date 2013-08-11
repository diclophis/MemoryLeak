#!/bin/sh

#rm assets/.DS_Store
#rm -Rf platforms/android/build
#rm -Rf platforms/android/obj
#rm -Rf platforms/android/libs
#~/android-ndk-r7-crystax-5.beta2/ndk-build clean && 

cd platforms/android

CRYSTAX=~/android-ndk-r8-crystax-1

$CRYSTAX/ndk-build clean && $CRYSTAX/ndk-build && ant debug && ant debug uninstall && ant debug install
