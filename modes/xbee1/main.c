#include <stdio.h>
#include <stdlib.h>

#include "../../internal.h"
#include "../../mode.h"

struct xbee_mode mode_xbee1 = {
	.name = "xbee1",
	
	.conTypes = NULL,
	
	.init = NULL,
	.shutdown = NULL,
	
	.rx_io = NULL,
	.tx_io = NULL,
	
	.thread = NULL,
};

