/*
 * $Id: time_converttime.c,v 1.2 2005-01-02 09:07:19 obarthel Exp $
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

/****************************************************************************/

struct tm *
__convert_time(ULONG seconds, LONG gmt_offset, struct tm * tm)
{
	DECLARE_UTILITYBASE();
	struct ClockData clock_data;
	struct tm * result;
	ULONG delta;

	ENTER();

	assert( UtilityBase != NULL );

	/* We need to convert the time from Unix-style UTC
	 * back into Amiga style local time.
	 *
	 * First, the Unix time offset will have to go.
	 */
	if(seconds < UNIX_TIME_OFFSET)
		seconds = 0;
	else
		seconds -= UNIX_TIME_OFFSET;

	/* Now the local time offset will have to go. */
	seconds -= gmt_offset;

	Amiga2Date(seconds, &clock_data);

	tm->tm_sec		= clock_data.sec;
	tm->tm_min		= clock_data.min;
	tm->tm_hour		= clock_data.hour;
	tm->tm_mday		= clock_data.mday;
	tm->tm_mon		= clock_data.month - 1;
	tm->tm_year		= clock_data.year - 1900;
	tm->tm_wday		= clock_data.wday;
	tm->tm_isdst	= -1;

	clock_data.mday		= 1;
	clock_data.month	= 1;

	delta = Date2Amiga(&clock_data);

	tm->tm_yday = (seconds - delta) / (24 * 60 * 60);

	result = tm;

	RETURN(result);
	return(result);
}
