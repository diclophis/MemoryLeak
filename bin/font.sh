#!/bin/sh

convert -size 320x100 xc:lightblue -font Menlo-Regular -pointsize 72 \
-fill black -draw "text 28,68 'Anthony'" \
-fill white -draw "text 25,65 'Anthony'" \
font_shadow.jpg
