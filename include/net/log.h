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

/*
 * small LOG utility
 *
 *   switch NANONET_LOG_ENABLE macro, then include this from .c file.
 */
#if !defined NANONET_LOG_INCLUDED
#define      NANONET_LOG_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#if defined _WIN32
#  include <windows.h>
#endif

#define PP_TOSTRING(x)      PP_TOSTRING_IMPL(x)
#define PP_TOSTRING_IMPL(x) #x

#ifdef _WIN32
#  define FUNC_NAME __FUNCTION__
#else
#  define FUNC_NAME __func__
#endif // _WIN32

#define LOG_MSG_PATH_MAX_LENGTH     10
#define LOG_MSG_FUNCNAME_MAX_LENGTH 25

typedef enum { NANONET_LOG_STDOUT
             , NANONET_LOG_STDERR
			 , NANONET_LOG_DEBUGVIEW
			 , NANONET_LOG_SYSLOG
			 , NANONET_LOG_OFF       // turns it off temporarily
} NANONET_LOG_TYPE;

extern NANONET_LOG_TYPE nanonet_log_type; // output destination
extern size_t const LOG_MSG_BUFF_SIZE;

// use in macros, you should not call these functions directly.
int nanolog_impl (const char * file, const int line, const char * func, const char * format, ...);
int nanolog_impl2(const char * format, ...);
char const * nanolog_tail (size_t n, char const * const string);
int nanolog_dummy(const char * dummy, ...);

#endif    /* NANONET_LOG_INCLUDED */


#if _MSC_VER > 1200 || __STDC_VERSION__==199901L
#  define VARIADIC_MACRO_ENABLE
#else
#  if defined VARIADIC_MACRO_ENABLE
#    undef VARIADIC_MACRO_ENABLE
#  endif
#endif

#if defined NANONET_LOG_ENABLE
#  undef  NANOLOG
#  if defined VARIADIC_MACRO_ENABLE
#    define NANOLOG(...)                                                                  \
	     nanolog_impl( nanolog_tail(LOG_MSG_PATH_MAX_LENGTH    ,__FILE__) , __LINE__      \
                     , nanolog_tail(LOG_MSG_FUNCNAME_MAX_LENGTH,FUNC_NAME), __VA_ARGS__)
#  else
#    define NANOLOG \
	     nanolog_impl( nanolog_tail(LOG_MSG_PATH_MAX_LENGTH    ,__FILE__) , __LINE__      \
                     , nanolog_tail(LOG_MSG_FUNCNAME_MAX_LENGTH,FUNC_NAME), "")           \
         , nanolog_impl2
#  endif
#  undef  NANOVLOG
#  define NANOVLOG(expr) NANOVLOG_I(expr)
#  undef  NANOVLOG_I
#  define NANOVLOG_I(expr) NANOLOG("%s <%d>\n", #expr, (expr))
#else
// disabled log
#  if defined VARIADIC_MACRO_ENABLE
#    if defined NANOLOG
#      undef NANOLOG
#    endif
#    if defined NANOVLOG
#      undef NANOVLOG
#    endif
#    define NANOLOG(...)
#    define NANOVLOG(dummy)
#  else
#    if defined NANOLOG
#      undef NANOLOG
#    endif
#    if defined NANOVLOG
#      undef NANOVLOG
#    endif
#    define NANOLOG nanolog_dummy
#    define NANOVLOG(dummy)
#  endif
#endif // NANONET_LOG_ENABLE


