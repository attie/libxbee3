include ../config.mk
include ../buildconfig.mk

SRCS:=common

OBJS:=$(addsuffix /lib/mode.o,$(MODELIST)) $(addsuffix .o,$(SRCS))

.PHONY: ALWAYS


all: modes.o


clean:
	rm -f modes.o
	@for dir in `find . -mindepth 1 -maxdepth 1 -type d`; do echo $(MAKE) -C $$dir clean; $(MAKE) -C $$dir clean ; done

distclean:
	rm -f modes.o
	@for dir in `find . -mindepth 1 -maxdepth 1 -type d`; do echo $(MAKE) -C $$dir distclean; $(MAKE) -C $$dir distclean ; done


modes.o: $(OBJS)
	$(LD) -r $(filter %.o,$^) -o $@

%/lib/mode.o: % ALWAYS
	$(MAKE) -C $*

