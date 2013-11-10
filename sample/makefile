PROG?=main
.SUFFIXES:

all: $(PROG)
	./$(PROG)

new: clean all

clean:
	-rm $(PROG)

$(PROG): $(PROG).c ../../../lib/libxbee3.dll
	gcc $(filter %.c,$^) -g -o $@ -I ../../.. -L ../../../lib -lxbee3 -lpthreadGC2
