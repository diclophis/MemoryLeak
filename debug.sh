#!/bin/sh

~/android-ndk-r4-crystax/ndk-build && \
ant debug && ant uninstall && ant install

#~/android-ndk-r4-crystax/ndk-build
#ant debug && ant uninstall && ant install
