/*
 * $Id: unistd_time_delay.c,v 1.9 2006-04-05 08:39:45 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#ifndef PROTO_TIMER_H
#include <proto/timer.h>
#endif /* PROTO_TIMER_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* A quick workaround for the timeval/timerequest->TimeVal/TimeRequest
   change in the recent OS4 header files. */

#if defined(__NEW_TIMEVAL_DEFINITION_USED__)

#define timeval		TimeVal
#define tv_secs		Seconds
#define tv_micro	Microseconds

#define timerequest	TimeRequest
#define tr_node		Request
#define tr_time		Time

#endif /* __NEW_TIMEVAL_DEFINITION_USED__ */

/****************************************************************************/

unsigned int
__time_delay(unsigned long seconds,unsigned long microseconds)
{
	unsigned int result = 0;

	ENTER();

	SHOWVALUE(seconds);

	if(__check_abort_enabled)
		__check_abort();

	if((seconds > 0 || microseconds > 0) && NOT __timer_busy)
	{
		#if defined(__amigaos4__)
		struct TimerIFace * ITimer = __ITimer;
		#else
		struct Library * TimerBase = __TimerBase;
		#endif /* __amigaos4__ */

		ULONG signals_to_wait_for;
		ULONG seconds_then;
		ULONG timer_signal;
		struct timeval tv;
		ULONG signals;

		__timer_busy = TRUE;

		__timer_request->tr_node.io_Command	= TR_ADDREQUEST;
		__timer_request->tr_time.tv_secs	= seconds;
		__timer_request->tr_time.tv_micro	= microseconds;

		timer_signal = (1UL << __timer_port->mp_SigBit);

		signals_to_wait_for = timer_signal;

		SetSignal(0,signals_to_wait_for);

		if(__check_abort_enabled)
			SET_FLAG(signals_to_wait_for,__break_signal_mask);

		PROFILE_OFF();
		GetSysTime(&tv);
		PROFILE_ON();

		seconds_then = tv.tv_secs + seconds;

		SendIO((struct IORequest *)__timer_request);

		while(TRUE)
		{
			PROFILE_OFF();
			signals = Wait(signals_to_wait_for);
			PROFILE_ON();

			if(FLAG_IS_SET(signals,__break_signal_mask))
			{
				ULONG seconds_now;

				if(CheckIO((struct IORequest *)__timer_request) == BUSY)
					AbortIO((struct IORequest *)__timer_request);

				WaitIO((struct IORequest *)__timer_request);

				SetSignal(__break_signal_mask,__break_signal_mask);
				__check_abort();

				/* Now figure out how many seconds have elapsed and
				   how many would still remain. */
				PROFILE_OFF();
				GetSysTime(&tv);
				PROFILE_ON();

				seconds_now = tv.tv_secs;
				if(seconds_now < seconds_then)
					result = seconds_then - seconds_now;

				break;
			}

			if(FLAG_IS_SET(signals,timer_signal))
			{
				WaitIO((struct IORequest *)__timer_request);
				break;
			}
		}

		__timer_busy = FALSE;
	}

	RETURN(result);
	return(result);
}
