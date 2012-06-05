PROG?=main

all: $(PROG)
	LD_LIBRARY_PATH=../../../lib ./$(PROG)

new: clean all

clean:
	-rm $(PROG)

$(PROG): $(PROG).c zdo.c zdo.h ../../../lib/libxbee.so
	gcc $(filter %.c,$^) -g -o $@ -I ../../.. -L ../../../lib -lxbee -lpthread -lrt
