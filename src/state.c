#include <stdlib.h>
#include <net/state.h>

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
	int i;
	for (i=0; i<src.size; ++i)
		dst->buffer[dst->used+i] = src.buffer[i];
	dst->used += src.size;
}


