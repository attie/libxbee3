include config.mk
include libconfig.mk
include buildconfig.mk

RELEASE_ITEMS:=$(DESTDIR)/$(LIBOUT).so.$(LIBFULLREV)       \
               $(DESTDIR)/$(LIBOUT).so                     \
               $(DESTDIR)/$(LIBOUT).so.$(LIBFULLREV).dbg   \
               $(DESTDIR)/$(LIBOUT).a.$(LIBFULLREV)        \
               $(DESTDIR)/$(LIBOUT).a                      \
               $(SYS_HEADERS)                              \
               $(RELEASE_FILES)

PDEPS:=makefile config.mk libconfig.mk buildconfig.mk

###############################################################################

.PHONY: all help clean distclean new release .%.dir
.PHONY: install install_dbg install_sudo install_dbg_sudo
.PHONY: $(SYS_MANDIR)/%.gz.symlink
.PHONY: ALWAYS

OBJS:=$(addprefix $(BUILDDIR)/,$(addsuffix .o,$(SRCS)))


all: $(DESTDIR)/$(LIBOUT).so $(DESTDIR)/$(LIBOUT).a

install: all
	@sudo make install_sudo

install_dbg: all
	@sudo make install_dbg_sudo

install_sudo: all \
              $(addprefix $(SYS_INCDIR)/,$(SYS_HEADERS)) \
              $(addprefix $(SYS_MANDIR)/,$(addsuffix .gz,$(SYS_MANPAGES))) \
              $(addprefix $(SYS_MANDIR)/,$(addsuffix .gz.symlink,$(SYS_MANLINKS))) \
              $(SYS_LIBDIR)/$(LIBOUT).so.$(LIBFULLREV) \
              $(SYS_LIBDIR)/$(LIBOUT).a.$(LIBFULLREV) \

install_dbg_sudo: install_sudo $(SYS_LIBDIR)/$(LIBOUT).so.$(LIBFULLREV).dbg

help:
	@echo "usage:"
	@echo "  make [all]        - to simply build $(LIBOUT)"
	@echo "  make clean        - to remote all object files and start again"
	@echo "  make new          - to perform a 'clean', followed by an 'all'"
	@echo "  make distclean    - to remove ALL generated files and directories"
	@echo "  make release      - to make a *.tar.bz2 file containing all files required to make use of $(LIBOUT)"
	@echo "  make install      - to install $(LIBOUT) on this system"
	@echo "  make install_dbg  - to install $(LIBOUT) along with debug information on this system"
	@echo "  make help         - to display this help information"
	@echo ""
	@echo "environment:"
	@echo "  CROSS_COMPILE     - set this to your toolchain's prefix (e.g: arm-none-linux-)"
	@echo "  CFLAGS            - add flags for the compile step of the build"
	@echo "  CLINKS            - add flags for the link step of the build"
	@echo ""
	@echo "other information:"
	@echo "  to modify settings for the build environment and control various aspects of the resulting binary,"
	@echo "  edit 'config.mk'"
	

$(SYS_LIBDIR)/$(LIBOUT).%.$(LIBFULLREV): $(DESTDIR)/$(LIBOUT).%.$(LIBFULLREV)
	install -g $(SYS_GROUP) -o $(SYS_USER) -m 755 -DT $^ $@
	ln -fs $@ $(subst .$(LIBFULLREV),,$@)

$(SYS_LIBDIR)/$(LIBOUT).so.$(LIBFULLREV).dbg: $(DESTDIR)/$(LIBOUT).so.$(LIBFULLREV).dbg
	install -g $(SYS_GROUP) -o $(SYS_USER) -m 755 -DT $^ $@

$(SYS_INCDIR)/%.h: %.h
	install -g $(SYS_GROUP) -o $(SYS_USER) -m 644 -DT $^ $@

$(SYS_MANDIR)/%.gz: $(MANDIR)/%
	$(GZIP) < $^ > $@
	chown $(SYS_USER):$(SYS_GROUP) $@
	chmod 644 $@

$(SYS_MANDIR)/%.gz.symlink: $(MANDIR)/%
	ln -sf $(shell readlink $^).gz $(patsubst %.symlink,%,$@)
	chown -h $(SYS_USER):$(SYS_GROUP) $(patsubst %.symlink,%,$@)


new: clean
	@$(MAKE) all

clean:
	rm -f $(BUILDDIR)/*.o
	rm -rdf $(DESTDIR)/*
	$(MAKE) -C modes clean

distclean:
	rm -rdf $(BUILDDIR) .$(BUILDDIR).dir
	rm -rdf $(DESTDIR) .$(DESTDIR).dir
	$(MAKE) -C modes distclean

tidy:
	rm -f `find . -name '*~'`


release: all
	tar -cjvf $(LIBOUT)_v$(LIBFULLREV)_`date +%Y-%m-%d`_`git rev-parse --verify --short HEAD`_`uname -m`.tar.bz2 --transform='s#^#libxbee/#' $(RELEASE_ITEMS)


.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
	@touch $@


$(DESTDIR)/$(LIBOUT).so: $(DESTDIR)/$(LIBOUT).so.$(LIBFULLREV)
	ln -fs `basename $^` $@

$(DESTDIR)/$(LIBOUT).so.$(LIBFULLREV): .$(DESTDIR).dir $(DESTDIR)/$(LIBOUT).o
	$(GCC) -shared -Wl,-soname,$(LIBOUT).so.$(LIBFULLREV) $(CLINKS) $(filter %.o,$^) -o $@
	$(OBJCOPY) --only-keep-debug $@ $@.dbg
	$(OBJCOPY) --add-gnu-debuglink=$@.dbg $@
	$(OBJCOPY) --strip-debug $@

$(DESTDIR)/$(LIBOUT).a: $(DESTDIR)/$(LIBOUT).a.$(LIBFULLREV)
	ln -fs `basename $^` $@

$(DESTDIR)/$(LIBOUT).a.$(LIBFULLREV): .$(DESTDIR).dir $(DESTDIR)/$(LIBOUT).o
	$(AR) rcs $@ $(filter %.o,$^)

$(DESTDIR)/$(LIBOUT).o: .$(DESTDIR).dir $(OBJS) modes/$(DESTDIR)/_modes.o
	$(LD) -r $(filter %.o,$^) -o $@


modes/$(DESTDIR)/_modes.o: .$(BUILDDIR).dir modes ALWAYS
	$(MAKE) -C modes MODELIST='$(MODELIST)'

$(BUILDDIR)/%.d: .$(BUILDDIR).dir %.c $(PDEPS)
	$(GCC) -MM -MT $(addprefix $(BUILDDIR)/,$*.o) $*.c -o $@

$(BUILDDIR)/ver.o: $(BUILDDIR)/ver.d $(wildcard %.c) $(wildcard %.h) $(PDEPS)
	$(GCC) $(CFLAGS) $(VER_DEFINES) ver.c -o $@

$(BUILDDIR)/mode.o: $(BUILDDIR)/mode.d $(PDEPS)
	$(GCC) $(CFLAGS) -DMODELIST='$(addsuffix $(COMMA),$(addprefix &mode_,$(MODELIST))) NULL' mode.c -o $@

$(BUILDDIR)/%.o: $(BUILDDIR)/%.d $(PDEPS)
	$(GCC) $(CFLAGS) $*.c -o $@

include $(wildcard $(BUILDDIR)/*.d)
