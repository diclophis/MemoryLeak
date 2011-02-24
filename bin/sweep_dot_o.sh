#!/bin/sh

mkdir /tmp/swept_up_o
find . -name "*.o" -print0 | xargs -0 -I {} mv {} /tmp/swept_up_o
