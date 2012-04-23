### build rules follow... ###

# generate the symlinks (libxbee.so -> libxbee.so.x.x.x)
$(DESTDIR)/$(LIBNAME).a $(DESTDIR)/$(LIBNAME).so: $(DESTDIR)/$(LIBNAME).%: $(DESTDIR)/$(LIBNAME).%.$(LIBFULLREV)
	$(SYMLINK) -fs `basename $^` $@

# generate the shared object & debug file
$(DESTDIR)/$(LIBNAME).so.$(LIBFULLREV): .$(DESTDIR).dir $(DESTDIR)/$(LIBNAME).o
	$(GCC) -shared -Wl,-soname,$(LIBNAME).so.$(LIBFULLREV) $(CLINKS) $(filter %.o,$^) -o $@
	$(OBJCOPY) --only-keep-debug $@ $@.dbg
	$(OBJCOPY) --add-gnu-debuglink=$@.dbg $@
	$(OBJCOPY) --strip-debug $@

# generate the static library
$(DESTDIR)/$(LIBNAME).a.$(LIBFULLREV): .$(DESTDIR).dir $(DESTDIR)/$(LIBNAME).o
	$(AR) rcs $@ $(filter %.o,$^)

# generate the 'libxbee' object file
$(DESTDIR)/$(LIBNAME).o: .$(DESTDIR).dir $(addprefix $(BUILDDIR)/,__core.o __mode.o $(foreach mode,$(MODELIST),__mode_$(mode).o))
	$(LD) -r -o $@ $(filter %.o,$^)

$(BUILDDIR)/__core.o: $(CORE_OBJS)
$(BUILDDIR)/__mode.o: $(MODE_OBJS)
$(BUILDDIR)/__%.o:
	$(LD) -r -o $@ $(filter %.o,$^)

###
# dynamically generate these rules for each mode
define mode_rule
# build a mode's object & dep file
.PRECIOUS: $$(BUILDDIR)/modes_$1_%.d
$$(BUILDDIR)/__mode_$1.o: .$$(BUILDDIR).dir $(MODE_$1_OBJS)
	$$(LD) -r -o $$@ $$(filter %.o,$$^)
$$(BUILDDIR)/modes_$1_%.o: .$$(BUILDDIR).dir  $$(BUILDDIR)/modes_$1_%.d
	$$(GCC) $$(CFLAGS) modes/$1/$$*.c -c -o $$@
$$(BUILDDIR)/modes_$1_%.d: .$$(BUILDDIR).dir modes/$1/%.c
	$$(GCC) -MM -MT $$(@:.d=.o) $$(filter %.c,$$^) -o $$@
endef
$(foreach mode,$(MODELIST),$(eval $(call mode_rule,$(mode))))
#####

# build the common mode code
$(BUILDDIR)/modes_%.o: .$(BUILDDIR).dir $(BUILDDIR)/modes_%.d
	$(GCC) $(CFLAGS) modes/$*.c -c -o $@
$(BUILDDIR)/modes_%.d: .$(BUILDDIR).dir modes/%.c
	$(GCC) -MM -MT $(@:.d=.o) $(filter %.c,$^) -o $@

###
# these objects require special treatment
$(BUILDDIR)/ver.o: $(BUILDDIR)/%.o: .$(BUILDDIR).dir $(BUILDDIR)/%.d
	$(GCC) $(CFLAGS) $(VER_DEFINES) $*.c -c -o $@
$(BUILDDIR)/mode.o: $(BUILDDIR)/%.o: .$(BUILDDIR).dir $(BUILDDIR)/%.d
	$(GCC) $(CFLAGS) -DMODELIST='$(addsuffix $(COMMA),$(addprefix &mode_,$(MODELIST))) NULL' $*.c -c -o $@
#####

# build a core object & dep file
$(BUILDDIR)/%.o: .$(BUILDDIR).dir $(BUILDDIR)/%.d
	$(GCC) $(CFLAGS) $*.c -c -o $@
$(BUILDDIR)/%.d: .$(BUILDDIR).dir %.c
	$(GCC) -MM -MT $(@:.d=.o) $(filter %.c,$^) -o $@

# include all the dep files avaliable
-include $(wildcard $(BUILDDIR)/*.d)

ifneq ($(MAN2HTML),)
# generate HTML from man pages
$(addprefix $(HTMLDIR)/,$(filter-out index.html,$(SYS_HTMLPAGES))): $(HTMLDIR)/%.html: $(MANDIR)/%
	@echo "cat $^ | $(MAN2HTML) -rp | tail -n +3 > $@"
	@mkdir -p `dirname $@`
	@if [ ! -h $^ ]; then                        \
	cat $^ | $(MAN2HTML) -rp | tail -n +3 > $@;  \
  chmod 644 $@;                                \
else                                           \
	$(SYMLINK) -fs $(shell readlink $^).html $@; \
fi
$(HTMLDIR)/index.html: $(filter %libxbee.3.html,$(HTMLDIR)/$(SYS_HTMLPAGES))
	$(SYMLINK) -fs $(subst $(HTMLDIR)/,,$^) $@
endif
