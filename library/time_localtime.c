/*
 * $Id: time_localtime.c,v 1.2 2004-11-17 19:07:22 obarthel Exp $
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

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

struct tm *
localtime_r(const time_t *t,struct tm * tm_ptr)
{
	struct tm * result = NULL;
	LONG gmt_offset;

	ENTER();

	assert( t != NULL && tm_ptr != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(t == NULL || tm_ptr == NULL)
		{
			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* The time parameter given represents local time and
	 * must be converted to UTC before we proceed.
	 */
	if(__default_locale != NULL)
		gmt_offset = 60 * __default_locale->loc_GMTOffset;
	else
		gmt_offset = 0;

	SHOWVALUE(gmt_offset);

	result = __convert_time((*t), gmt_offset, tm_ptr);

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

struct tm *
localtime(const time_t *t)
{
	static struct tm tm;

	struct tm * result;

	ENTER();

	result = localtime_r(t,&tm);

	RETURN(result);
	return(result);
}
