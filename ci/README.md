# Build Instructions for AppImage

This folder contains a `Dockerfile` to create a AppImage for Kristall.

Requirements:
- docker
- docker-compose

Build process:
```sh
docker build --tag kristall-ci:1.0 .
docker-compose up
```

This will create two files in the output folder:
- `kristall`
  The raw binary output
- `Kristall-nightly-x86_64.AppImage`
  The *AppImage* build of Kristall, should run on any 
  
Result was tested on:
- Gentoo / Kernel 5.4.48
- Arch Linux / Kernel 5.7.6