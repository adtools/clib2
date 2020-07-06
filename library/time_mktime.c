/*
 * $Id: time_mktime.c,v 1.11 2015-06-26 11:22:00 obarthel Exp $
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

time_t
mktime(struct tm *tm)
{
	DECLARE_UTILITYBASE();
	struct ClockData clock_data;
	ULONG seconds;
	time_t result = (time_t)-1;
	LONG combined_seconds;
	int month, year;

	ENTER();

	assert( tm != NULL );
	assert( UtilityBase != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(tm == NULL)
		{
			SHOWMSG("invalid tm parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* Normalize the year and month. */
	year = tm->tm_year + 1900;
	month = tm->tm_mon + 1;

	if(month < 0 || month > 12)
	{
		int y;

		y = month / 12;

		month -= y * 12;
		year += y;
	}

	if(month < 1)
	{
		month += 12;
		year -= 1;
	}

	/* The year must be valid. Amiga time begins with January 1st, 1978. */
	if(year < 1978)
	{
		SHOWVALUE(year);
		SHOWMSG("invalid year");
		goto out;
	}

	/* Convert the first day of the month in the given year
	   into the corresponding number of seconds. */
	memset(&clock_data, 0, sizeof(clock_data));
	
	clock_data.mday		= 1;
	clock_data.month	= month;
	clock_data.year		= year;

	seconds = Date2Amiga(&clock_data);

	/* Put the combined number of seconds involved together,
	   covering the seconds/minutes/hours of the day as well
	   as the number of days of the month. This will be added
	   to the number of seconds for the date. */
	combined_seconds = tm->tm_sec + 60 * (tm->tm_min + 60 * (tm->tm_hour + 24 * (tm->tm_mday-1)));
	
	/* If the combined number of seconds is negative, adding it
	 * to the number of seconds for the date should not produce
	 * a negative value.
	 */
	if(combined_seconds < 0 && seconds < (ULONG)(-combined_seconds))
	{
		SHOWVALUE(seconds);
		SHOWVALUE(combined_seconds);
		SHOWMSG("invalid combined number of seconds");
		goto out;
	}

	seconds += combined_seconds;

	__locale_lock();

	/* The data in 'struct tm *tm' was given in local time. We need
	   to convert the result into UTC. */
	if(__default_locale != NULL)
		seconds += 60 * __default_locale->loc_GMTOffset;

	__locale_unlock();

	/* Adjust for the difference between the Unix and the
	   AmigaOS epochs, which differ by 8 years. */
	result = seconds + UNIX_TIME_OFFSET;

	/* Finally, normalize the provided time and date information. */
	localtime_r(&result, tm);

 out:

	RETURN(result);
	return(result);
}
