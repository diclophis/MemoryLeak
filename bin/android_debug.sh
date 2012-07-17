#!/bin/sh

#rm assets/.DS_Store
#rm -Rf platforms/android/build
#rm -Rf platforms/android/obj
#rm -Rf platforms/android/libs

cd platforms/android

#~/android-ndk-r4-crystax/ndk-build clean && ~/android-ndk-r4-crystax/ndk-build && \
#~/android-ndk-r7-crystax-5.beta2/ndk-build clean && 

~/android-ndk-r7-crystax-5.beta2/ndk-build

ant debug && ant debug uninstall && ant debug install
