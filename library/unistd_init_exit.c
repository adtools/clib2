/*
 * $Id: unistd_init_exit.c,v 1.1.1.1 2004-07-26 16:32:28 obarthel Exp $
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

#include "stdlib_protos.h"

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

int
__unistd_init(void)
{
	ENTER();

	NewList((struct List *)&__unlink_list);

	RETURN(OK);
	return(OK);
}

/****************************************************************************/

void
__unistd_exit(void)
{
	ENTER();

	PROFILE_OFF();

	if(__unlink_list.mlh_Head != NULL && NOT IsListEmpty((struct List *)&__unlink_list))
	{
		struct UnlinkNode * uln;
		BPTR old_dir;

		while((uln = (struct UnlinkNode *)RemHead((struct List *)&__unlink_list)))
		{
			old_dir = CurrentDir(uln->uln_Lock);

			D(("deleting '%s'",uln->uln_Name));

			DeleteFile(uln->uln_Name);

			CurrentDir(old_dir);

			UnLock(uln->uln_Lock);
		}
	}

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

	if(__current_directory_changed)
	{
		BPTR old_dir;

		old_dir = CurrentDir(__original_current_directory);
		__original_current_directory = ZERO;

		if(__unlock_current_directory)
		{
			UnLock(old_dir);

			__unlock_current_directory = FALSE;
		}

		__current_directory_changed = FALSE;
	}

	PROFILE_ON();

	LEAVE();
}
