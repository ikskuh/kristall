@echo off

REM This script generates .ico file for Windows platforms from the vector icon
REM requires: imagemagick, pngquant

SET OPTS=-density 3000 -depth 24 -background none

MKDIR tmp_ico
ECHO Creating PNGs...
magick convert %OPTS% -resize 16x16   kristall.svg tmp_ico/16.png
magick convert %OPTS% -resize 20x20   kristall.svg tmp_ico/20.png
magick convert %OPTS% -resize 24x24   kristall.svg tmp_ico/24.png
magick convert %OPTS% -resize 40x40   kristall.svg tmp_ico/40.png
magick convert %OPTS% -resize 48x48   kristall.svg tmp_ico/48.png
magick convert %OPTS% -resize 64x64   kristall.svg tmp_ico/64.png
magick convert %OPTS% -resize 96x96   kristall.svg tmp_ico/96.png
magick convert %OPTS% -resize 256x256 kristall.svg tmp_ico/256.png
magick convert %OPTS% -resize 512x512 kristall.svg tmp_ico/512.png

ECHO Compressing PNGs...
CD tmp_ico
pngquant --skip-if-larger --ext .new.png --strip 256 *
DEl 16.png 20.png 24.png 40.png 48.png 64.png 96.png 256.png 512.png
CD ..


ECHO Packing into ico...
magick convert -define png:compression-level=9 tmp_ico/* kristall.ico
RMDIR /S /Q tmp_ico
