# Kristall
A high-quality visual cross-platform gemini browser.

![Preview Image](https://mq32.de/public/336ac416892fd9064593631e7be9f7d8e266196b.png)

## Features
- Document rendering
  - `text/gemini`
  - `text/html` (reduced feature set)
  - `text/markdown`
  - `text/*`
  - `image/*`
  - `video/*`
  - `audio/*`
- Multi-protocol support
  - [Gemini](https://gemini.circumlunar.space/)
  - HTTP
  - HTTPS
  - [Finger](https://tools.ietf.org/html/rfc1288)
  - [Gopher](https://tools.ietf.org/html/rfc1436) (WIP)
- [Outline generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png) ([Video](https://mq32.de/public/kristall-02.mp4))
- Favourite Sites
- Navigation history
- Tabbed interface
- Survives [ConMans torture suite](gemini://gemini.conman.org/test/torture/)
- [Special link highlighting for different targets](https://mq32.de/public/92f3ec7a64833d01f1ed001d15c8db4158e5d3c2.png)
- Color Themes
  - Custom document color theme
  - [Automatic light/dark theme based on the host name](https://mq32.de/public/kristall-01.mp4)
  - Dark/Light UI theme
- Crossplatform supports
  - Linux
  - Windows
  - FreeBSD
  - NetBSD
  - OpenBSD
  - MacOS X
  - Haiku

## Screenshots

### Generates Outlines

![Outline Generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png)

### Fully Customizable Site Theme

![Site Theme](https://mq32.de/public/7123e22a58969448c27b24df8510f4d56921bf23.png)

## Build/Install Instructions

### Requirements

- Latest Qt5 version with `widgets` and `network` modules

### Build

#### *nix

There's a small `Makefile` provided that does all necessary steps and creates a build directory, then copies the build artifact from the build directory. Just do `make` in the root directory, it should work.

Or you can use the usual Qt5 build process:

```sh
mkdir build
cd build
qmake ../src/kristall.pro
make
```

### Notes for OpenBSD
- It seems like Qt wants `libzstd.so.3.1` instead of `libzstd.so.3.2`. Just symlink that file into the build directory
- Use `make` and not `gmake` to build the project.

##### Notes for Ubuntu 20.04:
- Requires packages
  - `qt5-default`
  - `qt5-qmake`
  - `qtmultimedia5-dev` 
  - `libqt5svg5-dev`
  - `libssl-dev`
  - `make`
  - `g++`
 
These notes are probably also correct for Debian (someone please verify)

##### Notes for Manjaro/Arch
- Requires packages
  - `qt5`
  - `qt5-multimedia`

##### Notes on void linux
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
2. Use `make` to build th executable

### Manual Installation

#### Unix / XDG

The provided desktop file can be installed into the local system
```sh
ln -s Kristall.desktop ~/.local/share/applications/kristall.desktop
```

#### Haiku

Link the `kristall` executable to your applications menu:

```sh
cd ~/config/settings/deskbar/menu/Applications/
ln -s /path/to/kristall .
```

## Credits

- Thanks to [James Tomasino](https://tomasino.org) for helping out with understanding gopher
- Thanks to [Vane Vander](https://mayvaneday.art/) for providin the Haiku build instructions
- Thanks to James Tomasino, styan and tiwesdaeg for improving the `Makefile`
- Thanks to [Alex Naskos](https://github.com/alexnask) for providing windows build instructions
- Thanks to tiwesdaeg for improving the application icon

## TODO / Roadmap

### 0.3 - TLS and security
- [ ] TLS Handling
   - [ ] Enable TOFU for HTTPS
  - [ ] Add management for client certificates
    - [ ] Rename/delete/merge groups
    - [ ] Import/export PEM certificates and keys
    - [ ] Add a "scope" option to certificates so users can restrict the scope where the certificate is valid
- [ ] UX improvements
  - [ ] Auto-expanding outline
- [ ] Allow configurable timeouts
	- [ ] Implement server timeouts
	- [ ] Add configuration option for connect/transfer timeouts
- [ ] Fix: Status bar
    
### 0.4 - The colorful release
- [ ] Implement dual-colored icon theme
- [ ] Improve UX
  - [ ] Provide text search function
  - [ ] auto-highlighting/following outline
  - [ ] Add maximum text widht option
- [ ] Improved favourites
  - [ ] Allow custom favourite title (not the URL itself)
  - [ ] Allow grouping favourites into a single-hierarchy folder structure
- [ ] Improve Unicode/Emoji support
  - Seems to need multiple font families per font?
- [ ] Replace Qt markdown with standalone markdown renderer
  - [ ] Enable markdown theming
- [ ] Improve Styles
  - [ ] Add text/kristall-theme mime
    - [ ] Will use a preview document instead of displaying the content
    - [ ] Allow import of themes
    - [ ] File extension is `.kthm`
  - [ ] Direct preview instead of importing it as a preset. Allow user to save preset then manually
- [ ] Recognize home directories with /~home and such and add "substyles"
- [ ] [Add favicon support](gemini://mozz.us/files/rfc_gemini_favicon.gmi)
  - [ ] Add auto-generated "favicons"
  - [ ] Check if the site follows this guideline: `#<ICON> Title` where `<ICON>` is a unicode emoji
  - [ ] Opt-In: Regularly check for `domain/favicon.txt`
  - [ ] Opt-In: Regularly check for `domain/favicon.ico`

### Unspecced
- [ ] Add option: "Transient certificates survive an application reboot and are stored on disk"
- [ ] FTP
  - [ ] Search for FTP library or use self-written one?
- [ ] Implement more protocols
  - [ ] Gopher
    - [ ] Support more media types (include uudecode and hexbin decoder)
- [ ] Improve UX
- [ ] Implement data streaming
  - [ ] Rework the networking API to allow streaming of content instead of receive everything, then display
  - [ ] Benefits for audio/video/progressive image formats
  - [ ] 

## Bugs
  
> <styan> xq: When using torsocks(1) on kristall QNetworkInterface complains loudly about not being permitted to create an IPv6 socket..

MAC needs different default font:
https://usercontent.irccloud-cdn.com/file/OrkuXDfQ/image.png




Status bar is broken:
It doesn't seem to map to tabs, if you have multiple
open, which was a bit confusing (and would also be a really cool
addition for easy comparison of responses).

<tiwesdaeg> gemini://tilde.pink/ the first paragraph is displayed with large spacing
