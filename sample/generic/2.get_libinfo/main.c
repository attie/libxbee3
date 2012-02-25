#include <stdio.h>
#include <stdlib.h>

#include <xbee.h>

int main(void) {

	printf("Libxbee revision: %s\n", libxbee_revision);
	printf("Libxbee commit: %s\n", libxbee_commit);
	printf("Libxbee committer: %s\n", libxbee_committer);
	printf("Libxbee build time: %s\n", libxbee_buildtime);

	return 0;
}
