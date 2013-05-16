#!/bin/sh

touch assets/models/.DS_Store && rm assets/models/.DS_Store
touch assets/textures/.DS_Store && rm assets/textures/.DS_Store

cd platforms/osx_glut/

#make clean
make && cd ../../

trap "kill 0" SIGINT SIGTERM EXIT

#./platforms/osx_glut/build/raptor_island 

./platforms/osx_glut/build/raptor_island $1 &
./platforms/osx_glut/build/raptor_island $1 &
sleep 5
osascript -e '
tell application id "com.apple.systemevents"
tell front window of (first application process where frontmost = true)
set position to {0, 0}
end tell
end tell'
sleep 9999
