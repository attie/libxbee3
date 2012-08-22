### build rules follow... ###

# generate the DLL
$(DESTDIR)/$(LIBNAME)$(LIBMAJ).dll: .$(DESTDIR).dir $(CORE_OBJS) $(MODE_OBJS) $(MODE_MODE_OBJS) $(BUILDDIR)/win32.res
	$(LD) $(CLINKS) $(FINLNK) "/LIBPATH:$(SDKPATH)Lib" "/LIBPATH:$(VCPATH)\lib" /OUT:$@ /MAP:$@.map $(filter %.o,$^) $(filter %.res,$^)

$(BUILDDIR)/win32.res: xsys_win32.rc
	$(RC) "/I$(SDKPATH)\Include" "/I$(VCPATH)\include" /n /fo$@ $^

###
# dynamically generate these rules for each mode
define mode_rule
# build a mode's object
$$(BUILDDIR)/modes_$1_%.o: .$$(BUILDDIR).dir modes/$1/%.c
	$$(CC) $$(CFLAGS) modes/$1/$$*.c /c /Fo$$@
endef
$(foreach mode,$(MODELIST),$(eval $(call mode_rule,$(mode))))
#####

# build the common mode code
$(BUILDDIR)/modes_%.o: .$(BUILDDIR).dir modes/%.c
	$(CC) $(CFLAGS) modes/$*.c /c /Fo$@

###
# these objects require special treatment
$(BUILDDIR)/ver.o: $(BUILDDIR)/%.o: .$(BUILDDIR).dir %.c
	$(CC) $(CFLAGS) $(VER_DEFINES) $*.c /c /Fo$@
$(BUILDDIR)/mode.o: $(BUILDDIR)/%.o: .$(BUILDDIR).dir %.c
	$(CC) $(CFLAGS) /DMODELIST="$(addsuffix $(COMMA),$(addprefix &mode_,$(MODELIST))) NULL" $*.c /c /Fo$@
#####

# build a core object
$(BUILDDIR)/xsys.o: xsys_win32.c
$(BUILDDIR)/%.o: .$(BUILDDIR).dir %.c
	$(CC) $(CFLAGS) $*.c /c /Fo$@
