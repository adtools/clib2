/*
 * $Id: time_clock.c,v 1.2 2004-09-29 19:57:58 obarthel Exp $
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

/****************************************************************************/

static struct DateStamp start_time;

/****************************************************************************/

CLIB_CONSTRUCTOR(init_start_time)
{
	/* Remember when this program was started. */
	DateStamp(&start_time);

	RETURN(OK);

	CONSTRUCTOR_SUCCEED();
}

/****************************************************************************/

clock_t
clock(void)
{
	struct DateStamp now;
	LONG minutes_now,ticks_now;
	LONG minutes_start,ticks_start;
	clock_t result;

	ENTER();

	/* Get the current time. */

	PROFILE_OFF();
	DateStamp(&now);
	PROFILE_ON();

	/* Break the current and start time down into minutes and ticks. */
	minutes_now		= now.ds_Days * 24 * 60 + now.ds_Minute;
	ticks_now		= now.ds_Tick;

	minutes_start	= start_time.ds_Days * 24 * 60 + start_time.ds_Minute;
	ticks_start		= start_time.ds_Tick;

	/* Subtract the start time from the current time. We start
	 * with the ticks.
	 */
	ticks_now -= ticks_start;

	/* Check for underflow. */
	while(ticks_now < 0)
	{
		/* Borrow a minute from the current time. */
		ticks_now += 60 * TICKS_PER_SECOND;

		minutes_now--;
	}

	/* Now for the minutes. */
	minutes_now -= minutes_start;

	/* Check if any time has passed at all, then return the difference. */
	if(minutes_now >= 0)
		result = (clock_t)(minutes_now * 60 * TICKS_PER_SECOND + ticks_now);
	else
		result = (clock_t)0;

	RETURN(result);
	return(result);
}
