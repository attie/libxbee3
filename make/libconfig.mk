LIBMAJ:=               3
LIBMIN:=               0
LIBREV:=               7

LIBFULLREV:=           $(LIBMAJ).$(LIBMIN).$(LIBREV)

LIBNAME:=              libxbee

MANDIR:=               man
HTMLDIR:=              html
BUILDDIR:=             .build
DESTDIR:=              lib

CONSTRUCTIONDIRS:=     $(BUILDDIR) $(DESTDIR)

SYS_HEADERS:=          xbee.h xbeep.h

### the OS config can override this if any are incompatible
MODELIST:=             xbee1 xbee2 xbee5 net debug
