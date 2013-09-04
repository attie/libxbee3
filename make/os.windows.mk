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
CFLAGS+=               /nologo "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /RTCs /Gd $(DEBUG) $(addprefix /D,$(OPTIONS)) /DLIBXBEE_BUILD
CLINKS+=               /nologo "/LIBPATH:$(SDKPATH)\Lib" "/LIBPATH:$(VCPATH)\lib" /VERSION:$(LIBMAJ).$(LIBMIN) $(LDBUG)
FINLNK:=               /DLL

LIB_OUT:=              $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll

CLEAN_FILES=           $(BUILDDIR)\*.o

POST_BUILD:=           windows.mk

MODELIST:=             xbee1 xbee2 xbee3 xbee5 xbee6b xbeeZB
