/*
 * $Id: time_mktime.c,v 1.10 2006-01-08 12:04:27 obarthel Exp $
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
	ULONG seconds, delta;
	time_t result = (time_t)-1;
	int max_month_days;

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

	/* The month must be valid. */
	if(tm->tm_mon < 0 || tm->tm_mon > 11)
	{
		SHOWVALUE(tm->tm_mon);
		SHOWMSG("invalid month");
		goto out;
	}

	/* The day of the month must be valid. */
	if(tm->tm_mday < 1 || tm->tm_mday > 31)
	{
		SHOWVALUE(tm->tm_mday);
		SHOWMSG("invalid day of month");
		goto out;
	}

	/* The year must be valid. */
	if(tm->tm_year < 78)
	{
		SHOWVALUE(tm->tm_year);
		SHOWMSG("invalid year");
		goto out;
	}

	/* Is this the month of February? */
	if(tm->tm_mon == 1)
	{
		int year;

		/* We need to have the full year number for the
		   leap year calculation below. */
		year = tm->tm_year + 1900;

		/* Now for the famous leap year calculation rules... In
		   the given year, how many days are there in the month
		   of February? */
		if((year % 4) != 0)
			max_month_days = 28;
		else if ((year % 400) == 0)
			max_month_days = 29;
		else if ((year % 100) == 0)
			max_month_days = 28;
		else
			max_month_days = 29;
	}
	else
	{
		static const char days_per_month[12] =
		{
			31, 0,31,
			30,31,30,
			31,31,30,
			31,30,31
		};

		max_month_days = days_per_month[tm->tm_mon];
	}

	/* The day of the month must be valid. */
	if(tm->tm_mday < 0 || tm->tm_mday > max_month_days)
	{
		SHOWVALUE(tm->tm_mday);
		SHOWMSG("invalid day of month");
		goto out;
	}

	/* The hour must be valid. */
	if(tm->tm_hour < 0 || tm->tm_hour > 23)
	{
		SHOWVALUE(tm->tm_hour);
		SHOWMSG("invalid hour");
		goto out;
	}

	/* The minute must be valid. */
	if(tm->tm_min < 0 || tm->tm_min > 59)
	{
		SHOWVALUE(tm->tm_min);
		SHOWMSG("invalid minute");
		goto out;
	}

	/* Note: the number of seconds can be larger than 59
	         in order to account for leap seconds. */
	if(tm->tm_sec < 0 || tm->tm_sec > 60)
	{
		SHOWVALUE(tm->tm_sec);
		SHOWMSG("invalid seconds");
		goto out;
	}

	clock_data.sec		= (tm->tm_sec > 59) ? 59 : tm->tm_sec;
	clock_data.min		= tm->tm_min;
	clock_data.hour		= tm->tm_hour;
	clock_data.mday		= tm->tm_mday;
	clock_data.month	= tm->tm_mon + 1;
	clock_data.year		= tm->tm_year + 1900;

	seconds = Date2Amiga(&clock_data) + (tm->tm_sec - 59);

	/* The AmigaOS "epoch" starts with January 1st, 1978, which was
	   a Sunday. */
	tm->tm_wday	= (seconds / (24 * 60 * 60)) % 7;

	clock_data.mday		= 1;
	clock_data.month	= 1;

	delta = Date2Amiga(&clock_data);

	tm->tm_yday	= (seconds - delta) / (24 * 60 * 60);

	__locale_lock();

	/* The data in 'struct tm *tm' was given in local time. We need
	   to convert the result into UTC. */
	if(__default_locale != NULL)
		seconds += 60 * __default_locale->loc_GMTOffset;

	__locale_unlock();

	/* Finally, adjust for the difference between the Unix and the
	   AmigaOS epochs, which differ by 8 years. */
	result = seconds + UNIX_TIME_OFFSET;

 out:

	RETURN(result);
	return(result);
}
