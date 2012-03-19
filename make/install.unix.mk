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
  echo $^; \
	$(SYMLINK) -fs $(shell readlink $^).gz $@; \
fi
	chown -h $(SYS_USER):$(SYS_GROUP) $@

$(SYS_MANDIR)/%:
	@echo $*
