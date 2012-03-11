### libxbee internal build configuration options:
### it is recommended that you DO NOT change this file

BUILDDIR:=      .build
DESTDIR:=       lib
MANDIR:=        man

LIBFULLREV:=    $(LIBMAJ).$(LIBMIN).$(LIBREV)

SYS_HEADERS:=   $(SYS_HEADERS)
SYS_MANPAGES:=  $(subst $(MANDIR)/,,$(SYS_MANPAGES))
SYS_MANLINKS:=  $(subst $(MANDIR)/,,$(SYS_MANLINKS))

RELEASE_FILES:= $(RELEASE_FILES)
INSTALL_FILES:= $(INSTALL_FILES)

AR:=            $(CROSS_COMPILE)ar
LD:=            $(CROSS_COMPILE)ld
GCC:=           $(CROSS_COMPILE)gcc
GPP:=           $(CROSS_COMPILE)g++
OBJCOPY:=       $(CROSS_COMPILE)objcopy
GZIP:=          gzip
MAKE+=          --no-print-directory

DEBUG:=         -g
CFLAGS+=        -Wall -c -fPIC $(DEBUG) $(addprefix -D,$(OPTIONS))
#CFLAGS+=       -pedantic
CFLAGS+=        -fvisibility=hidden
CPPFLAGS:=      $(CFLAGS)
CFLAGS+=        -Wstrict-prototypes -Wno-variadic-macros
CLINKS+=        $(addprefix -l,$(LIBS)) $(DEBUG)

COMMA:=         ,
.PRECIOUS: .%.dir $(BUILDDIR)/%.d
