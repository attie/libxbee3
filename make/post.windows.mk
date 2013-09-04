CORE_SRCS:=    $(filter-out net_%,$(CORE_SRCS))

MODELIST:=     $(filter-out net,$(MODELIST))

OPTIONS+=      XBEE_NO_NET_SERVER

ifneq ($(GITPATH),)
GIT_COMMIT:=           $(shell $(GITPATH) log -1 --format='%H')
GIT_COMMITTER:=        $(shell $(GITPATH) log -1 --format='%cn (%ce)')
else
GIT_COMMIT:=           -- unknown --
GIT_COMMITTER:=        -- unknown --
endif

VER_DEFINES=          /DLIB_REVISION="\"$(LIBFULLREV)\""                      \
                      /DLIB_COMMIT="\"$(GIT_COMMIT)\""                        \
                      /DLIB_COMMITTER="\"$(GIT_COMMITTER)\""                  \
                      /DLIB_BUILDTIME="\"$(shell date /T) $(shell time /T)\""
