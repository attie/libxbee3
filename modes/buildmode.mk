OBJS:=$(addprefix $(BUILDDIR)/,$(addsuffix .o,$(SRCS)))
PDEPS:=makefile ../../buildconfig.mk

all: $(DESTDIR)/mode.o


clean:
	rm -f $(BUILDDIR)/*.o
	rm -rdf $(DESTDIR)/*

distclean: clean
	rm -rdf $(BUILDDIR) .$(BUILDDIR).dir
	rm -rdf $(DESTDIR) .$(DESTDIR).dir


.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
	@touch $@



$(DESTDIR)/mode.o: .$(DESTDIR).dir $(OBJS)
	$(LD) -r $(filter %.o,$^) -o $@


$(BUILDDIR)/%.d: .$(BUILDDIR).dir %.c $(PDEPS)
	$(GCC) -MM -MT $(addprefix $(BUILDDIR)/,$*.o) $*.c -o $@

$(BUILDDIR)/%.o: $(BUILDDIR)/%.d $(PDEPS)
	$(GCC) $(CFLAGS) $*.c -o $@

include $(wildcard $(BUILDDIR)/*.d)

