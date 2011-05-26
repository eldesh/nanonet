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
#if !defined NETWORK_INCLUDED
#define      NETWORK_INCLUDED

#if defined _WIN32
#  define _CRT_SECURE_NO_WARNINGS
#  include <winsock2.h>
#  include <ws2tcpip.h>
  typedef SOCKET socket_t;
#else
#  include <sys/param.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <netdb.h>

#  include <ctype.h>
#  include <errno.h>
#  include <signal.h>
#  include <sysexits.h>
#  include <unistd.h>
#  include <inttypes.h>
  typedef int    socket_t;
#  define INVALID_SOCKET  (socket_t)(~0)
#endif
#include <stdarg.h>
#include "bool.h"

#if defined _WIN32
  typedef          int   int32_t;
  typedef   signed short int16_t;
  typedef          char  int8_t;
  //typedef char byte;
  typedef unsigned long  uint32_t;
  typedef unsigned short uint16_t;
  typedef unsigned char  uint8_t;
#else
  typedef char byte;
#endif


bool network_init(void); // initialize network subsystem
void network_end(void);  // end network subsystem

int nanonet_error (void); // return error number of last called network system API
char const * nanonet_error_tostring (int err); // to string value of nanonet_error()

struct addrinfo make_hint(int family, int type, int protocol);
// addrinfo ctor from host/port presented by c-string
struct addrinfo * make_addrinfo(char const * host, char const * port);
// set host:port to buffers as strings. return success or failure.
bool setnameinfo(struct sockaddr const * addr, socklen_t len, char * host, char * port);
// get host:port strings with allocated memory. return NULL if getting host:port failed.
char * getnameinfo_as_string(struct sockaddr const * addr, socklen_t len);
// make a listened socket binded to host:port.
socket_t server_socket (char const * host, char const * port);
// make a socket connected to host:port
socket_t client_socket (char const * host, char const * port);
// close socket. return success or failure.(for non Windows systems, return always true)
bool close_socket_t(socket_t soc);
// received characters to a buffer(buff, size),
// call serv() with the received string and buffer-size(byte).
// return success or failure.
bool servline(socket_t sock, char * buff, size_t size, bool (*serv)(socket_t, char const *, size_t));

bool sendall(socket_t sock, char const * buf, size_t len, int flags);

// exchange byte order
uint32_t net_to_host_uint32_t(uint32_t x);
uint16_t net_to_host_uint16_t(uint16_t x);
uint32_t host_to_net_uint32_t(uint32_t x);
uint16_t host_to_net_uint16_t(uint16_t x);

fd_set make_fd_set(void);

socket_t  single_accept(socket_t sock, bool (*serv)(socket_t));
socket_t vsingle_accept(socket_t sock, bool (*serv)(socket_t, va_list), ...);

#endif    /* NETWORK_INCLUDED */

