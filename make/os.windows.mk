SHELL:=cmd

DEFCONFIG:=            windows.mk
BUILD_RULES:=          windows.mk

CC=                    "$(VCPATH)\bin\cl.exe"
LD=                    "$(VCPATH)\bin\link.exe"
RC=                    "$(SDKPATH)\bin\rc.exe"

MKDIR=                 mkdir $* || true
RM:=                   del /F
RMDIR:=                del /F /Q

DEBUG?=                /MT
CFLAGS+=               /nologo "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /RTCs /Gd $(DEBUG) $(addprefix /D,$(OPTIONS))
CLINKS+=               /nologo "/LIBPATH:$(SDKPATH)\Lib" "/LIBPATH:$(VCPATH)\lib" /VERSION:$(LIBMAJ).$(LIBMIN) $(LDBUG)
FINLNK:=               /DLL

LIB_OUT:=              $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll

CLEAN_FILES=           $(BUILDDIR)\*.o

VER_DEFINES:=          /DLIB_REVISION="\"$(LIBFULLREV)\""                      \
                       /DLIB_COMMIT="\"-- unknown --\""                        \
                       /DLIB_COMMITTER="\"-- unknown --\""                     \
                       /DLIB_BUILDTIME="\"$(shell date /T) $(shell time /T)\""

OPTIONS+=              XBEE_NO_NET_SERVER

POST_BUILD:=           windows.mk

MODELIST:=             xbee1 xbee2 xbee5
