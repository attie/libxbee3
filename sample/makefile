PROG?=main
.SUFFIXES:

all: $(PROG)

run: all
	LD_LIBRARY_PATH=../../../lib ./$(PROG)

new: clean all

clean:
	-rm $(PROG)

$(PROG): $(PROG).c ../../../lib/libxbee3.dll
	gcc $(filter %.c,$^) -g -o $@ -I ../../.. -L ../../../lib -lxbee3 -lpthreadGC2
