#CROSS_COMPILE?=

LIBDIR:=   ../..
LIBDEPS:=  xbee.h

all: $(LIBDIR)/lib/libxbee.so libxbeep.so main

main: main.cpp
	$(CROSS_COMPILE)g++ $^ -o $@ -L. -L$(LIBDIR)/lib -lxbee -lxbeep -lpthread -lrt

libxbeep.so: libxbeep.o
	$(CROSS_COMPILE)g++ -shared $^ -o $@ -L$(LIBDIR)/lib -lxbee -lpthread -lrt

libxbeep.o: xbeep.cpp xbeep.h $(addprefix $(LIBDIR)/,$(LIBDEPS))
	$(CROSS_COMPILE)g++ -I$(LIBDIR) $(filter %.cpp,$^) -c -o $@


.PHONY: run

run: main $(LIBDIR)/lib/libxbee.so libxbeep.so
	LD_LIBRARY_PATH=$(LIBDIR)/lib:. ./main