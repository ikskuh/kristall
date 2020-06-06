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
- [ ] Make document style customizable
- [ ] Add history navigation
  - [ ] "also, being able to click and load a url from the history pane"
  - [ ] "Couldn't you just have  an array of URLs? And when they go forward, you slice the array up to the that point and add the new url to the end"