SHELL:=cmd

DEFCONFIG:=            windows.mk
BUILD_RULES:=          windows.mk

CC=                    "$(VCPATH)\bin\cl.exe"
LD=                    "$(VCPATH)\bin\link.exe"
RC=                    "$(SDKPATH)\bin\rc.exe"

MKDIR=                 mkdir $* || true
RM:=                   del /F
RMDIR:=                del /FQ

DEBUG:=                /MT
CFLAGS+=               /nologo "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /RTCs /Gz $(DEBUG) $(addprefix /D,$(OPTIONS))
CLINKS+=               /nologo "/LIBPATH:$(SDKPATH)\Lib" "/LIBPATH:$(VCPATH)\lib"
INCLNK:=               /INCREMENTAL /FORCE:UNRESOLVED 
FINLNK:=               /INCREMENTAL:NO /DLL 

LIB_OUT:=              $(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll

VER_DEFINES:=          /DLIB_REVISION="\"$(LIBFULLREV)\""                      \
                       /DLIB_COMMIT="\"-- unknown --\""                        \
                       /DLIB_COMMITTER="\"-- unknown --\""                     \
                       /DLIB_BUILDTIME="\"$(shell date /T) $(shell time /T)\""

OPTIONS+=              XBEE_NO_NET_SERVER

POST_BUILD:=           windows.mk
