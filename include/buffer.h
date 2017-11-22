#if !defined BUFFER_INCLUDED
#define      BUFFER_INCLUDED

#include <stddef.h>
#if defined _WIN32
#else
  typedef unsigned char byte;
#endif
#include <net/log.h>
#include <bool.h>

typedef struct buffer_ {
	size_t size;
	size_t used;
	byte * buffer;
} buffer;

typedef struct buffer_slice_ {
	size_t size;
	byte const * buffer;
} buffer_slice;

buffer make_buffer(int size);
buffer_slice make_buffer_slice(byte const * buffer, size_t size);
buffer_slice taken_buffer_slice(buffer_slice xs, size_t n);
buffer_slice dropn_buffer_slice(buffer_slice xs, size_t n);
void delete_buffer(buffer * buff);
void copy_buffer(buffer src, buffer * dst);
void copy_slice_to_buffer(buffer_slice src, buffer * dst);
// expand size of buffer to (size)
// if fail expand, buffer isn't changed.
bool expand_buffer(buffer * buff, size_t size);

#endif    /* BUFFER_INCLUDED */

