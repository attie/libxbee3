SHELL:=cmd

DEFCONFIG:=            mingw.mk
BUILD_RULES:=          mingw.mk
POST_BUILD:=           mingw.mk

AS=                    $(CROSS_COMPILE)as
GCC=                   $(CROSS_COMPILE)gcc
GXX=                   $(CROSS_COMPILE)g++
LD=                    $(CROSS_COMPILE)ld
OBJCOPY=               $(CROSS_COMPILE)objcopy
AR=                    $(CROSS_COMPILE)ar
DEFLATE:=              gzip
SYMLINK:=              ln
MKDIR=                 mkdir $*
RM:=                   rm -f
RMDIR:=                rm -rf
INSTALL=               install -g $(SYS_GROUP) -o $(SYS_USER) -DT
MAN2HTML:=             

DEBUG:=                -g
LIBS:=                 pthreadGC2
CFLAGS+=               -Wall -c $(DEBUG) $(addprefix -D,$(OPTIONS)) -DLIBXBEE_BUILD
CFLAGS+=               -Wno-unused-function
#CFLAGS+=              -pedantic
CFLAGS+=               -fvisibility=hidden
CFLAGS+=               -Wno-variadic-macros
CXXFLAGS:=             $(CFLAGS)
CFLAGS+=               -Wstrict-prototypes
CLINKS+=               $(addprefix -l,$(LIBS)) $(DEBUG)
CXXLINKS+=             $(CLINKS) -Llib -lxbee

LIB_OUT=               $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll                   \
                       $(DESTDIR)/$(LIBNAME)$(LIBMAJ).a                     \
                       $(addprefix $(HTMLDIR)/,$(SYS_HTMLPAGES))
#                       $(DESTDIR)/$(LIBNAME)p$(LIBMAJ).dll                  \
#                       $(DESTDIR)/$(LIBNAME)p$(LIBMAJ).a                    \

RELEASE_FILES=         $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll.$(LIBFULLREV)     \
                       $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll                   \
                       $(DESTDIR)/$(LIBNAME)$(LIBMAJ).a.$(LIBFULLREV)       \
                       $(DESTDIR)/$(LIBNAME)$(LIBMAJ).a                     \
                       $(addprefix $(MANDIR)/,$(SYS_MANPAGES))     \
                       $(addprefix $(HTMLDIR)/,$(SYS_HTMLPAGES))   \
                       xbee.h                                      \
                       xbeep.h                                     \
                       README HISTORY COPYING COPYING.LESSER
#                       $(DESTDIR)/$(LIBNAME)p.dll.$(LIBFULLREV)    \
#                       $(DESTDIR)/$(LIBNAME)p.dll                  \
#                       $(DESTDIR)/$(LIBNAME)p.a.$(LIBFULLREV)      \
#                       $(DESTDIR)/$(LIBNAME)p.a                    \

CLEAN_FILES=           $(BUILDDIR)/*.o \
                       $(BUILDDIR)/*.d

DISTCLEAN_FILES=       $(HTMLDIR)/*.html \
                       $(HTMLDIR)/*/*.html
