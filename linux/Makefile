CC ?= cc
PKG_CONFIG ?= pkg-config
WARN = -std=c11 -O2 -Wall -Wextra -Wpedantic
GTK_CFLAGS = $(shell $(PKG_CONFIG) --cflags gtk4)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk4)
GLIB_CFLAGS = $(shell $(PKG_CONFIG) --cflags glib-2.0)
GLIB_LIBS = $(shell $(PKG_CONFIG) --libs glib-2.0)

VERSION = 1.0.0
PREFIX ?= /usr
DESTDIR ?=
STAGE = build/stage

.PHONY: all clean selftest install stage deb rpm packages
all: nfkcgen

# GUI: UI(main.c) + 생성 로직(generator.c)
nfkcgen: main.c generator.c generator.h
	$(CC) $(WARN) $(GTK_CFLAGS) -o $@ main.c generator.c $(GTK_LIBS)

# 생성 로직 검증: GTK 없이 glib만 링크
selftest: selftest.c generator.c generator.h
	$(CC) $(WARN) $(GLIB_CFLAGS) -o nfkcgen-selftest selftest.c generator.c $(GLIB_LIBS)
	./nfkcgen-selftest

install: nfkcgen
	install -Dm755 nfkcgen $(DESTDIR)$(PREFIX)/bin/nfkcgen
	install -Dm644 org.xisik.nfkcgen.desktop $(DESTDIR)$(PREFIX)/share/applications/org.xisik.nfkcgen.desktop

stage: nfkcgen
	rm -rf $(STAGE)
	$(MAKE) install DESTDIR=$(CURDIR)/$(STAGE) PREFIX=/usr

# dist/nfkcgen_<버전>_amd64.deb
deb: stage
	rm -rf build/deb && cp -a $(STAGE) build/deb && mkdir -p build/deb/DEBIAN dist
	printf 'Package: nfkcgen\nVersion: $(VERSION)\nArchitecture: amd64\nMaintainer: Xisik <kaede982006@gmail.com>\nDepends: libgtk-4-1\nSection: devel\nPriority: optional\nHomepage: https://github.com/kaede982006/nfkcgen\nDescription: NFKC research string generator (GTK 4)\n Generates classic/modern type 1-4 experimental strings and compares\n code point lengths before and after NFKC normalization.\n' > build/deb/DEBIAN/control
	dpkg-deb --root-owner-group --build build/deb dist/nfkcgen_$(VERSION)_amd64.deb

# dist/nfkcgen-<버전>-1.x86_64.rpm (라이브러리 의존성은 rpmbuild가 자동 생성)
rpm: stage
	mkdir -p dist build/rpm
	rpmbuild -bb nfkcgen.spec --define "_topdir $(CURDIR)/build/rpm" \
	    --define "version $(VERSION)" --define "stage $(CURDIR)/$(STAGE)"
	cp build/rpm/RPMS/x86_64/nfkcgen-$(VERSION)-1.x86_64.rpm dist/

packages: deb rpm

clean:
	rm -rf nfkcgen nfkcgen-selftest build
