/*
 * $Id: time_asctime.c,v 1.1.1.1 2004-07-26 16:32:21 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

/****************************************************************************/

static void
add_to_string(char * to,int to_size,const char * string,int * offset_ptr)
{
	int offset;
	int len;

	assert( to != NULL && to_size > 0 && string != NULL && offset_ptr != NULL );

	offset = (*offset_ptr);

	assert( offset < to_size );

	len = strlen(string);
	if(offset + len > (to_size-1))
		len = (to_size-1) - offset;

	if(len > 0)
	{
		memmove(&to[offset],string,(size_t)len);
		offset += len;

		assert( offset < to_size );

		to[offset] = '\0';

		(*offset_ptr) = offset;
	}
}

/****************************************************************************/

char *
asctime(const struct tm *tm)
{
	static char buffer[40];

	struct tm copy_tm;
	char number[16];
	char * result = NULL;
	const char * b;
	int offset = 0;

	ENTER();

	SHOWPOINTER(tm);

	assert( tm != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(tm == NULL)
		{
			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* Fill in the week day if it's not in proper range. */
	if(tm->tm_wday < 0 || tm->tm_wday > 6)
	{
		struct tm other_tm;
		time_t seconds;

		other_tm = (*tm);

		seconds = mktime(&other_tm);
		if(seconds != (time_t)-1)
		{
			__convert_time(seconds,0,&other_tm);

			copy_tm = (*tm);
			copy_tm.tm_wday = other_tm.tm_wday;

			tm = &copy_tm;
		}
	}

	if(0 <= tm->tm_wday && tm->tm_wday <= 6)
		b = __abbreviated_week_day_names[tm->tm_wday];
	else
		b = "---";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer)," ",	&offset);

	if(0 <= tm->tm_mon && tm->tm_mon <= 11)
		b = __abbreviated_month_names[tm->tm_mon];
	else
		b = "---";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer)," ",	&offset);

	if(1 <= tm->tm_mday && tm->tm_mday <= 31)
		b = __number_to_string((unsigned int)tm->tm_mday,number,sizeof(number),2);
	else
		b = "--";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer)," ",	&offset);

	if(0 <= tm->tm_hour && tm->tm_hour <= 23)
		b = __number_to_string((unsigned int)tm->tm_hour,number,sizeof(number),2);
	else
		b = "--";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer),":",	&offset);

	if(0 <= tm->tm_min && tm->tm_min <= 59)
		b = __number_to_string((unsigned int)tm->tm_min,number,sizeof(number),2);
	else
		b = "--";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer),":",	&offset);

	if(0 <= tm->tm_sec && tm->tm_sec <= 59)
		b = __number_to_string((unsigned int)tm->tm_sec,number,sizeof(number),2);
	else
		b = "--";

	add_to_string(buffer,sizeof(buffer),b,		&offset);
	add_to_string(buffer,sizeof(buffer)," ",	&offset);

	if(0 <= tm->tm_year)
		b = __number_to_string((unsigned int)1900 + tm->tm_year,number,sizeof(number),0);
	else
		b = "----";

	add_to_string(buffer,sizeof(buffer),b,&offset);

	SHOWSTRING(buffer);

	add_to_string(buffer,sizeof(buffer),"\n",&offset);

	assert( offset < sizeof(buffer) );
	assert( strlen(buffer) < sizeof(buffer) );

	result = buffer;

 out:

	RETURN(result);
	return(result);
}
