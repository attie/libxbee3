include ../config.mk
include ../buildconfig.mk


SRCS:=common


PDEPS:=makefile ../config.mk ../buildconfig.mk
OBJS:=$(addsuffix /$(DESTDIR)/_mode.o,$(MODELIST)) $(addprefix $(BUILDDIR)/,$(addsuffix .o,$(SRCS)))

.PHONY: ALWAYS


all: $(DESTDIR)/_modes.o


clean:
	rm -f $(BUILDDIR)/*.o
	rm -rdf $(DESTDIR)/*
	@for dir in `find . -mindepth 1 -maxdepth 1 -type d`; do if [ -f $$dir/makefile ]; then echo $(MAKE) -C $$dir clean; $(MAKE) -C $$dir clean; fi; done

distclean:
	rm -rdf $(BUILDDIR) .$(BUILDDIR).dir
	rm -rdf $(DESTDIR) .$(DESTDIR).dir
	@for dir in `find . -mindepth 1 -maxdepth 1 -type d`; do if [ -f $$dir/makefile ]; then echo $(MAKE) -C $$dir distclean; $(MAKE) -C $$dir distclean; fi; done


.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
	@touch $@


$(DESTDIR)/_modes.o: .$(DESTDIR).dir $(OBJS)
	$(LD) -r $(filter %.o,$^) -o $@


$(BUILDDIR)/%.d: .$(BUILDDIR).dir %.c $(PDEPS)
	$(GCC) -MM -MT $(addprefix $(BUILDDIR)/,$*.o) $*.c -o $@

$(BUILDDIR)/%.o: $(BUILDDIR)/%.d $(PDEPS)
	$(GCC) $(CFLAGS) $*.c -o $@

%/lib/_mode.o: % ALWAYS
	$(MAKE) -C $*

include $(wildcard $(BUILDDIR)/*.d)

