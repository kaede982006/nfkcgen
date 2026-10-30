CC ?= cc
PKG_CONFIG ?= pkg-config
CFLAGS += -std=c11 -O2 -Wall -Wextra -Wpedantic $(shell $(PKG_CONFIG) --cflags gtk4)
LDLIBS += $(shell $(PKG_CONFIG) --libs gtk4)

.PHONY: all clean
all: nfkc-string-generator

nfkc-string-generator: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f nfkc-string-generator
