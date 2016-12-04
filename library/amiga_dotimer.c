/*
 * $Id: amiga_dotimer.c,v 1.8 2006-04-05 06:43:56 obarthel Exp $
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

#include <exec/errors.h>

#include <devices/timer.h>

#include <string.h>

/****************************************************************************/

#include <proto/exec.h>
#include <clib/alib_protos.h>

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#include "macros.h"
#include "debug.h"

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

LONG
DoTimer(struct timeval *tv,LONG unit,LONG command)
{
	struct timerequest * tr = NULL;
	struct MsgPort * mp;
	LONG error;

	PROFILE_OFF();

	assert( tv != NULL );

	#if defined(__amigaos4__)
	{
		mp = AllocSysObjectTags(ASOT_PORT,
			ASOPORT_Action,		PA_SIGNAL,
			ASOPORT_AllocSig,	FALSE,
			ASOPORT_Signal,		SIGB_SINGLE,
			ASOPORT_Target,		FindTask(NULL),
		TAG_DONE);

		if(mp == NULL)
		{
			error = IOERR_OPENFAIL;
			goto out;
		}
	}
	#else
	{
		mp = AllocVec(sizeof(*mp),MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR);
		if(mp == NULL)
		{
			error = IOERR_OPENFAIL;
			goto out;
		}

		mp->mp_Node.ln_Type	= NT_MSGPORT;
		mp->mp_Flags		= PA_SIGNAL;
		mp->mp_SigBit		= SIGB_SINGLE;
		mp->mp_SigTask		= FindTask(NULL);

		NewList(&mp->mp_MsgList);
	}
	#endif /* __amigaos4__ */

	tr = (struct timerequest *)CreateIORequest(mp,sizeof(*tr));
	if(tr == NULL)
	{
		error = IOERR_OPENFAIL;
		goto out;
	}

	error = OpenDevice(TIMERNAME,(ULONG)unit,(struct IORequest *)tr,0);
	if(error != 0)
		goto out;

	tr->tr_node.io_Command	= command;
	tr->tr_time.tv_secs		= tv->tv_secs;
	tr->tr_time.tv_micro	= tv->tv_micro;

	SetSignal(0,(1UL << mp->mp_SigBit));

	error = DoIO((struct IORequest *)tr);

	tv->tv_secs		= tr->tr_time.tv_secs;
	tv->tv_micro	= tr->tr_time.tv_micro;

 out:

	if(tr != NULL)
	{
		if(tr->tr_node.io_Device != NULL)
			CloseDevice((struct IORequest *)tr);

		DeleteIORequest((struct IORequest *)tr);
	}

	#if defined(__amigaos4__)
	{
		if(mp != NULL)
			FreeSysObject(ASOT_PORT,mp);
	}
	#else
	{
		FreeVec(mp);
	}
	#endif /* __amigaos4__ */

	PROFILE_ON();

	return(error);
}
