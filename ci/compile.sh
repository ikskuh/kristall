#!/bin/bash

set -eo pipefail

rm -rf /kristall

git clone https://github.com/MasterQ32/kristall /kristall

cd /kristall

make -j$(nproc)

mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/doc/libc6/

make install DESTDIR=AppDir PREFIX=/usr/

export VERSION=nightly

# Create copyright notice that linuxdeployqt wants
cp /usr/share/doc/libc6/copyright AppDir/usr/share/doc/libc6/copyright

echo "Running linuxdeployqt"
linuxdeployqt AppDir/usr/share/applications/Kristall.desktop -unsupported-allow-new-glibc -appimage

echo "Running linuxdeploy"
linuxdeploy --appdir AppDir --library=/usr/lib/x86_64-linux-gnu/libssl.so.1.1 --output=appimage

cp kristall /artifacts

cp Kristall-nightly-$(uname -m).AppImage /artifacts/

