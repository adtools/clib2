/*
 * $Id: unistd_sleep.c,v 1.1.1.1 2004-07-26 16:32:30 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

unsigned int
sleep(unsigned int seconds)
{
	static BOOL sleeping;
	unsigned int result = 0;

	ENTER();

	SHOWVALUE(seconds);

	if(__check_abort_enabled)
		__check_abort();

	if(seconds > 0)
	{
		if(NOT sleeping)
		{
			BOOL timer_open;

			sleeping = TRUE;

			if(__timer_request == NULL)
			{
				timer_open = FALSE;

				SHOWMSG("trying to open the timer");

				PROFILE_OFF();

				__timer_port = CreateMsgPort();
				if(__timer_port != NULL)
				{
					__timer_request = (struct timerequest *)CreateIORequest(__timer_port,sizeof(*__timer_request));
					if(__timer_request != NULL)
					{
						if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)__timer_request,0) == 0)
							timer_open = TRUE;
					}
				}

				if(NOT timer_open)
				{
					SHOWMSG("that didn't work");

					DeleteIORequest((struct IORequest *)__timer_request);
					__timer_request = NULL;

					DeleteMsgPort(__timer_port);
					__timer_port = NULL;
				}

				PROFILE_ON();
			}
			else
			{
				timer_open = TRUE;
			}

			if(timer_open)
			{
				ULONG signals_to_wait_for;
				ULONG seconds_then;
				ULONG timer_signal;
				struct DateStamp ds;
				ULONG signals;

				__timer_request->tr_node.io_Command	= TR_ADDREQUEST;
				__timer_request->tr_time.tv_secs	= seconds;
				__timer_request->tr_time.tv_micro	= 0;

				timer_signal = (1UL << __timer_port->mp_SigBit);

				signals_to_wait_for = timer_signal;

				SetSignal(0,signals_to_wait_for);

				if(__check_abort_enabled)
					SET_FLAG(signals_to_wait_for,SIGBREAKF_CTRL_C);

				PROFILE_OFF();
				DateStamp(&ds);
				PROFILE_ON();

				seconds_then = (ds.ds_Days * 24 * 60 + ds.ds_Minute) * 60 + ds.ds_Tick / TICKS_PER_SECOND;

				SendIO((struct IORequest *)__timer_request);

				while(TRUE)
				{
					PROFILE_OFF();
					signals = Wait(signals_to_wait_for);
					PROFILE_ON();

					if(FLAG_IS_SET(signals,SIGBREAKF_CTRL_C))
					{
						ULONG seconds_now;

						if(CheckIO((struct IORequest *)__timer_request))
							AbortIO((struct IORequest *)__timer_request);

						WaitIO((struct IORequest *)__timer_request);

						SetSignal(SIGBREAKF_CTRL_C,SIGBREAKF_CTRL_C);
						__check_abort();

						PROFILE_OFF();
						DateStamp(&ds);
						PROFILE_ON();

						seconds_now = (ds.ds_Days * 24 * 60 + ds.ds_Minute) * 60 + ds.ds_Tick / TICKS_PER_SECOND;

						seconds_now -= seconds_then;
						if(seconds_now < seconds)
							result = (seconds - seconds_now);

						break;
					}

					if(FLAG_IS_SET(signals,timer_signal))
					{
						WaitIO((struct IORequest *)__timer_request);
						break;
					}
				}
			}

			sleeping = FALSE;
		}
	}

	RETURN(result);
	return(result);
}
