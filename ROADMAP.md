# Roadmap

This document contains TODO items for planned Kristall releases as well as some random TODOs and reported bugs.

## 0.4 - The colorful release
- [x] Implement dual-colored icon theme
- [ ] Improve UX
  - [x] Fix all tab-indices
  - [ ] Provide text search function
  - [ ] auto-highlighting/following outline
  - [ ] Add maximum text width option
  - [ ] Remember scroll position for navigating back
- [ ] Improved favourites
  - [ ] Allow custom favourite title (not the URL itself)
  - [ ] Allow grouping favourites into a single-hierarchy folder structure
- [ ] Improve Unicode/Emoji support
  - Seems to need multiple font families per font?
- [x] Replace Qt markdown with standalone markdown renderer
  - [x] Enable basic markdown theming
  - [ ] Extent theming to support H4…H6, emph/bold text
  - [ ] Improve block quote / code block rendering
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
- [ ] Implement graphic fingerprint display instead of hex-based one
- [ ] `<krixano>` xq, I have a feature request for Kristall - the ability to middle click a tab to close it.
- [ ] `<krixano>` Also, middle clicking links to open them in new tab
- [ ] Support "offline files"
  - [ ] Allow manually caching a file to be visited when no internet connection is 
  - [ ] Add an "offline mode" that only allowes cached files
  - [ ] New url scheme for cached sites: kristall+cache://
  - [ ] Add window that 
- [ ] Folder based color scheme system
  - [ ] Migrate settings-based color schemes to folder
  - [ ] Add per-site scheming
  - [ ] Define human-readable color scheme file format

## Unspecced
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

<styan> xq: Also, Kristall still compilers on my FreeBSD laptop.  And still core-dumps if it is compiled while LibreSSL is installed (not a Kristall problem, just a QT problem).