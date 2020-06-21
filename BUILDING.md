# Building

Here you can find build instructions for Kristall.

## Requirements

- Latest Qt5 version with `widgets` and `network` modules

## Build

### *nix

There's a small `Makefile` provided that does all necessary steps and creates a build directory, then copies the build artifact from the build directory. Just do `make` in the root directory, it should work.

Or you can use the usual Qt5 build process:

```sh
mkdir build
cd build
qmake ../src/kristall.pro
make
```

#### Notes for OpenBSD
- It seems like Qt wants `libzstd.so.3.1` instead of `libzstd.so.3.2`. Just symlink that file into the build directory
- Use `make` and not `gmake` to build the project.

#### Notes for Ubuntu
Supported Versions: 18.04, 20.04
Unsupported Versions: 16.04

- Required packages
  - `qt5-default`
  - `qt5-qmake`
  - `qtmultimedia5-dev` 
  - `libqt5svg5-dev`
  - `libssl-dev`
  - `make`
  - `g++`

These notes are probably also correct for Debian (someone please verify)

#### Notes for Manjaro/Arch
- Required packages
  - `qt5`
  - `qt5-multimedia`
  - `openssl` or `libressl`

#### Notes on void linux
- set env variable `QT_SELECT=5`

#### Windows

Compile OpenSSL with the following steps:  
- Install [Perl](https://www.perl.org/get.html) (either ActiveState or Strawberry)
- Install [NASM](https://www.nasm.us/)
- Add both to your PATH
- Clone [OpenSSL](https://github.com/openssl/openssl)
- Open a Visual Studio Developer Command Prompt (or a command prompt that has ran vcvarsall.bat). You will need Administrator privileges for the install step
- In the OpenSSL root directory, run `perl Configure VC-WIN32` if compiling for 32 bit, `perl Configure VC-WIN64A` for 64 bit
- Run `nmake`
- Run `nmake install` to install OpenSSL in `C:\Program Files\OpenSSL`

Use QtCreator to build `./src/kristall.pro` with default settings.

#### MacOS X

Use the `Makefile` to build `build/kristall` instead of the default target. There is currently no auto-recognition of MacOS (where qmake will output a `kristall.app` file), so you have to be a bit more manual.

#### Haiku

1. Install the following packages with `pkgman`:
    - `qt5`
    - `qt5_devel`
    - `qt5_tools`
    - `libiconv_devel`
2. Use `make` to build th executable

## Manual Installation

### Unix / XDG

The provided desktop file can be installed into the local system
```sh
ln -s Kristall.desktop ~/.local/share/applications/kristall.desktop
```

### Haiku

Link the `kristall` executable to your applications menu:

```sh
cd ~/config/settings/deskbar/menu/Applications/
ln -s /path/to/kristall .
```