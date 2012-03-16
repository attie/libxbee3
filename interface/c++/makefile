LIBDIR:=   ../..
LIBDEPS:=  xbee.h

libxbee.o: libxbee.cpp $(addprefix $(LIBDIR)/,$(LIBDEPS))
	g++ -iquote $(LIBDIR) $(filter %.cpp,$^) -c -o $@
