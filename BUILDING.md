# Building

Here you can find build instructions for Kristall.

## Requirements

- Qt 5.9 or higher with `widgets` and `network` modules

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

#### Notes on FreeBSD
- Kristall can't be used with LibreSSL, but requires OpenSSL (probably because of Qt)
- Required packages
  - `qt5`
  - `make`
  - `git` - optional, for cloning from CLI
 - Aditionally, the following must be exported for qmake: `export QMAKESPEC=freebsd-clang`
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

#### Notes for Fedora 32 (and probably other recent versions)
In the small `Makefile` mentioned above, comment out the line:

* `QMAKE_COMMAND := qmake`

And uncomment the line:
* `# QMAKE_COMMAND := /usr/bin/qmake-qt5`

This is because Fedora has a different naming scheme for the qmake executable. 

- Key required packages
    - `qt5-qtbase`
    - `qt5-qtbase-devel`
    - `qt5-qtmultimedia-devel`
    - `qt5-qtsvg-devel`
    - `ccache-3.7.7-1.fc32.x86_64`
    - `make`
    - `openssl-devel`
    - And the packages they depend on

#### Notes for Manjaro/Arch
- Required packages
  - `qt5-base`
  - `qt5-multimedia`
  - `qt5-svg`
  - `openssl` or `libressl`

#### Notes on void linux
- set env variable `QT_SELECT=5`

#### Notes on alpine linux
- Requires packages
  - qt5-qtbase-dev
  - qtchooser
  - qt5-qtsvg-dev
  - qt5-qtmultimedia-dev

#### Windows (MinGW)
Install Qt via the Qt installer. Install the following components:

**Required:**
- `Qt/Qt 5.15.0/MinGW 8.1.0 64-bit`
- `Qt/Developer and Designer Tools/MinGW 8.1.0 64 bit`
- `Qt/Developer and Designer Tools/OpenSSL 64-bit binaries`
**Optional:**
- `Qt/Developer and Designer Tools/Qt Creator 4.12.3 CDB Debugger Support`
- `Qt/Developer and Designer Tools/Debugging Tools for Windows`
- `Qt/Developer and Designer Tools/Qt Creator 4.12.3 CDB Debugger Support`
- `Qt/Developer and Designer Tools/OpenSSL source code`

If you didn't istall Qt to `C:\Qt`, you have to adjust the paths in `src/kristall.pro` for the `win32-g++` adjustments to the path you used.

Then open `src/kristall.pro` with Qt creator to compile the project. Alternativly you can use `ci\build-and-deploy.bat`, but note that this script tries to deploy the file to `random-projects.net`, so this will fail in the end.

**Troubleshouting:**
If you get an error message that `MSVCR100.dll` is missing, you may need to install the [Microsoft C++ Runtime](https://www.microsoft.com/en-us/download/details.aspx?id=14632).

**Troubleshouting:**
*QtCreator says the exe crashed when starting.* This happens when you are missing the OpenSSL dlls. Copy them from `C:\Qt\Tools\OpenSSL\Win_x64\bin\libcrypto-1_1-x64.dll` and `C:\Qt\Tools\OpenSSL\Win_x64\bin\libssl-1_1-x64.dll` to the debug/release folder of your build. Then the exe should be starting.

#### Windows MSVC (not up to date)

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

#### macOS

1. Run `brew install qt openssl` to install the `qt` and `openssl` packages with [Homebrew](https://brew.sh/).
2. Run `make build/kristall` to build `kristall.app` in the `build` directory.

#### Haiku

1. Install the following packages with `pkgman`:
    - `qt5`
    - `qt5_devel`
    - `qt5_tools`
    - `libiconv_devel`
    - `openssl_devel` (should be preinstalled)
2. Use `make` to build th executable

## Manual Installation

### Unix / XDG

The provided desktop file and additional mime types (along with the text/gemini) can be installed into the local system
```sh
cp Kristall.desktop ~/.local/share/applications/kristall.desktop
cp kristall-mime-info.xml ~/.local/share/mime/packages/kristall-mime-info.xml
update-desktop-database ~/.local/share/applications
update-mime-database ~/.local/share/mime
```

### macOS

Copy `kristall.app` from the `build` directory to `/Applications`.

### Haiku

Link the `kristall` executable to your applications menu:

```sh
cd ~/config/settings/deskbar/menu/Applications/
ln -s /path/to/kristall .
```
