all: build-dir kristall

kristall: build/kristall
		cp $< $@

build/kristall: build-dir
		cd build && qmake ../src/kristall.pro && $(MAKE)

build-dir:
		mkdir -p build

.PHONY: build-dir
.SUFFIXES:
