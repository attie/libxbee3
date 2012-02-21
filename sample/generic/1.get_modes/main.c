#include <stdio.h>
#include <stdlib.h>

#include <xbee.h>

int main(void) {
	char **modes;
	int i;

	if (xbee_modeGetList(&modes) != XBEE_ENONE) {
		printf("xbee_modeGetList() returned an error...\n");
		return 1;
	}

	for (i = 0; modes[i]; i++) {
		printf("mode %d - %s\n", i, modes[i]);
	}

	free(modes);

	return 0;
}
