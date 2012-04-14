SHELL:=cmd
PROG?=main

### compiler locations
VCPATH:=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC
SDKPATH:=C:\Program Files\Microsoft SDKs\Windows\v6.0A

CC=                    "$(VCPATH)\bin\cl.exe"
CFLAGS+=               /nologo /RTCs /Gd /MT "/I..\..\.." "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /link "/LIBPATH:..\..\..\lib" "/LIBPATH:$(SDKPATH)\Lib" "/LIBPATH:$(VCPATH)\lib"
PATH+=;../../../lib

all: $(PROG).exe
	./$(PROG)

new: clean all

clean:
	-del /F /Q $(PROG)

$(PROG).exe: $(PROG).c ../../../lib/libxbee3.dll
	$(CC) $(filter %.c,$^) /Fe$@ $(CFLAGS)
