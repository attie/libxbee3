DEFCONFIG:=            linux.mk
BUILD_RULES:=          darwin.mk
INSTALL_RULES:=        darwin.mk

AS=                    $(CROSS_COMPILE)as
GCC=                   $(CROSS_COMPILE)gcc
GXX=                   $(CROSS_COMPILE)g++
LD=                    $(CROSS_COMPILE)ld
OBJCOPY=               $(CROSS_COMPILE)objcopy
AR=                    $(CROSS_COMPILE)ar
DEFLATE:=              gzip
SYMLINK:=              ln
MKDIR=                 @if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
RM:=                   rm -f
RMDIR:=                rm -rf
INSTALL=               install -g $(SYS_GROUP) -o $(SYS_USER) -DT
MAN2HTML:=             man2html

DEBUG:=                -g
CFLAGS+=               -Wall -c -fPIC $(DEBUG) $(addprefix -D,$(OPTIONS))
#CFLAGS+=              -pedantic
#CFLAGS+=               -fvisibility=hidden
CFLAGS+=               -Wno-variadic-macros
CXXFLAGS:=             $(CFLAGS) -fvisibility=hidden
CFLAGS+=               -Wstrict-prototypes
CLINKS+=               -fPIC $(addprefix -l,$(LIBS)) $(DEBUG)
CXXLINKS+=             $(CLINKS)

LIB_OUT=               $(DESTDIR)/$(LIBNAME).dylib                    \
                       $(DESTDIR)/$(LIBNAME).dylib.$(LIBFULLREV).dbg  \
                       $(DESTDIR)/$(LIBNAME).a                     \
                       $(DESTDIR)/$(LIBNAME)p.dylib                   \
                       $(DESTDIR)/$(LIBNAME)p.dylib.$(LIBFULLREV).dbg \
                       $(DESTDIR)/$(LIBNAME)p.a                    \
                       $(addprefix $(HTMLDIR)/,$(SYS_HTMLPAGES))

INSTALL_FILES=         $(SYS_LIBDIR)/$(LIBNAME).dylib.$(LIBFULLREV)                    \
                       $(SYS_LIBDIR)/$(LIBNAME).dylib.$(LIBFULLREV).dbg                \
                       $(SYS_LIBDIR)/$(LIBNAME).dylib                                  \
                       $(SYS_LIBDIR)/$(LIBNAME).a.$(LIBFULLREV)                     \
                       $(SYS_LIBDIR)/$(LIBNAME).a                                   \
                       $(SYS_LIBDIR)/$(LIBNAME)p.dylib.$(LIBFULLREV)                   \
                       $(SYS_LIBDIR)/$(LIBNAME)p.dylib.$(LIBFULLREV).dbg               \
                       $(SYS_LIBDIR)/$(LIBNAME)p.dylib                                 \
                       $(SYS_LIBDIR)/$(LIBNAME)p.a.$(LIBFULLREV)                    \
                       $(SYS_LIBDIR)/$(LIBNAME)p.a                                  \
                       $(addprefix $(SYS_MANDIR)/,$(addsuffix .gz,$(SYS_MANPAGES))) \
                       $(SYS_INCDIR)/xbee.h                                         \
                       $(SYS_INCDIR)/xbeep.h

RELEASE_FILES=         $(DESTDIR)/$(LIBNAME).dylib.$(LIBFULLREV)      \
                       $(DESTDIR)/$(LIBNAME).dylib.$(LIBFULLREV).dbg  \
                       $(DESTDIR)/$(LIBNAME).dylib                    \
                       $(DESTDIR)/$(LIBNAME).a.$(LIBFULLREV)       \
                       $(DESTDIR)/$(LIBNAME).a                     \
                       $(DESTDIR)/$(LIBNAME)p.dylib.$(LIBFULLREV)     \
                       $(DESTDIR)/$(LIBNAME)p.dylib.$(LIBFULLREV).dbg \
                       $(DESTDIR)/$(LIBNAME)p.dylib                   \
                       $(DESTDIR)/$(LIBNAME)p.a.$(LIBFULLREV)      \
                       $(DESTDIR)/$(LIBNAME)p.a                    \
                       $(addprefix $(MANDIR)/,$(SYS_MANPAGES))     \
                       $(addprefix $(HTMLDIR)/,$(SYS_HTMLPAGES))   \
                       xbee.h                                      \
                       xbeep.h                                     \
                       README HISTORY COPYING COPYING.LESSER

CLEAN_FILES=           $(BUILDDIR)/*.o \
                       $(BUILDDIR)/*.d

DISTCLEAN_FILES=       $(HTMLDIR)/*.html \
                       $(HTMLDIR)/*/*.html

VER_DEFINES=           -DLIB_REVISION="\"$(LIBFULLREV)\""                             \
                       -DLIB_COMMIT="\"$(shell git log -1 --format="%H")\""           \
                       -DLIB_COMMITTER="\"$(shell git log -1 --format="%cn <%ce>")\"" \
                       -DLIB_BUILDTIME="\"$(shell date)\""
