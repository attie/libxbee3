CORE_SRCS:=    $(filter-out net_%,$(CORE_SRCS))

MODELIST:=     $(filter-out net,$(MODELIST))

OPTIONS+=              XBEE_NO_NET_SERVER
