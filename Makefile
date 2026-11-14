VERSION = 1.0
PREFIX ?= /usr

CC ?= cc
CFLAGS ?= -O2 -Wall
CPPFLAGS += -DNFKCGEN_VERSION=\"$(VERSION)\"
CFLAGS += $(shell pkg-config --cflags gtk4)
LDLIBS += $(shell pkg-config --libs gtk4) -lm

nfkcgen: main.c generator.c generator.h Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ main.c generator.c $(LDLIBS)

install: nfkcgen
	install -Dm755 nfkcgen $(DESTDIR)$(PREFIX)/bin/nfkcgen
	install -Dm644 org.xisik.nfkcgen.desktop $(DESTDIR)$(PREFIX)/share/applications/org.xisik.nfkcgen.desktop
	# shortcut: 1024px PNG dropped into the 256x256 theme dir; GTK scales it. Add real sizes only if the icon looks soft.
	install -Dm644 logo.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/256x256/apps/org.xisik.nfkcgen.png

clean:
	rm -f nfkcgen test-generator test-ui

.PHONY: clean install

check: test-generator
	./test-generator

test-generator: test-generator.c generator.c generator.h
	$(CC) $(CFLAGS) -Wextra -Werror -o $@ test-generator.c generator.c $(LDLIBS)

.PHONY: check

check-ui: test-ui
	xvfb-run -a dbus-run-session -- ./test-ui

test-ui: test-ui.c main.c generator.c generator.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Wextra -Werror -o $@ test-ui.c generator.c $(LDLIBS)

.PHONY: check-ui
