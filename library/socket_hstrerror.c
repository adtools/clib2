/*
 * $Id: socket_hstrerror.c,v 1.1 2004-07-28 15:50:45 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

const char *
hstrerror(int error_number)
{
	const char * result;

	if(error_number < 1 || error_number > 4)
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

		strcpy(error_buffer,"Unknown resolver error ");
		strcat(error_buffer,number);

		result = error_buffer;
	}
	else
	{
		switch(error_number)
		{
			case 1:	/* HOST_NOT_FOUND */

				result = "Unknown host";
				break;

			case 2:	/* TRY_AGAIN */

				result = "Host name lookup failure; try again";
				break;

			case 3:	/* NO_RECOVERY */

				result = "Unknown server error_number";
				break;

			case 4:	/* NO_ADDRESS */

				result = "No address associated with name";
				break;
		}
	}

	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
