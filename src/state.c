/*
 * Copyright (c) 2011, eldesh (nephits@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FREEBSD PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <net/state.h>
#include <net/log.h>

st_service_tuple make_st_service(SERVICE_ST_TYPE st, service_type serv) {
	st_service_tuple sts;
	sts.state = st;
	sts.service = serv;
	return sts;
}

static struct timeval timeval_from_sec(int sec) {
	struct timeval time;
	time.tv_sec = sec;
	time.tv_usec = 0;
	return time;
}

bool state_machine_service(socket_t sock, service_type start_service
						                , void * ctx
										, void (*ctx_dtor)(void *))
{
	buffer buff = make_buffer(0);
	service_type service = start_service;
	buffer_slice bs = make_buffer_slice(buff.buffer, buff.used);
	st_service_tuple st = make_st_service(ST_INVALID, NULL);
	if (sock==INVALID_SOCKET)
		return false;
	while (1) {
		size_t const req_buffer_size = 512;
		int len;
		/* expand buffer */
		// oldbuff
		// [                         allocated                    ]
		// [        received                     |  indeterminate ]
		// [ used(consumed by service) | nonused |                ]
		// 
		// newbuff
		// [                         oldbuffer-size               | nonused ]
		// [                         allocated                              ]
		// [ nonused(copied from oldbuf) |                                  ] <- expanded buffer!
		// 
		buffer newbuff = make_buffer(req_buffer_size+bs.size);
		copy_slice_to_buffer(bs, &newbuff);
		delete_buffer(&buff);
		buff = newbuff;

		if (st.state==ST_SHORT || buff.used==0)
			len=recv_buffer(sock, &buff, 0);
		else if (st.state==ST_TRANSITION)
			len=recv_buffer_timeout(sock, &buff, 0, timeval_from_sec(1));

		if (len==0) { // connection have been gracefully closed
			break;
		} else if (len==SOCKET_ERROR) {
			NANOLOG("recv failed <%s>\n", nanonet_error_tostring(nanonet_error()));
			break;
		} else {
			bs = make_buffer_slice(buff.buffer, buff.used);
			st = service(&ctx, &bs);
			if (st.state==ST_SHORT) {	// not enough inputted sequence
				bs = make_buffer_slice(buff.buffer, buff.used);
			} else if (st.state==ST_TRANSITION) {
				service = st.service;
			} else if (st.state==ST_END) {
				break;
			} else if (st.state==ST_INVALID) {
				NANOLOG("invalid state\n");
				break;
			} else {
				NANOLOG("state machine occured fatal error with transition to unknown state XD\n");
				assert(false);
			}
		}
	}
	(ctx_dtor ? ctx_dtor : free)(ctx);
	delete_buffer(&buff);
	return false;
}


