### libxbee internal configuration options:
### it is recommended that you DO NOT change this file

LIBMAJ:=        3
LIBMIN:=        0
LIBREV:=        5

LIBOUT:=        libxbee

LIBS:=          rt pthread

SRCS:=          ll ver prepare xbee pkt conn error frame mode thread log tx rx \
                xsys net net_io net_handlers net_callbacks mutex

MODELIST:=      xbee1 xbee2 net

SYS_HEADERS=    xbee.h
SYS_MANPAGES=   $(shell find $(MANDIR) -type f -name '*.3')
SYS_MANLINKS=   $(shell find $(MANDIR) -type l -name '*.3')

RELEASE_FILES=  HISTORY LICENSE README
INSTALL_FILES=  $(addprefix $(SYS_INCDIR)/,$(SYS_HEADERS)) \
                $(addprefix $(SYS_MANDIR)/,$(addsuffix .gz,$(SYS_MANPAGES))) \
                $(addprefix $(SYS_MANDIR)/,$(addsuffix .gz,$(SYS_MANLINKS))) \
                $(SYS_LIBDIR)/$(LIBOUT).so.$(LIBFULLREV) \
                $(SYS_LIBDIR)/$(LIBOUT).a.$(LIBFULLREV)

VER_DEFINES=    -DLIB_REVISION="\"$(LIBFULLREV)\""                             \
                -DLIB_COMMIT="\"$(shell git log -1 --format="%H")\""           \
                -DLIB_COMMITTER="\"$(shell git log -1 --format="%cn <%ce>")\"" \
                -DLIB_BUILDTIME="\"$(shell date)\""
