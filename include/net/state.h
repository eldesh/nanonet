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
#if !defined SERVICE_STATE_MACHINE_INCLUDED
#define      SERVICE_STATE_MACHINE_INCLUDED

#include <net/net.h>

typedef enum {
	  ST_VALID       // 
	, ST_TRANSITION  // occur transition go to other state
	, ST_END         // state machine is end
	, ST_INVALID     // invalid state
	, ST_SHORT       // given buffer is too short for parsing
	, ST_NONE
} SERVICE_ST_TYPE;

typedef struct buffer_ {
	int size; // not size_t for using return value of recv
	int used; // used size
	byte * buffer;
} buffer;

typedef struct buffer_slice_ {
	size_t size;
	byte const * buffer;
} buffer_slice;

struct st_service_tuple_;
typedef struct st_service_tuple_ st_service_tuple;

// buffer_slice is better for arg2 ?
typedef st_service_tuple (*service_type)(buffer_slice *);

struct st_service_tuple_ {
	SERVICE_ST_TYPE state;  //
	service_type service;   // if st<>ST_TRANSITION then service is null
};

st_service_tuple make_st_service(SERVICE_ST_TYPE st, service_type serv);
buffer make_buffer(int size);
buffer_slice make_buffer_slice(byte const * buffer, size_t size);
void delete_buffer(buffer * buff);
void copy_buffer(buffer src, buffer * dst);
void copy_slice_to_buffer(buffer_slice src, buffer * dst);
int recv_buffer(socket_t sock, buffer * buff, int flags);
bool state_machine_service(socket_t sock, service_type start_service);

#endif    /* SERVICE_STATE_MACHINE_INCLUDED */
