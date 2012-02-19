PROG?=main

all: $(PROG) ../../lib/libxbee.so
	LD_LIBRARY_PATH=../../lib ./$^

new: clean all

clean:
	-rm $(PROG)

$(PROG): $(PROG).c
	gcc $^ -g -o $@ -I ../.. -L ../../lib -lxbee
