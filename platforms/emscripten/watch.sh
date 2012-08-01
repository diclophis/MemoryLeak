#!/bin/sh

watch -n 0.1 "ls -lhtr /tmp/emscripten_temp && du -s /tmp/emscripten_temp && ls -lhtr build && du -s build && ls -lh /tmp/tmp*"
