#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net/net.h>
#define NANONET_LOG_ENABLE
#include <net/log.h>

bool recv_int_array_serv(socket_t soc) {
	if (soc==INVALID_SOCKET)
		return false;
	{
		int i;
		for (i=0; ; ++i) {
			uint32_t v;
			int r=recvuint32(soc, &v, 0);
			if (r==0)
				return true;
			if (r<0) {
				NANOLOG("recv failure\n");
				return false;
			}
			printf("recved <%d>\n", v);
			if (i%5==0)
				NANOLOG("received %d integers!\n", i);
		}
		return true;
	}
}

int main () {
	nanonet_log_type = NANONET_LOG_DEBUGVIEW;
	NANOLOG("start\n");
	close_socket_t
	  (single_accept
	      (server_socket
		    ("localhost", "5555")
		, recv_int_array_serv));
	NANOLOG("end..\n");
	return 0;
}


