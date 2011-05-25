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


#if !defined _WIN32
static int WSAGetLastError(void) {
	return errno;
}
#endif


st_service_tuple make_st_service(SERVICE_ST_TYPE st, service_type serv) {
	st_service_tuple sts;
	sts.state = st;
	sts.service = serv;
	return sts;
}

buffer make_buffer(int size) {
	buffer buff;
	buff.used = 0;
	buff.size = size;
	if (size==0)
		buff.buffer = NULL;
	else if (size<0)
		buff.buffer = NULL;
	else
		buff.buffer = (byte*)calloc(size, sizeof(byte));
	return buff;
}

buffer_slice make_buffer_slice(byte const * buffer, size_t size) {
	buffer_slice bs;
	bs.buffer = buffer;
	bs.size   = size;
	return bs;
}

void delete_buffer(buffer * buff) {
	if (buff) {
		buff->size = 0;
		buff->used = 0;
		free(buff->buffer);
		buff->buffer = NULL;
	}
}

void copy_buffer(buffer src, buffer * dst) {
	int i;
	for (i=0; i<src.used; ++i)
		dst->buffer[dst->used+i] = src.buffer[i];
	dst->used += src.used;
}
void copy_slice_to_buffer(buffer_slice src, buffer * dst) {
	size_t i;
	for (i=0; i<src.size; ++i)
		dst->buffer[dst->used+i] = src.buffer[i];
	dst->used += src.size;
}

int recv_buffer(socket_t sock, buffer * buff, int flags) {
	int len=recv(sock, &buff->buffer[buff->used], buff->size-buff->used, flags);
	if (0<len)
		buff->used += len;
	return len;
}

bool state_machine_service(socket_t sock, service_type start_service
						                , void * ctx
										, void (*ctx_dtor)(void *))
{
	buffer buff = make_buffer(0);
	service_type service = start_service;
	buffer_slice bs = make_buffer_slice(buff.buffer, buff.used);
	if (sock==INVALID_SOCKET)
		return false;
	while (1) {
		size_t const req_buffer_size = 512;
		int len;
		/* expand buffer */
		// oldbuff
		// [                         allocated                         ]
		// [        received                     |  indeterminate      ]
		// [ used(consumed by service) | nonused |                     ]
		// 
		// newbuff
		// [                         oldbuffer-size                    | nonused ]
		// [                         allocated                                   ]
		// [ nonused(copied from oldbuf) |                                       ] <- expanded buffer!
		// 
		buffer newbuff = make_buffer(req_buffer_size+bs.size);
		copy_slice_to_buffer(bs, &newbuff);
		delete_buffer(&buff);
		buff = newbuff;

		len=recv_buffer(sock, &buff, 0);
		if (len==0) { // connection have been gracefully closed
			break;
		} else if (len<0) {
			fprintf(stderr, "recv failed <%d>\n", WSAGetLastError());
			break;
		} else {
			st_service_tuple st;
			bs = make_buffer_slice(buff.buffer, buff.used);
			st = service(&ctx, &bs);
			if (st.state==ST_SHORT) {	// not enough inputted sequence
				bs = make_buffer_slice(buff.buffer, buff.used);
			} else if (st.state==ST_TRANSITION) {
				service = st.service;
			} else if (st.state==ST_END) {
				break;
			} else if (st.state==ST_INVALID) {
				fprintf(stderr, "invalid state\n");
				break;
			} else {
				fprintf(stderr, "state machine occured fatal error with transition to unknown state XD\n");
				assert(false);
			}
		}
	}
	(ctx_dtor ? ctx_dtor : free)(ctx);
	delete_buffer(&buff);
	return false;
}


