#!/bin/sh

rm assets/.DS_Store
rm assets/models/.DS_Store
rm assets/textures/.DS_Store

cd platforms/android

~/android-ndk-r4-crystax/ndk-build clean && ~/android-ndk-r4-crystax/ndk-build

ant clean && ant debug && ant uninstall && ant install
