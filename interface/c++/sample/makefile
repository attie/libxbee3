#CROSS_COMPILE?=

LIBDIR:=   ../../..
LIBDEPS:=  xbee.h

all: $(LIBDIR)/lib/libxbee.so ../libxbeep.so main

main: main.cpp
	$(CROSS_COMPILE)g++ $^ -o $@ -L. -L$(LIBDIR)/lib -lxbee -lxbeep -lpthread -lrt


.PHONY: run

run: all
	LD_LIBRARY_PATH=$(LIBDIR)/lib:.. ./main
