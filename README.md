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
  - macOS
  - Haiku

## Screenshots

### Generates Outlines

![Outline Generation](https://mq32.de/public/a50ef327f4150d870393b1989c5b41db495b56f7.png)

### Fully Customizable Site Theme

![Site Theme](https://mq32.de/public/7123e22a58969448c27b24df8510f4d56921bf23.png)

## Build/Install Instructions

**Note:** `master` branch is the latest development status (sometimes called "nightly") whereas the tagged versions are the stable releases.

If you want to build a stable experience, check out the latest version and build that!

See [BUILDING.md](BUILDING.md)

## Credits

- Thanks to [James Tomasino](https://tomasino.org) for helping out with understanding gopher
- Thanks to [Vane Vander](https://mayvaneday.art/) for providing the Haiku build instructions
- Thanks to James Tomasino, styan and tiwesdaeg for improving the `Makefile`
- Thanks to [Alex Naskos](https://github.com/alexnask) for providing windows build instructions
- Thanks to tiwesdaeg for improving the application icon

### Bundled Dependencies

- Thanks to [Alexander Huszagh](https://github.com/Alexhuszagh) and
  [BreezeStyleSheets contributors](https://github.com/Alexhuszagh/BreezeStyleSheets/graphs/contributors) for
  the dependency [BreezeStyleSheets](https://github.com/bkmgit/kristall/tree/master/lib/BreezeStyleSheets)
  available under an MIT license
- Thanks to [John MacFarlane](https://github.com/jgm) and
  [Cmark contributors](https://github.com/commonmark/cmark/graphs/contributors) for the dependency
  [cmark](https://github.com/MasterQ32/kristall/tree/master/lib/cmark) which is available under BSD-2-Clause
  and MIT licenses
- Thanks to Jonathan Tang and [gumbo parser contributors](https://github.com/google/gumbo-parser/graphs/contributors)
  for the dependency [gumbo parser](https://github.com/MasterQ32/kristall/tree/master/lib/gumbo-parser)
  available under an Apache-2.0 license
- Thanks to [Luis Lpz](https://gist.github.com/luis-l) for the dependency
  [luis-l-gist](https://github.com/MasterQ32/kristall/tree/master/lib/luis-l-gist)
  
## Changelog

See [src/about/updates.gemini](src/about/updates.gemini)

## Roadmap

See [ROADMAP.md](ROADMAP.md)

## License

Kristall is released under the GPLv3 or (at your option) any later version.
[See LICENSE as well](LICENSE)
