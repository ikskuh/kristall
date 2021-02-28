# Install to /usr/local unless otherwise specified, such as `make PREFIX=/app`
PREFIX?=/usr/local

# Man pages by default instaled to /usr/share/man, like above this can
# be specified on command line.
MANPATH?=/usr/share/man

# What to run to install various files
INSTALL?=install
# Run to install the actual binary
INSTALL_PROGRAM=$(INSTALL) -Dm 755
# Run to install application data, with differing permissions
INSTALL_DATA=$(INSTALL) -m 644

# Directories into which to install the various files
bindir=$(DESTDIR)$(PREFIX)/bin
sharedir=$(DESTDIR)$(PREFIX)/share
mandir=$(DESTDIR)$(MANPATH)/man1
MAKEDIR=mkdir -p

# Default Qmake Command For Ubuntu (and probably other Debian) distributions

QMAKE_COMMAND := qmake
# For Fedora 32 and similar distributions, use the next line instead of the above.
# QMAKE_COMMAND := /usr/bin/qmake-qt5

UNAME := $(shell uname)
# Homebrew on macOS does not link Qt5 into the system path.
ifeq ($(UNAME),Darwin)
	HOMEBREW_PATH=export PATH="$(PATH):/usr/local/opt/qt/bin";
endif

kristall: build/kristall
	cp build/kristall $@

.PHONY: build/kristall
build/kristall: src/*
	mkdir -p build
	cd build; $(HOMEBREW_PATH) $(QMAKE_COMMAND) CONFIG+=$(QMAKE_CONFIG) ../src/kristall.pro && $(MAKE)
	cd doc; ./gen-man.sh

install: kristall
	# Prepare directories
	$(MAKEDIR) $(sharedir)/icons/hicolor/scalable/apps/
	$(MAKEDIR) $(sharedir)/icons/hicolor/16x16/apps/
	$(MAKEDIR) $(sharedir)/icons/hicolor/32x32/apps/
	$(MAKEDIR) $(sharedir)/icons/hicolor/64x64/apps/
	$(MAKEDIR) $(sharedir)/icons/hicolor/128x128/apps/
	$(MAKEDIR) $(sharedir)/applications/
	$(MAKEDIR) $(sharedir)/mime/packages/
	$(MAKEDIR) $(bindir)

	# Install files
	$(INSTALL_DATA) src/icons/kristall.svg $(sharedir)/icons/hicolor/scalable/apps/net.random-projects.kristall.svg
	$(INSTALL_DATA) src/icons/kristall-16.png $(sharedir)/icons/hicolor/16x16/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-32.png $(sharedir)/icons/hicolor/32x32/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-64.png $(sharedir)/icons/hicolor/64x64/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-128.png $(sharedir)/icons/hicolor/128x128/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) Kristall.desktop $(sharedir)/applications/Kristall.desktop
	$(INSTALL_DATA) kristall-mime-info.xml $(sharedir)/mime/packages/kristall.xml
	$(INSTALL_DATA) doc/kristall.1 $(mandir)/kristall.1
	$(INSTALL_PROGRAM) kristall $(bindir)/kristall

uninstall:
	# Remove the .desktop
	rm -f $(sharedir)/applications/Kristall.desktop
	# Remove the icons
	rm -f $(sharedir)/icons/hicolor/scalable/apps/net.random-projects.kristall.png
	rm -f $(sharedir)/icons/hicolor/*x*/apps/net.random-projects.kristall.png
	# Remove the binary
	rm -f $(bindir)/kristall
	# Remove man page
	rm -f $(mandir)/kristall.1

clean:
	rm -rf build
	rm -f kristall
