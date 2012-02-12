OBJS:=$(addprefix $(BUILDDIR)/,$(addsuffix .o,$(SRCS)))
PDEPS:=makefile ../buildmode.mk ../../config.mk ../../buildconfig.mk

all: $(DESTDIR)/_mode.o


clean:
	rm -f $(BUILDDIR)/*.o
	rm -rdf $(DESTDIR)/*

distclean:
	rm -rdf $(BUILDDIR) .$(BUILDDIR).dir
	rm -rdf $(DESTDIR) .$(DESTDIR).dir


.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
	@touch $@



$(DESTDIR)/_mode.o: .$(DESTDIR).dir $(OBJS)
	$(LD) -r $(filter %.o,$^) -o $@


$(BUILDDIR)/%.d: .$(BUILDDIR).dir %.c $(PDEPS)
	$(GCC) -MM -MT $(addprefix $(BUILDDIR)/,$*.o) $*.c -o $@

$(BUILDDIR)/%.o: $(BUILDDIR)/%.d $(PDEPS)
	$(GCC) $(CFLAGS) $*.c -o $@

include $(wildcard $(BUILDDIR)/*.d)
