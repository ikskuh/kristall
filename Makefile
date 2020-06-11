PREFIX ?= /usr/local
BINDIR ?= $(DESTDIR)$(PREFIX)/bin

default: all
all: kristall

kristall: build/kristall
	cp build/kristall $@

build/kristall:
	mkdir -p build
	cd build && qmake ../src/kristall.pro && $(MAKE)

install: kristall
	install -m 755 ./kristall $(BINDIR)/kristall

clean:
	rm -rf build
	rm -f kristall
