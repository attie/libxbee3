#CROSS_COMPILE?=

LIBDIR:=   ../..
LIBDEPS:=  xbee.h

libxbee_cpp.so: libxbee.o
	$(CROSS_COMPILE)g++ -shared $^ -o $@

libxbee.o: libxbee.cpp $(addprefix $(LIBDIR)/,$(LIBDEPS))
	$(CROSS_COMPILE)g++ -iquote $(LIBDIR) $(filter %.cpp,$^) -c -o $@
