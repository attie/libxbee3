### libxbee configuration options:

### system install directories
SYS_LIBDIR:=    /usr/lib
SYS_INCDIR:=    /usr/include

### setup a cross-compile toolchain (either here, or in the environment)
#CROSS_COMPILE?= 
#CFLAGS+=        
#CLINKS+=        

### un-comment to remove ALL logging (smaller & faster binary)
#OPTIONS+=       XBEE_DISABLE_LOGGING

### un-comment to remove network server functionality
#OPTIONS+=       XBEE_NO_NET
#OPTIONS+=       XBEE_NO_NET_CLIENT
#OPTIONS+=       XBEE_NO_NET_SERVER
#OPTIONS+=       XBEE_NO_NET_STRICT_VERSIONS

### un-comment to turn off hardware flow control
#OPTIONS+=       XBEE_NO_RTSCTS
