#!/bin/sh

cd platforms/ios
xcodebuild clean -alltargets -configuration "Debug" -sdk "iphoneos6.1"
xcodebuild install -alltargets -configuration "Debug" -sdk "iphoneos6.1"
