#if !defined NETWORK_INCLUDED
#define      NETWORK_INCLUDED

#if defined _WIN32
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

#endif    /* NETWORK_INCLUDED */

