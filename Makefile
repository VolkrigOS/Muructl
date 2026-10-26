
CC = cc
STRIP = strip

PREFIX  = /usr
BINDIR  = $(PREFIX)/bin
DESTDIR =

CFLAGS  = -Os \
          -flto \
          -ffunction-sections \
          -fdata-sections \
          -fno-unwind-tables \
          -fno-asynchronous-unwind-tables \
          -fno-ident \
          -fomit-frame-pointer \
          -DNDEBUG \
          -Wall -Wextra

LDFLAGS = -flto \
          -Wl,--gc-sections \
          -Wl,--strip-all

all: muructl murud

muructl: ctl.c
	$(CC) $(CFLAGS) ctl.c -o muructl $(LDFLAGS)
	$(STRIP) --strip-all muructl

murud: murud.c
	$(CC) $(CFLAGS) murud.c -o murud $(LDFLAGS)
	$(STRIP) --strip-all murud

install: all
	install -Dm755 muructl $(DESTDIR)$(BINDIR)/muructl
	install -Dm755 murud $(DESTDIR)$(BINDIR)/murud

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/muructl
	rm -f $(DESTDIR)$(BINDIR)/murud

clean:
	rm -f muructl murud

.PHONY: all install uninstall clean
