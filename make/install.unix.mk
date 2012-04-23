### install rules follow... ###

$(SYS_LIBDIR)/$(LIBNAME).so.$(LIBFULLREV).dbg: $(DESTDIR)/$(LIBNAME).so.$(LIBFULLREV).dbg
	$(INSTALL) -m 755 $^ $@

$(SYS_LIBDIR)/$(LIBNAME).%.$(LIBFULLREV): $(DESTDIR)/$(LIBNAME).%.$(LIBFULLREV)
	$(INSTALL) -m 755 $^ $@

$(SYS_LIBDIR)/$(LIBNAME).%: $(SYS_LIBDIR)/$(LIBNAME).%.$(LIBFULLREV)
	$(SYMLINK) -fs $^ $@

$(addprefix $(SYS_INCDIR)/,$(SYS_HEADERS)): $(SYS_INCDIR)/%: %
	$(INSTALL) -m 644 $^ $@

$(addprefix $(SYS_MANDIR)/,$(addsuffix .gz,$(SYS_MANPAGES))): $(SYS_MANDIR)/%.gz: $(MANDIR)/%
	@echo $(INSTALL) -m 644 $^ $@
	@if [ ! -h $^ ]; then                      \
  $(DEFLATE) < $^ > $@;                      \
  chmod 644 $@;                              \
else                                         \
	$(SYMLINK) -fs $(shell readlink $^).gz $@; \
fi
	chown -h $(SYS_USER):$(SYS_GROUP) $@


### release follows... ###

release: $(RELEASE_FILES)
	tar -acf $(LIBNAME)_v$(LIBFULLREV)_`date +%Y-%m-%d`_`git rev-parse --verify --short HEAD`_`uname -m`_`uname -s`.tar.bz2 --transform='s#^#$(LIBNAME)_$(LIBFULLREV)/#SH' $^
