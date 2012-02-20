### libxbee internal configuration options:
### it is recommended that you DO NOT change this file

LIBMAJ:=        3
LIBMIN:=        0
LIBREV:=        2

LIBOUT:=        libxbee

LIBS:=          rt pthread dl

SRCS:=          ll ver prepare xbee pkt conn error frame mode thread log tx rx \
                xsys

MODELIST:=      xbee1 xbee2

SYS_HEADERS:=   xbee.h
RELEASE_FILES:= HISTORY

VER_DEFINES=    -DLIB_REVISION="\"$(LIBFULLREV)\""                             \
                -DLIB_COMMIT="\"$(shell git log -1 --format="%H")\""           \
                -DLIB_COMMITTER="\"$(shell git log -1 --format="%cn <%ce>")\"" \
                -DLIB_BUILDTIME="\"$(shell date)\""
