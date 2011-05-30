#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NANONET_LOG_ENABLE
#include <net/log.h>

int main () {
	NANOLOG("nanonet test starting !\n");
	nanonet_log_type = NANONET_LOG_STDOUT;
	NANOLOG("message to stdout\n");
	nanonet_log_type = NANONET_LOG_STDERR;
	NANOLOG("message to stderr\n");
	nanonet_log_type = NANONET_LOG_DEBUGVIEW;
	NANOLOG("message to debugview\n");
	nanonet_log_type = NANONET_LOG_OFF;
	NANOLOG("message to space...?\n");
	nanonet_log_type = NANONET_LOG_DEBUGVIEW;
	NANOLOG("nanonet test complete!\n");
	return 0;
}

