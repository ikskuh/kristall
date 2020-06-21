# Kristall
A high-quality visual cross-platform gemini browser.

![Preview Image](https://mq32.de/public/336ac416892fd9064593631e7be9f7d8e266196b.png)

## Features
- Multi-protocol support
  - [Gemini](https://gemini.circumlunar.space/)
  - HTTP
  - HTTPS
  - [Finger](https://tools.ietf.org/html/rfc1288)
  - [Gopher](https://tools.ietf.org/html/rfc1436)
- Document rendering
  - `text/gemini`
  - `text/html` (reduced feature set)
  - `text/markdown`
  - `text/*`
  - `image/*`
  - `video/*`
  - `audio/*`
- TLS Management
  - Supports client certificates
  - Supports TOFU and CA TLS handling for both Gemini and HTTPS
- [Outline generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png) ([Video](https://mq32.de/public/kristall-02.mp4))
- Favourite Sites
- Navigation history
- Tabbed interface
- Survives [ConMans torture suite](gemini://gemini.conman.org/test/torture/) as well as the [Egsam Torture Suite](gemini://egsam.pitr.ca/)
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

See [BUILDING.md](BUILDING.md)

## Credits

- Thanks to [James Tomasino](https://tomasino.org) for helping out with understanding gopher
- Thanks to [Vane Vander](https://mayvaneday.art/) for providin the Haiku build instructions
- Thanks to James Tomasino, styan and tiwesdaeg for improving the `Makefile`
- Thanks to [Alex Naskos](https://github.com/alexnask) for providing windows build instructions
- Thanks to tiwesdaeg for improving the application icon

## Changelog

See [src/about/updates.gemini](src/about/updates.gemini)

## Roadmap

See [ROADMAP.md](ROADMAP.md)