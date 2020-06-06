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
- [Outline generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png) ([Video](https://mq32.de/public/kristall-02.mp4))
- Favourite Sites
- Tabbed interface
- Survives [ConMans torture suite](gemini://gemini.conman.org/test/torture/)
- [Special link highlighting for different targets](https://mq32.de/public/92f3ec7a64833d01f1ed001d15c8db4158e5d3c2.png)
- Color Themes
  - Custom color theme
  - [Automatic light/dark theme based on the host name](https://mq32.de/public/kristall-01.mp4)
- Navigation history
- Crossplatform supports
  - Linux
  - Windows
  - FreeBSD
  - NetBSD
  - OpenBSD

## Screenshots

### Generates Outlines

![Outline Generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png)

### Fully Customizable Site Theme

![Site Theme](https://mq32.de/public/7123e22a58969448c27b24df8510f4d56921bf23.png)

## Build Instructions

### Requirements

- Latest Qt5 version with `widgets` and `network` modules

### Build

The usual Qt5 build process:

```sh
mkdir build
cd build
qmake ../kristall.pro
make
```

Notes for OpenBSD:
- It seems like Qt wants `libzstd.so.3.1` instead of `libzstd.so.3.2`. Just symlink that file into the build directory
- Use `make` and not `gmake` to build the project.

## TODO
- [ ] Survive full torture suite
  - [ ] Correctly parse mime parameters
    - [ ] Correctly parse charset (0013, 0014)
    - [ ] Correctly parse other params (0015)
    - [ ] Correctly parse undefined params (0016)
- [ ] Recognize home directories with /~home and such and add "substyles"
- [ ] Add favicon support
  - [ ] Add auto-generated "favicons"
  - [ ] Check if the site follows this guideline: `#<ICON> Title` where `<ICON>` is a unicode emoji
  - [ ] Opt-In: Regularly check for `domain/favicon.txt`
  - [ ] Opt-In: Regularly check for `domain/favicon.ico`

