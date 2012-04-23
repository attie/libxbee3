# get hold of the build system
include make/buildsys.mk

# the configuration can be found in config.mk after running `make configure`

all: $(LIB_OUT)

configure: config.mk
	@true

new: clean
	@$(MAKE) all

clean:
	$(RM) $(BUILDDIR)/*.o
	$(RM) $(BUILDDIR)/*.d
	$(RM) $(HTMLDIR)/*/*.html

distclean: clean
	$(RM) $(patsubst %,.%.dir,$(CONSTRUCTIONDIRS))
	$(RMDIR) $(CONSTRUCTIONDIRS)

# get hold of the build rules
include $(firstword $(BUILD_RULES))

# get hold of the install rules if there are any
ifneq ($(INSTALL_RULES),)
include $(firstword $(INSTALL_RULES))
endif
install: $(INSTALL_FILES)

uninstall:
	rm -rf $(INSTALL_FILES)
