/*
 * $Id: unistd_init_exit.c,v 1.14 2006-09-27 09:40:06 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

/* Names of files and directories to delete when shutting down. */
struct MinList			NOCOMMON __unlink_list;
struct SignalSemaphore	NOCOMMON __unlink_semaphore;

/****************************************************************************/

CLIB_CONSTRUCTOR(unistd_init)
{
	ENTER();

	NewList((struct List *)&__unlink_list);
	InitSemaphore(&__unlink_semaphore);

	LEAVE();

	CONSTRUCTOR_SUCCEED();
}

/****************************************************************************/

CLIB_DESTRUCTOR(unistd_exit)
{
	ENTER();

	PROFILE_OFF();

	if(__unlink_list.mlh_Head != NULL && NOT IsMinListEmpty(&__unlink_list))
	{
		struct UnlinkNode * uln;
		BPTR old_dir;

		/* Close all the files that still might be open. */
		__close_all_files();

		while((uln = (struct UnlinkNode *)RemHead((struct List *)&__unlink_list)))
		{
			D(("deleting '%s'",uln->uln_Name));

			old_dir = CurrentDir(uln->uln_Lock);
			DeleteFile(uln->uln_Name);
			CurrentDir(old_dir);

			UnLock(uln->uln_Lock);
		}
	}

	PROFILE_ON();

	LEAVE();
}
