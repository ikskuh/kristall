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

See [BUILDING.md](BUILDING.md)

## Credits

- Thanks to [James Tomasino](https://tomasino.org) for helping out with understanding gopher
- Thanks to [Vane Vander](https://mayvaneday.art/) for providin the Haiku build instructions
- Thanks to James Tomasino, styan and tiwesdaeg for improving the `Makefile`
- Thanks to [Alex Naskos](https://github.com/alexnask) for providing windows build instructions
- Thanks to tiwesdaeg for improving the application icon

## TODO / Roadmap

### 0.3 - TLS and security
- [ ] TLS Handling
  - [ ] Add management for client certificates
    - [ ] Rename/delete/merge groups
    - [ ] Import/export PEM certificates and keys
    - [ ] Add a "scope" option to certificates so users can restrict the scope where the certificate is valid
    
### 0.4 - The colorful release
- [ ] Implement dual-colored icon theme
- [ ] Improve UX
  - [ ] Provide text search function
  - [ ] auto-highlighting/following outline
  - [ ] Add maximum text width option
  - [ ] Remember scroll position for navigating back
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
- [ ] Smart searchbar
  - [ ] Allow quick access to favourites
  - [ ] Allow scrolling through "global" history
  - [ ] Allow users to gather a list of "visited pages"
  - [ ] Make default protocol configurable
- [ ] Ctrl-F search in documents
- [ ] Add "view source" option to show original document

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
- [ ] Explicitly don't support data:// urls

## Bugs
  
> <styan> xq: When using torsocks(1) on kristall QNetworkInterface complains loudly about not being permitted to create an IPv6 socket..

MAC needs different default font:
https://usercontent.irccloud-cdn.com/file/OrkuXDfQ/image.png




Status bar is broken:
It doesn't seem to map to tabs, if you have multiple
open, which was a bit confusing (and would also be a really cool
addition for easy comparison of responses).

<tiwesdaeg> gemini://tilde.pink/ the first paragraph is displayed with large spacing
