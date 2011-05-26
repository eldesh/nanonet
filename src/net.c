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
		fprintf(stderr, "%s failure <%s>\n", __FUNCTION__, nanonet_error_tostring(r));
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
		fprintf(stderr, "getaddrinfo failed <%d>\n", nanonet_error());
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
		fprintf(stderr, "getnameinfo failed <%d>\n", nanonet_error());
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
		printf("server_socket [%s:%s]\n", hostname, portname);
		sock=socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (sock==INVALID_SOCKET) {
			fprintf(stderr, "socket failed <%d>\n", nanonet_error());
			goto END;
		}
		if (bind(sock, info->ai_addr, info->ai_addrlen)) {
			fprintf(stderr, "bind failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
		if (listen(sock, SOMAXCONN)) {
			fprintf(stderr, "listen failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
	} else
		fprintf(stderr, "make_addrinfo failed <%d>\n", nanonet_error());
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
		printf("[%s:%s]\n", hostname, portname);
		sock=socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (sock==INVALID_SOCKET) {
			fprintf(stderr, "socket failed <%d>\n", nanonet_error());
			goto END;
		}
		if (connect(sock, info->ai_addr, info->ai_addrlen)==INVALID_SOCKET) {
			fprintf(stderr, "connect failed <%d>\n", nanonet_error());
			close_socket_t(sock);
			sock=INVALID_SOCKET;
			goto END;
		}
	} else
		fprintf(stderr, "make_addrinfo failed <%d>\n", nanonet_error());
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
			fprintf(stderr, "recv failed <%d>\n", nanonet_error());
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
			fprintf(stderr, "sendall error\n");
			return false;
		}
		acc += r;
	} while (acc<len);
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

socket_t vsingle_accept(socket_t sock, bool (*serv)(socket_t, va_list), ...) {
	bool r;
	if (sock==INVALID_SOCKET)
		return sock;
	do {
		struct sockaddr_in addr;
		int len=sizeof(addr);
		socket_t acc=accept(sock, (struct sockaddr*)&addr, &len);
		if (acc==INVALID_SOCKET) {
			fprintf(stderr, "accept failed <%d>\n", nanonet_error());
		} else {
			char * hostport = getnameinfo_as_string((struct sockaddr const*)&addr, len);
			printf("accept [%s] <- [%s:%d]\n", hostport
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
			fprintf(stderr, "accept failed <%d>\n", nanonet_error());
		} else {
			char * hostport = getnameinfo_as_string((struct sockaddr const*)&addr, len);
			printf("accept [%s] <- [%s:%d]\n", hostport
				         , inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			free(hostport);
			r=serv(acc);
		}
		close_socket_t(acc);
	} while (r); // end if serv is fail
	return sock;
}

