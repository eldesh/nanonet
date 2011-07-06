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
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <net/net.h>
#include <net/log.h>

#if defined _WIN32
#  include <io.h>
#  include <Mswsock.h>
#elif defined __linux__
#  include <sys/sendfile.h>
#endif

char const * nanonet_error_tostring (int err) {
	return strerror(err);
}

int nanonet_error (void) {
	return errno;
}

bool network_init(void) {
#if defined _WIN32
	WSADATA wsaData;
	int r=WSAStartup(MAKEWORD(2,0), &wsaData);
	if (r) {
		NANOLOG("%s failure <%s>\n", __FUNCTION__, nanonet_error_tostring(r));
		return false;
	} else
		return true;
#else
	return true;
#endif
}

void network_end(void) {
#if defined _WIN32
	WSACleanup();
#endif
}

struct addrinfo make_hint(int family, int type, int protocol) {
	struct addrinfo hint;
#if defined _WIN32
	ZeroMemory(&hint, sizeof(hint));
#else
	memset(&hint, 0, sizeof(hint));
#endif
	hint.ai_family   = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;
	return hint;
}

struct addrinfo * make_addrinfo(char const * host, char const * port) {
	struct addrinfo * res=NULL;
	struct addrinfo hint=make_hint(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
	int err=getaddrinfo(host, port, &hint, &res);
	if (err) {
		NANOLOG("getaddrinfo failed <%d>\n", nanonet_error());
		return NULL;
	}
	return res;
}

bool setnameinfo(struct sockaddr const * addr, socklen_t len, char * host, char * port) {
	if (getnameinfo(addr, len
		, host, NI_MAXHOST
		, port, NI_MAXSERV
		, NI_NUMERICHOST | NI_NUMERICSERV))
	{
		NANOLOG("getnameinfo failed <%d>\n", nanonet_error());
		return false;
	} else
		return true;
}

char * getnameinfo_as_string(struct sockaddr const * addr, socklen_t len) {
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	if (setnameinfo(addr, len, host, port)) {
		char * info=(char*)calloc(NI_MAXHOST+NI_MAXSERV, sizeof(char));
		strcpy(info, host);
		strcat(info, ":");
		strcat(info, port);
		return info;
	} else
		return NULL;
}


bool close_socket_t(socket_t soc) {
#if defined _WIN32
	return closesocket(soc)==0;
#else
	if (0<=soc)
		close(soc);
	return true;
#endif
}

socket_t server_socket (char const * host, char const * port) {
	socket_t sock=INVALID_SOCKET;
	struct addrinfo * info=make_addrinfo(host, port);
	if (info) {
		char hostname[NI_MAXHOST];
		char portname[NI_MAXSERV];
		if (!setnameinfo(info->ai_addr, info->ai_addrlen, hostname, portname))
			goto END;
		NANOLOG("server_socket [%s:%s]\n", hostname, portname);
		sock=socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (sock==INVALID_SOCKET) {
			NANOLOG("socket failed <%d>\n", nanonet_error());
			goto END;
		}
		{
			char const enable  = '1';
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))==SOCKET_ERROR) {
				NANOLOG("setsockopt <%s>\n", nanonet_error_tostring(nanonet_error()));
				close_socket_t(sock);
				sock=INVALID_SOCKET;
				goto END;
			}
		}
		if (bind(sock, info->ai_addr, info->ai_addrlen)) {
			NANOLOG("bind failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
		if (listen(sock, SOMAXCONN)) {
			NANOLOG("listen failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
	} else
		NANOLOG("make_addrinfo failed <%d>\n", nanonet_error());
END:
	freeaddrinfo(info);
	return sock;
}

socket_t client_socket (char const * host, char const * port) {
	socket_t sock=INVALID_SOCKET;
	struct addrinfo * info=make_addrinfo(host, port);
	if (info) {
		char hostname[NI_MAXHOST];
		char portname[NI_MAXSERV];
		if (!setnameinfo(info->ai_addr, info->ai_addrlen, hostname, portname))
			goto END;
		NANOLOG("[%s:%s]\n", hostname, portname);
		sock=socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (sock==INVALID_SOCKET) {
			NANOLOG("socket failed <%d>\n", nanonet_error());
			goto END;
		}
		if (connect(sock, info->ai_addr, info->ai_addrlen)==INVALID_SOCKET) {
			NANOLOG("connect failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
	} else
		NANOLOG("make_addrinfo failed <%d>\n", nanonet_error());
END:
	freeaddrinfo(info);
	return sock;
}

bool servline(socket_t sock, char * buff, size_t size, bool (*serv)(socket_t, char const *, size_t)) {
	if (sock==INVALID_SOCKET)
		return false;
	{
		int len=recv(sock, buff, size, 0);
		if (len==0) {
			// connection have been gracefully closed
			return false;
		} else if (len<0) {
			NANOLOG("recv failed <%d>\n", nanonet_error());
			return false;
		} else {
			buff[len]='\0';
			return serv(sock, buff, size);
		}
	}
}

fd_set make_fd_set(void) {
	fd_set fds;
	FD_ZERO(&fds);
	return fds;
}

bool sendall(socket_t sock, char const * buf, size_t len, int flags) {
	size_t acc=0;
	do {
		int r=send(sock, buf+acc, len-acc, flags);
		if (r==INVALID_SOCKET) {
			NANOLOG("sendall error\n");
			return false;
		}
		acc += r;
	} while (acc<len);
	return true;
}

bool senduint32(socket_t sock, uint32_t val, int flags) {
	char intbuff[4];
	*((int32_t*)intbuff) = host_to_net_uint32_t(val);
//	for (i=0; i<4; ++i)
//		dst[i] = (byte)(val >> (24-8*i));
	return sendall(sock, intbuff, 4, flags);
}

#if defined __linux__
static int sendfile_generic(socket_t sock, int fd, off_t * offset, size_t count) {
	char * buff=(char*)calloc(count, sizeof(char));
	int sum=0;
	do {
		ssize_t r;
		if (offset)		r= read(fd, buff, count-sum);
		else			r=pread(fd, buff, count-sum, *offset +sum);
		if (r==-1) {
			NANOLOG("failed <%s>\n", nanonet_error_tostring(nanonet_error()));
			sum=INVALID_SOCKET;
			break;
		}
		if (r==0) // read gracefuly finished
			break;
		sum += r;
		if (offset)
			*offset = *offset + r;
		if (sendall(sock, buff, r, 0)==INVALID_SOCKET) {
			NANOLOG("failed <%s>\n", nanonet_error_tostring(nanonet_error()));
			sum=INVALID_SOCKET;
			break;
		}
	} while (sum<count);
	free(buff);
	return sum;
}
#endif // __linux__

bool send_file(socket_t sock, FILE * fp, size_t size, int flags) {
#if defined _WIN32
	HANDLE h = (HANDLE)_get_osfhandle(_fileno(fp));
	if (!TransmitFile(sock, h, size, 0, NULL, NULL, 0)) {
#elif defined  __linux__
	int fd = fileno(fp);
	if (sendfile(sock, fd, NULL, size)==INVALID_SOCKET) {
#else
	if (sendfile_generic(sock, fd, NULL, size)==INVALID_SOCKET) {
#endif
		NANOLOG("failed <%s>\n", nanonet_error_tostring(nanonet_error()));
		return false;
	}
	return true;
}

uint32_t net_to_host_uint32_t(uint32_t x) {
	return ntohl(x);
}
uint16_t net_to_host_uint16_t(uint16_t x) {
	return ntohs(x);
}
uint32_t host_to_net_uint32_t(uint32_t x) {
	return htonl(x);
}
uint16_t host_to_net_uint16_t(uint16_t x) {
	return htons(x);
}


int recv_timeout(socket_t sock, char * buffer, int len, int flags, struct timeval timeout) {
	fd_set fds = make_fd_set();
	FD_SET(sock, &fds);
	switch (select(sock+1, &fds, NULL, NULL, &timeout)) {
	case -1:
		NANOLOG("select <%s>\n", nanonet_error_tostring(nanonet_error()));
		break;
	case 0: // timed out
		return -2;
	default:
		if (FD_ISSET(sock, &fds))
			return recv(sock, buffer, len, flags);
		break;
	}
	return SOCKET_ERROR;
}

uint32_t recvuint32(socket_t sock, uint32_t * val, int flags) {
	uint32_t x;
	int r=recv(sock, (char*)&x, 4, flags);
	*val = net_to_host_uint32_t(x);
	return r;
}

int recv_all(socket_t sock, char * buffer, int len, int flags) {
	return recv(sock, buffer, len, flags | MSG_WAITALL);
}

int recv_buffer(socket_t sock, buffer * buff, int flags) {
	int len=recv(sock, &buff->buffer[buff->used], buff->size-buff->used, flags);
	if (0<len)
		buff->used += len;
	return len;
}
int recv_buffer_timeout(socket_t sock, buffer * buff, int flags, struct timeval timeout) {
	int len=recv_timeout(sock, (char*)&buff->buffer[buff->used], buff->size-buff->used, flags, timeout);
	if (0<len)
		buff->used += len;
	return len;
}

socket_t vsingle_accept(socket_t sock, bool (*serv)(socket_t, va_list), ...) {
	bool r;
	if (sock==INVALID_SOCKET)
		return sock;
	do {
		struct sockaddr_in addr;
		int len=sizeof(addr);
		socket_t acc=accept(sock, (struct sockaddr*)&addr, &len);
		if (acc==INVALID_SOCKET) {
			NANOLOG("accept failed <%d>\n", nanonet_error());
		} else {
			char * hostport = getnameinfo_as_string((struct sockaddr const*)&addr, len);
			NANOLOG("accept [%s] <- [%s:%d]\n", hostport
				         , inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			free(hostport);
			{
				va_list ap;
				va_start(ap, serv);
				r=serv(acc, ap);
				va_end(ap);
			}
		}
		close_socket_t(acc);
	} while (r); // end if serv is fail
	return sock;
}

socket_t single_accept(socket_t sock, bool (*serv)(socket_t)) {
	bool r;
	if (sock==INVALID_SOCKET)
		return sock;
	do {
		struct sockaddr_in addr;
		int len=sizeof(addr);
		socket_t acc=accept(sock, (struct sockaddr*)&addr, &len);
		if (acc==INVALID_SOCKET) {
			NANOLOG("accept failed <%d>\n", nanonet_error());
		} else {
			char * hostport = getnameinfo_as_string((struct sockaddr const*)&addr, len);
			NANOLOG("accept [%s] <- [%s:%d]\n", hostport
				         , inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			free(hostport);
			r=serv(acc);
		}
		close_socket_t(acc);
	} while (r); // end if serv is fail
	return sock;
}

