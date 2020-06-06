# Kristall
A high-quality visual cross-platform gemini browser.

![Preview Image](https://mq32.de/public/affb08915c7e5d5d37dc702134f5af18e4dc8cd1.png)

## Features
- Document rendering
  - `text/gemini`
  - `text/html` (reduced feature set)
  - `text/markdown`
  - `text/*` 
  - `image/*`
- Outline generation
- Favourite Sites
- Tabbed interface
- Survives [ConMans torture suite](gemini://gemini.conman.org/test/torture/)
- [Special link highlighting for different targets](https://mq32.de/public/92f3ec7a64833d01f1ed001d15c8db4158e5d3c2.png)
- Color Themes
  - Custom color theme
  - Automatic light/dark theme based on the host name
- Navigation history
- Crossplatform supports
  - Linux
  - Windows
  - FreeBSD
  - NetBSD

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
  - [ ] Opt-In: Regularly check for `domain/favicon.ico`
- [ ] Theming
  - [ ] Fix default theme when not found.
