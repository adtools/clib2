/*
 * $Id: string_strerror.c,v 1.2 2005-01-02 09:07:19 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

/****************************************************************************/

static char * error_table[EILSEQ - EPERM + 1] =
{
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"Input/output error",
	"Device not configured",
	"Argument list too long",
	"Exec format error",
	"Bad file descriptor",
	"No child processes",
	"Resource deadlock avoided",
	"Cannot allocate memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device busy",
	"File exists",
	"Cross-device link",
	"Operation not supported by device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"Too many open files in system",
	"Too many open files",
	"Inappropriate ioctl for device",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Numerical argument out of domain",
	"Result too large",
	"Resource temporarily unavailable",
	"Operation now in progress",
	"Operation already in progress",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported on socket",
	"Protocol family not supported",
	"Address family not supported by protocol family",
	"Address already in use",
	"Can't assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped connection on reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",
	"Socket is already connected",
	"Socket is not connected",
	"Can't send after socket shutdown",
	"Too many references: can't splice",
	"Connection timed out",
	"Connection refused",
	"Too many levels of symbolic links",
	"File name too long",
	"Host is down",
	"No route to host",
	"Directory not empty",
	"Too many processes",
	"Too many users",
	"Disc quota exceeded",
	"Stale NFS file handle",
	"Too many levels of remote in path",
	"RPC struct is bad",
	"RPC version wrong",
	"RPC program not available",
	"Program version wrong",
	"Bad procedure for program",
	"No locks available",
	"Function not implemented",
	"Inappropriate file type or format",
	"Authentication error",
	"Need authenticator",
	"Identifier removed",
	"No message of the desired type.",
	"Value too large to be stored in data type.",
	"Encoding error detected"
};

/****************************************************************************/

char *
strerror(int error_number)
{
	char * result;

	if(error_number < EPERM || error_number > EILSEQ)
	{
		static char error_buffer[80];
		char number[30];
		char *s = number;
		int is_negative = 0;
		unsigned int n;
		int i,len,c;

		/* We convert the error number into in an unsigned
		   integer, so that numbers such as 0x80000000
		   can come out of the conversion. */
		if(error_number < 0)
		{
			is_negative = 1;

			n = (-error_number);
		}
		else
		{
			n = error_number;
		}

		/* Convert the error number into a string of digits. */
		len = 0;

		do
		{
			(*s++) = '0' + (n % 10);
			n /= 10;
			len++;
		}
		while(n > 0 && len < (int)sizeof(number)-1);

		/* Add the sign, if necessary. */
		if(is_negative && len < (int)sizeof(number)-1)
		{
			(*s++) = '-';
			len++;
		}

		(*s) = '\0';

		/* Reverse the string in place. */
		for(i = 0 ; i < len / 2 ; i++)
		{
			c				= number[len-1-i];
			number[len-1-i]	= number[i];
			number[i]		= c;
		}

		strcpy(error_buffer,"Unknown error ");
		strcat(error_buffer,number);

		result = error_buffer;
	}
	else
	{
		result = error_table[error_number - EPERM];
	}

	return(result);
}
