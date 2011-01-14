convert -resize 256x256\! -fuzz 5\% -transparent \#ffffff -alpha On private/shredder.gif assets/textures/7.png

montage -background transparent *.png -alpha On -transparent-color white -transparent white ring_01.png
