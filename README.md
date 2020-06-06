# Kristall
A high-quality visual cross-platform gemini browser.

![Preview Image](https://mq32.de/public/affb08915c7e5d5d37dc702134f5af18e4dc8cd1.png)

## Features
- Document rendering
  - `text/gemini`
  - `text/html` (reduced feature set)
  - `text/markdown`
  - `text/*` 
  - `image/*
- Outline generation
- Favourite Sites
- Tabbed interface
- Survives [ConMans torture suite](gemini://gemini.conman.org/test/torture/)

## TODO
- [ ] Survive full torture suite
  - [ ] Correctly parse mime parameters
    - [ ] Correctly parse charset (0013, 0014)
    - [ ] Correctly parse other params (0015)
    - [ ] Correctly parse undefined params (0016)
- [ ] Make document style customizable
- [ ] Add history navigation