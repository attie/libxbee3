#CROSS_COMPILE?=

LIBDIR:=   ../..
LIBDEPS:=  xbee.h

all: $(LIBDIR)/lib/libxbee.so libxbeep.so

libxbeep.so: libxbeep.o
	$(CROSS_COMPILE)g++ -shared $^ -o $@ -L$(LIBDIR)/lib -lxbee -lpthread -lrt

libxbeep.o: xbeep.cpp xbeep.h $(addprefix $(LIBDIR)/,$(LIBDEPS))
	$(CROSS_COMPILE)g++ -I$(LIBDIR) $(filter %.cpp,$^) -c -o $@
