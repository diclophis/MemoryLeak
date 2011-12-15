#!/bin/sh

rm assets/.DS_Store
#rm -Rf platforms/android/build
#rm -Rf platforms/android/obj
#rm -Rf platforms/android/libs

cd platforms/android

~/android-ndk-r4-crystax/ndk-build clean && ~/android-ndk-r4-crystax/ndk-build && \
ant debug && ant uninstall && ant install
