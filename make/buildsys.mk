### the main build system

include make/libconfig.mk
include make/os_detect.mk

ifeq ($(wildcard config.mk),)
ifeq ($(filter configure,$(MAKECMDGOALS)),)
$(error please run `make configure` first)
endif
endif

-include config.mk

COMMA:=                ,

.PRECIOUS: .%.dir $(BUILDDIR)/%.d
.PHONY:    all configure

config.mk:| $(DEFCONFIG)
	cp $(firstword $|) $@

.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; else echo "!mkdir $*"; fi
	@touch $@

CORE_SRCS:=$(wildcard *.c)
CORE_SRCS:=$(filter-out xsys_%.c,$(CORE_SRCS))
CORE_OBJS:=$(addprefix $(BUILDDIR)/,$(CORE_SRCS:.c=.o))

MODE_SRCS:=$(wildcard modes/*.c)
MODE_OBJS:=$(addprefix $(BUILDDIR)/,$(subst /,_,$(MODE_SRCS:.c=.o)))

MODE_MODE_SRCS:=
MODE_MODE_OBJS:=
define mode_srcs
MODE_$1_SRCS:=$$(wildcard modes/$1/*.c)
MODE_$1_OBJS:=$$(addprefix $$(BUILDDIR)/,$$(subst /,_,$$(MODE_$1_SRCS:.c=.o)))
MODE_MODE_SRCS+=$$(MODE_$1_SRCS)
MODE_MODE_OBJS+=$$(MODE_$1_OBJS)
endef
$(foreach mode,$(MODELIST),$(eval $(call mode_srcs,$(mode))))

MAN_DIRS:=
define man_dirs
MAN_DIRS+=$$(filter $$(MANDIR)/man%,$1)
endef
$(foreach dir,$(wildcard $(MANDIR)/*),$(eval $(call man_dirs,$(dir))))

SYS_MANPAGES:=
define man_srcs
SYS_MANPAGES+=$$(wildcard $(MANDIR)/man$(patsubst $(MANDIR)/man%,%,$1)/*.$(patsubst $(MANDIR)/man%,%,$1))
endef
$(foreach dir,$(MAN_DIRS),$(eval $(call man_srcs,$(dir))))
SYS_MANPAGES:=$(patsubst $(MANDIR)/%,%,$(SYS_MANPAGES))

$(CORE_OBJS) $(MODE_OBJS) $(MODE_MODE_OBJS): config.mk

.DEFAULT_GOAL:=
