/*
 * $Id: time_gettimeofday.c,v 1.8 2005-03-18 12:38:25 obarthel Exp $
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

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#include <sys/time.h>

/****************************************************************************/

#ifndef PROTO_TIMER_H
#include <proto/timer.h>
#endif /* PROTO_TIMER_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	#if defined(__amigaos4__)
	struct TimerIFace * ITimer = __ITimer;
	#else
	struct Library * TimerBase = __TimerBase;
	#endif /* __amigaos4__ */

	ULONG seconds,microseconds;
	struct timeval tv;

	ENTER();

	/* Obtain the current system time. */
	PROFILE_OFF();
	GetSysTime(&tv);
	PROFILE_ON();

	/* Convert the number of seconds so that they match the Unix epoch, which
	   starts (January 1st, 1970) eight years before the AmigaOS epoch. */
	seconds			= tv.tv_sec + UNIX_TIME_OFFSET;
	microseconds	= tv.tv_usec;

	__locale_lock();

	/* If possible, adjust for the local time zone. We do this because the
	   AmigaOS system time is returned in local time and we want to return
	   it in UTC. */
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

		/* The -1 means "we do not know if the time given is in
		   daylight savings time". */
		tzp->tz_dsttime = -1;

		SHOWVALUE(tzp->tz_minuteswest);
		SHOWVALUE(tzp->tz_dsttime);
	}

	__locale_unlock();

	RETURN(0);
	return(0);
}
