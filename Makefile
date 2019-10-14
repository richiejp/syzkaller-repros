SOURCES=$(wildcard linux/*.c)
BINARIES=$(SOURCES:.c=)
prefix ?= /opt/ltp

define install-one
install -m 775 $(f) $(DESTDIR)$(prefix)/syzkaller/bin

endef

.PHONY: clean install

all: $(BINARIES)

CFLAGS=-pthread

$(BINARIES): %: %.c
	-@if grep -q "__NR_mmap2" $^; then \
		M32="-m32"; \
	fi; \
	$(CC) $(CFLAGS) $$M32 $(LDFLAGS) $^ -o $@; \
	echo $(CC) $(CFLAGS) $$M32 $(LDFLAGS) $^ -o $@;

install: $(BINARIES)
	mkdir -p $(DESTDIR)$(prefix)/syzkaller/bin
	$(foreach f, $^, $(install-one))

clean:
	rm -f $(BINARIES)
