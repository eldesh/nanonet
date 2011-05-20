#if !defined BOOL_INCLUDED
#define      BOOL_INCLUDED

#if defined _WIN32
typedef enum {false=0, true} bool;
#else
//#  include <stdbool.h>
//typedef _Bool bool;
typedef enum {false=0, true} bool;
#endif

#endif    /* BOOL_INCLUDED */
