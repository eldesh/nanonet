#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <buffer.h>

#if !defined min
#  define min(lhs, rhs) ((lhs)<(rhs) ? (lhs) : (rhs))
#endif

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

buffer_slice taken_buffer_slice(buffer_slice xs, size_t n) {
	return make_buffer_slice(xs.buffer, min(n, xs.size));
}

buffer_slice dropn_buffer_slice(buffer_slice xs, size_t n) {
	size_t const len=min(n, xs.size);
	return make_buffer_slice(xs.buffer+len, xs.size-len);
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
	size_t i;
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

bool expand_buffer(buffer * buff, size_t size) {
	if (!buff)
		return false;

	if ((size_t)buff->size < size) {
		byte * newbuff = (byte*)realloc(buff->buffer, sizeof(byte)*size);
		if (!newbuff) {
			NANOLOG("reallocation fail\n");
			return false;
		}
		buff->buffer = newbuff;
		buff->size = size;
	}
	return true;
}

