### libxbee configuration options:

### system install directories
SYS_ROOT?=      
SYS_LIBDIR:=    $(SYS_ROOT)/usr/lib
SYS_INCDIR:=    $(SYS_ROOT)/usr/include
SYS_MANDIR:=    $(SYS_ROOT)/usr/share/man
SYS_GROUP:=     root
SYS_USER:=      root

### using this can create a smaller binary, by removing modes you won't use
#MODELIST:=      xbee1 xbee2 xbee3 xbee5 xbee6b xbeeZB net debug

### to use the 'install_html' rule, you must specify where to install the files to
#SYS_HTMLDIR:=   /var/www/libxbee.doc

### setup a cross-compile toolchain (either here, or in the environment)
# use the CodeSourcery 2011.03-41 toolchain (v4.5.2)
CROSS_COMPILE?= arm-linux-gnueabihf-
#CFLAGS+=
#CLINKS+=

### un-comment to remove ALL logging (smaller & faster binary)
#OPTIONS+=       XBEE_DISABLE_LOGGING

### use for more precise logging options
#OPTIONS+=	XBEE_2_STOP_BITS
#OPTIONS+=       XBEE_LOG_LEVEL=100
#OPTIONS+=       XBEE_LOG_NO_COLOR
#OPTIONS+=       XBEE_LOG_NO_RX
#OPTIONS+=       XBEE_LOG_NO_TX
#OPTIONS+=       XBEE_LOG_RX_DEFAULT_OFF
#OPTIONS+=       XBEE_LOG_TX_DEFAULT_OFF

### un-comment to disable strict objects (xbee/con/pkt pointers are usually checked inside functions)
### this may give increased execution speed, but will be more suseptible to incorrect parameters
#OPTIONS+=       XBEE_DISABLE_STRICT_OBJECTS

### un-comment to remove network server functionality
#OPTIONS+=       XBEE_NO_NET_SERVER
#OPTIONS+=       XBEE_NO_NET_STRICT_VERSIONS

### un-comment to turn off hardware flow control
#OPTIONS+=       XBEE_NO_RTSCTS

### un-comment to allow arbitrary baud rates - drivers may still reject them
#OPTIONS+=       XBEE_ALLOW_ARB_BAUD

### un-comment to use API mode 2
#OPTIONS+=       XBEE_API2
#OPTIONS+=       XBEE_API2_DEBUG
#OPTIONS+=       XBEE_API2_IGNORE_CHKSUM
#OPTIONS+=       XBEE_API2_SAFE_ESCAPE

### useful for debugging the core of libxbee
#OPTIONS+=       XBEE_NO_FINI

### manually start and end the library's global state (be careful!)
#OPTIONS+=       XBEE_MANUAL_INIT
#OPTIONS+=       XBEE_MANUAL_FINI
