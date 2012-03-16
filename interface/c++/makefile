LIBDIR:=   ../..
LIBDEPS:=  xbee.h

libxbee_cpp.so: libxbee.o
	g++ -shared $^ -o $@

libxbee.o: libxbee.cpp $(addprefix $(LIBDIR)/,$(LIBDEPS))
	g++ -iquote $(LIBDIR) $(filter %.cpp,$^) -c -o $@
