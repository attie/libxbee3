SHELL:=cmd

DEFCONFIG:=            windows.mk
BUILD_RULES:=          windows.mk

VCBINDIR?=bin
VCLIBDIR?=lib
SDKBINDIR?=bin
SDKLIBDIR?=lib

CC=                    "$(VCPATH)\$(VCBINDIR)\cl.exe"
LD=                    "$(VCPATH)\$(VCBINDIR)\link.exe"
RC=                    "$(SDKPATH)\$(SDKBINDIR)\rc.exe"

MKDIR=                 mkdir $* || true
RM:=                   del /F
RMDIR:=                del /F /Q

DEBUG?=                /MT
CFLAGS+=               /nologo "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /RTCs /Gd $(DEBUG) $(addprefix /D,$(OPTIONS)) /DLIBXBEE_BUILD
CLINKS+=               /nologo "/LIBPATH:$(SDKPATH)\$(SDKLIBDIR)" "/LIBPATH:$(VCPATH)\$(VCLIBDIR)" /VERSION:$(LIBMAJ).$(LIBMIN) $(LDBUG)
FINLNK:=               /DLL

LIB_OUT:=              $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll

CLEAN_FILES=           $(BUILDDIR)\*.o

POST_BUILD:=           windows.mk

MODELIST:=             xbee1 xbee2 xbee3 xbee5 xbee6b xbeeZB
