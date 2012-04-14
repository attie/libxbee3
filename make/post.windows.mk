CORE_SRCS:=    $(filter-out net_%,$(CORE_SRCS))

OPTIONS+=              XBEE_NO_NET_SERVER
