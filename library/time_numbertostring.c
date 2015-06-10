/*
 * $Id: time_numbertostring.c,v 1.3 2006-01-08 12:04:27 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

/****************************************************************************/

char *
__number_to_string(
	unsigned int	number,
	char *			string,
	size_t			max_len,
	size_t			min_len)
{
	char * s = string;
	size_t len,i;
	char c;

	assert( string != NULL && max_len >= min_len );

	/* One off for the terminating NUL. */
	if(max_len > 0)
		max_len--;

	len = 0;

	/* Convert the number digit by digit, building the
	 * string in reverse order.
	 */
	do
	{
		if(max_len == 0)
			break;

		max_len--;

		(*s++) = '0' + (number % 10);
		number /= 10;

		len++;
	}
	while(number > 0);

	/* Pad the string to the requested length, if necessary. */
	while(max_len > 0 && len < min_len)
	{
		max_len--;

		(*s++) = '0';
		len++;
	}

	(*s) = '\0';

	/* Reverse the string in place. */
	for(i = 0 ; i < len / 2 ; i++)
	{
		c				= string[len-1-i];
		string[len-1-i]	= string[i];
		string[i]		= c;
	}

	return(string);
}
