/*
 * $Id: time_gettimeofday.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
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

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

#include <sys/time.h>

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	ULONG seconds,microseconds;
	struct DateStamp ds;

	ENTER();

	PROFILE_OFF();
	DateStamp(&ds);
	PROFILE_ON();

	seconds			= UNIX_TIME_OFFSET + 60 * ((ULONG)ds.ds_Minute + 24 * 60 * (ULONG)ds.ds_Days) + (ds.ds_Tick / TICKS_PER_SECOND);
	microseconds	= (1000000 * (ds.ds_Tick % TICKS_PER_SECOND)) / TICKS_PER_SECOND;

	if(__default_locale != NULL)
		seconds += 60 * __default_locale->loc_GMTOffset;

	if(tp != NULL)
	{
		tp->tv_sec	= (long)seconds;
		tp->tv_usec	= (long)microseconds;

		SHOWVALUE(tp->tv_sec);
		SHOWVALUE(tp->tv_usec);
	}

	if(tzp != NULL)
	{
		if(__default_locale != NULL)
			tzp->tz_minuteswest = __default_locale->loc_GMTOffset;
		else
			tzp->tz_minuteswest = 0;

		tzp->tz_dsttime = -1;

		SHOWVALUE(tzp->tz_minuteswest);
		SHOWVALUE(tzp->tz_dsttime);
	}

	RETURN(0);
	return(0);
}
