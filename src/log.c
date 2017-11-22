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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <net/log.h>
#include <bool.h>

NANONET_LOG_TYPE nanonet_log_type = NANONET_LOG_DEBUGVIEW;

size_t const LOG_MSG_BUFF_SIZE = 1024;

static
int print_msg_impl( const char * file, const int line
                  , const char * func, const char * header, const char * format, va_list ap);

char const * nanolog_tail (size_t n, char const * const string) {
	if (strlen(string)>n)
		return string+(strlen(string)-n);
	else
		return string;
}

// enabled when NANONET_LOG_ENABLE is defined
int nanolog_impl( const char * file, const int line
				, const char * func, const char * format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret=print_msg_impl(file, line, func, "NANOLOG", format, ap);
	va_end(ap);
	return ret;
}

int nanolog_dummy(const char * dummy, ...) {  return 0;  }

static int vprint_msg_debugview (char const * format, va_list ap) {
#if defined _WIN32
	size_t const len = strlen(format)+256;
	char * buff = (char*)calloc(len, sizeof(char));
	int ret=vsnprintf(buff, len, format, ap);
	OutputDebugString(buff);
	free(buff);
	return ret;
#else
	return vfprintf(stderr, format, ap);
#endif
}

// output formatted string to there along the logtype 
static int nanolog_vprintf(char const * format, va_list ap) {
	int ret;
	if (nanonet_log_type==NANONET_LOG_DEBUGVIEW)
		ret = vprint_msg_debugview(format, ap);
	else if (nanonet_log_type==NANONET_LOG_STDERR)
		ret = vfprintf(stderr, format, ap);	// pass the format string to standard function
	else if (nanonet_log_type==NANONET_LOG_STDOUT)
		ret = vfprintf(stdout, format, ap);	// pass the format string to standard function
	else {
		assert(false); // unknown log type is specified
	}
	return ret;
}

int nanolog_impl2(const char * format, ...) {
	if (nanonet_log_type==NANONET_LOG_OFF)
		return 0;
	{
		// this is used when platform unsuport variadic macro
		int ret=-1;
		char * msg_buffer = (char*)malloc(sizeof(char)*(LOG_MSG_BUFF_SIZE+strlen(format)+1));
		va_list ap;
		sprintf(msg_buffer, "[NANOLOG] ");             // add log header to string
		strcpy(msg_buffer+strlen(msg_buffer), format); // concatenate specified format
		va_start(ap, format);
		ret=nanolog_vprintf(msg_buffer, ap);
		va_end(ap);
		free(msg_buffer);
		return ret;
	}
}


static int print_msg_impl( const char * file, const int line
						 , const char * func, const char * header, const char * format, va_list ap)
{
	if (nanonet_log_type==NANONET_LOG_OFF)
		return 0;
	{
		int result=-1;
		char * msg_buffer = (char*)malloc(sizeof(char)*(LOG_MSG_BUFF_SIZE+strlen(format)+1));
		sprintf(msg_buffer, "[%s] "                                               \
			"%"  PP_TOSTRING(LOG_MSG_PATH_MAX_LENGTH) "s "        \
			": %5d @ "                                            \
			"%-" PP_TOSTRING(LOG_MSG_FUNCNAME_MAX_LENGTH) "s> "   \
			, header, file, line, func);
		strcpy(msg_buffer+strlen(msg_buffer), format);
		result = nanolog_vprintf(msg_buffer, ap);
		free(msg_buffer);
		return result;
	}
}

