default: all
all: build-dir kristall
kristall: build/kristall
	cp build/kristall $@
build/kristall: build-dir
	cd build && qmake ../src/kristall.pro && $(MAKE)
build-dir:
	mkdir -p build
clean:
	rm -rf build
	rm -f kristall
.PHONY: build-dir
