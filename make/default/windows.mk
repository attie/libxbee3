### libxbee configuration options:

### compiler locations
VCPATH:=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC
SDKPATH:=C:\Program Files\Microsoft SDKs\Windows\v6.0A

### using this can create a smaller binary, by removing modes you won't use
#MODELIST:=             xbee1 xbee2 xbee5

### debug options
#DEBUG:=/Zi /MTd
#LDBUG:=/DEBUG

### un-comment to remove ALL logging (smaller & faster binary)
#OPTIONS+=       XBEE_DISABLE_LOGGING

### use for more precise logging options
OPTIONS+=       XBEE_LOG_NO_COLOR
#OPTIONS+=       XBEE_LOG_LEVEL=100
#OPTIONS+=       XBEE_LOG_RX
#OPTIONS+=       XBEE_LOG_TX

### un-comment to disable strict objects (xbee/con/pkt pointers are usually checked inside functions)
### this may give increased execution speed, but will be more suseptible to incorrect parameters
#OPTIONS+=       XBEE_DISABLE_STRICT_OBJECTS

### un-comment to turn off hardware flow control
#OPTIONS+=       XBEE_NO_RTSCTS

### un-comment to use API mode 2
#OPTIONS+=       XBEE_API2
#OPTIONS+=       XBEE_API2_DEBUG
#OPTIONS+=       XBEE_API2_IGNORE_CHKSUM
#OPTIONS+=       XBEE_API2_SAFE_ESCAPE

### useful for debugging the core of libxbee
#OPTIONS+=       XBEE_NO_FINI
