CC ?= cc
PKG_CONFIG ?= pkg-config
CFLAGS += -std=c11 -O2 -Wall -Wextra -Wpedantic $(shell $(PKG_CONFIG) --cflags gtk4)
LDLIBS += $(shell $(PKG_CONFIG) --libs gtk4)

.PHONY: all clean selftest
all: nfkcgen

nfkcgen: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

# build_string 분기 구조 검증 (GTK UI 없이 실행)
selftest: main.c
	$(CC) $(CFLAGS) -DNFKCGEN_SELFTEST -o nfkcgen-selftest $< $(LDLIBS)
	./nfkcgen-selftest

clean:
	rm -f nfkcgen nfkcgen-selftest
