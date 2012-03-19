### detects the current OS and does some configuration

# detect linux
ifeq ($(shell uname),Linux)
include make/os.linux.mk

# detect freebsd
else ifeq ($(shell uname),FreeBSD)
include make/os.freebsd.mk

# detect windows
else ifeq ($(OS),Windows_NT)
include make/os.windows.mk

# detect error
else
$(error Unknown OS, please see make/os_detect.mk)
endif

# some twisting
DEFCONFIG?=generic.mk
DEFCONFIG:=$(addprefix make/default/,$(firstword $(DEFCONFIG)))

BUILD_RULES?=generic.mk
BUILD_RULES:=$(addprefix make/build.,$(firstword $(BUILD_RULES)))

ifneq ($(INSTALL_RULES),)
INSTALL_RULES:=$(addprefix make/install.,$(firstword $(INSTALL_RULES)))
endif
