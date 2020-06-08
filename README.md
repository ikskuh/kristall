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

## Screenshots

### Generates Outlines

![Outline Generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png)

### Fully Customizable Site Theme

![Site Theme](https://mq32.de/public/7123e22a58969448c27b24df8510f4d56921bf23.png)

## Build Instructions

### Requirements

- Latest Qt5 version with `widgets` and `network` modules

### Build

## *nix

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

### Notes for Ubuntu 20.04:
- Requires packages
  - `qt5-default`
  - `qt5-qmake`
  - `qtmultimedia5-dev` 
  - `make`
  - `g++`

### Notes for Manjaro/Arch
- Requires packages
  - `qt5`
  - `qt5-multimedia`

### Notes on void linux
- set env variable `QT_SELECT=5`

## Windows

Just use QtCreator to build `./src/kristall.pro`. Default settings should be fine.

## TODO
- [ ] Survive full torture suite
  - [ ] Correctly parse mime parameters
    - [ ] Correctly parse charset (0013, 0014)
    - [ ] Correctly parse other params (0015)
    - [ ] Correctly parse undefined params (0016)
- [ ] TLS Handling
   - [ ] Allow user to ignore TLS errors
   - [ ] Enable TOFU for HTTPS/Gemini
   - [ ] Enable Client Certificate Management
- [ ] Recognize home directories with /~home and such and add "substyles"
- [ ] Add favicon support
  - [ ] Add auto-generated "favicons"
  - [ ] Check if the site follows this guideline: `#<ICON> Title` where `<ICON>` is a unicode emoji
  - [ ] Opt-In: Regularly check for `domain/favicon.txt`
  - [ ] Opt-In: Regularly check for `domain/favicon.ico`
- [ ] Implement more protocols
  - [ ] Gopher
    - [ ] Support more media types (include uudecode and hexbin decoder)
  - [ ] FTP
    - [ ] Search for FTP library or use self-written one?
- [ ] Improve UX
  - [ ] Rightclick with "open in new tab" and "open in this tab"
   - [ ] For history
   - [ ] For favourites
   - [ ] For documents
  - [ ] Image Zoom and Pan
  - [ ] Save/load/share theme preset
  - [ ] Improve Unicode/Emoji support
    - Seems to need multiple font families per font?

## Bugs
  
> <styan> xq: When using torsocks(1) on kristall QNetworkInterface complains loudly about not being permitted to create an IPv6 socket..

> Build on MacOS outputs kristall.app, not kristall
> Same for windows probably with kristall.exe
> <IceMichael> ah, ist unter mac ein kristall.app

MAC needs different default font

https://usercontent.irccloud-cdn.com/file/OrkuXDfQ/image.png

<IceMichael> Andale Mono, falls du das für mac irgendwie voreinstellen willst (aber kA, ob die überall diese ist)