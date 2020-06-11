# Install to /usr/local unless otherwise specified, such as `make PREFIX=/app`
PREFIX=/usr/local

# What to run to install various files
INSTALL=install -D
# Run to install the actual binary
INSTALL_PROGRAM=$(INSTALL)
# Run to install application data, with differing permissions
INSTALL_DATA=$(INSTALL) -m 644

# Directories into which to install the various files
bindir=$(DESTDIR)$(PREFIX)/bin
sharedir=$(DESTDIR)$(PREFIX)/share

kristall: build/kristall
	cp build/kristall $@

build/kristall:
	mkdir -p build
	cd build && qmake ../src/kristall.pro && $(MAKE)

install: kristall
	# Install icons
	$(INSTALL_DATA) src/icons/kristall.svg $(sharedir)/icons/hicolor/scalable/apps/net.random-projects.kristall.svg
	$(INSTALL_DATA) src/icons/kristall-16.png $(sharedir)/icons/hicolor/16x16/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-32.png $(sharedir)/icons/hicolor/32x32/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-64.png $(sharedir)/icons/hicolor/64x64/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) src/icons/kristall-128.png $(sharedir)/icons/hicolor/128x128/apps/net.random-projects.kristall.png
	$(INSTALL_DATA) Kristall.desktop $(sharedir)/applications/Kristall.desktop
	$(INSTALL_PROGRAM) kristall $(bindir)/kristall

uninstall:
	# Remove the .desktop
	rm -f $(sharedir)/applications/Kristall.desktop
	# Remove the icons
	rm -f $(sharedir)/icons/hicolor/scalable/apps/net.random-projects.kristall.png
	rm -f $(sharedir)/icons/hicolor/*x*/apps/net.random-projects.kristall.png
	# Remove the binary
	rm -f $(bindir)/kristall


clean:
	rm -rf build
	rm -f kristall
