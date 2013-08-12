PROG?=main

all: $(PROG)
	DYLD_LIBRARY_PATH=../../../lib ./$(PROG)

new: clean all

clean:
	-rm $(PROG)

$(PROG): $(PROG).c ../../../lib/libxbee.dylib
	gcc $(filter %.c,$^) -g -o $@ -I ../../.. -L ../../../lib -lxbee -lpthread
