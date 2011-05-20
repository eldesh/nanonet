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
typedef st_service_tuple (*service_type)(buffer, byte const **);

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

#endif    /* SERVICE_STATE_MACHINE_INCLUDED */
