#!/usr/bin/env ruby

require 'rubygems'
require 'rmagick'

img = Magick::Image::read($stdin).first
$stdout.write(img.export_pixels_to_str(0,0,256,256,'I', Magick::CharPixel))

#.split(//).collect { |p| p.unpack("C") 
