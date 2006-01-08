/*
 * $Id: unistd_timer.c,v 1.9 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* Local timer I/O. */
struct MsgPort *		NOCOMMON __timer_port;
struct timerequest *	NOCOMMON __timer_request;
BOOL					NOCOMMON __timer_busy;
struct Library *		NOCOMMON __TimerBase;

/****************************************************************************/

#if defined(__amigaos4__)
struct TimerIFace * NOCOMMON __ITimer;
#endif /* __amigaos4__ */

/****************************************************************************/

CLIB_CONSTRUCTOR(timer_init)
{
	BOOL success = FALSE;

	ENTER();

	__timer_port = CreateMsgPort();
	if(__timer_port == NULL)
	{
		__show_error("The timer message port could not be created.");
		goto out;
	}

	__timer_request = (struct timerequest *)CreateIORequest(__timer_port,sizeof(*__timer_request));
	if(__timer_request == NULL)
	{
		__show_error("The timer I/O request could not be created.");
		goto out;
	}

	if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)__timer_request,0) != OK)
	{
		__show_error("The timer could not be opened.");
		goto out;
	}

	__TimerBase = (struct Library *)__timer_request->tr_node.io_Device;

	#if defined(__amigaos4__)
	{
		__ITimer = (struct TimerIFace *)GetInterface(__TimerBase, "main", 1, 0);
		if(__ITimer == NULL)
		{
			__show_error("The timer interface could not be obtained.");
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}

/****************************************************************************/

CLIB_DESTRUCTOR(timer_exit)
{
	ENTER();

	#if defined(__amigaos4__)
	{
		if(__ITimer != NULL)
			DropInterface((struct Interface *)__ITimer);

		__ITimer = NULL;
	}
	#endif /* __amigaos4__ */

	__TimerBase = NULL;

	if(__timer_request != NULL)
	{
		if(__timer_request->tr_node.io_Device != NULL)
			CloseDevice((struct IORequest *)__timer_request);

		DeleteIORequest((struct IORequest *)__timer_request);
		__timer_request = NULL;
	}

	if(__timer_port != NULL)
	{
		DeleteMsgPort(__timer_port);
		__timer_port = NULL;
	}

	LEAVE();
}
