#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net/net.h>
#define NANONET_LOG_ENABLE
#include <net/log.h>

int random_int(int from, int to) {
	int range = to - from +1;
	assert(from<to);
	return from + rand()%range;
}

socket_t pass_int_array_serv(socket_t soc, int N) {
	if (soc!=INVALID_SOCKET) {
		int i;
		NANOLOG("send int array start!\n");
		for (i=0; i<N; ++i) {
			if (!senduint32(soc, random_int(0, N), 0)) {
				NANOLOG("sendall failure\n");
				return false;
			}
		}
		NANOLOG("send int array complete!\n");
		return true;
	}
	return soc;
}

int main (int argc, char * argv[]) {
	nanonet_log_type = NANONET_LOG_STDERR;
	int const N = 1<argc ? atoi(argv[1]) : 10;
	NANOLOG("start\n");
	close_socket_t
	  (pass_int_array_serv
	      (client_socket
		    ("localhost", "5555")
		, N));
	NANOLOG("end..\n");
	return 0;
}


