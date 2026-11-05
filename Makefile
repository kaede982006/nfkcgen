CC ?= cc
PKG_CONFIG ?= pkg-config
WARN = -std=c11 -O2 -Wall -Wextra -Wpedantic
GTK_CFLAGS = $(shell $(PKG_CONFIG) --cflags gtk4)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk4)
GLIB_CFLAGS = $(shell $(PKG_CONFIG) --cflags glib-2.0)
GLIB_LIBS = $(shell $(PKG_CONFIG) --libs glib-2.0)

.PHONY: all clean selftest
all: nfkcgen

# GUI: UI(main.c) + 생성 로직(generator.c)
nfkcgen: main.c generator.c generator.h
	$(CC) $(WARN) $(GTK_CFLAGS) -o $@ main.c generator.c $(GTK_LIBS)

# 생성 로직 검증: GTK 없이 glib만 링크
selftest: selftest.c generator.c generator.h
	$(CC) $(WARN) $(GLIB_CFLAGS) -o nfkcgen-selftest selftest.c generator.c $(GLIB_LIBS)
	./nfkcgen-selftest

clean:
	rm -f nfkcgen nfkcgen-selftest
