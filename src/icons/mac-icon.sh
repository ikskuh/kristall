#!/bin/sh

mkdir AppIcon.iconset
rsvg-convert kristall.svg -h 1024 > AppIcon.iconset/icon_512x512@2.png
rsvg-convert kristall.svg -h 512 > AppIcon.iconset/icon_512x512.png
rsvg-convert kristall.svg -h 512 > AppIcon.iconset/icon_256x256@2.png
rsvg-convert kristall.svg -h 256 > AppIcon.iconset/icon_256x256.png
rsvg-convert kristall.svg -h 256 > AppIcon.iconset/icon_128x128@2.png
rsvg-convert kristall.svg -h 128 > AppIcon.iconset/icon_128x128.png
rsvg-convert kristall.svg -h 64 > AppIcon.iconset/icon_32x32@2.png
rsvg-convert kristall.svg -h 32 > AppIcon.iconset/icon_32x32.png
rsvg-convert kristall.svg -h 32 > AppIcon.iconset/icon_16x16@2.png
rsvg-convert kristall.svg -h 16 > AppIcon.iconset/icon_16x16.png
iconutil -c icns AppIcon.iconset
rm -rf AppIcon.iconset
