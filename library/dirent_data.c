/*
 * $Id: dirent_data.c,v 1.6 2005-02-27 21:58:21 obarthel Exp $
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

#ifndef _DIRENT_HEADERS_H
#include "dirent_headers.h"
#endif /* _DIRENT_HEADERS_H */

/****************************************************************************/

/* Directories being scanned whose locks need to be freed when shutting down. */
struct MinList NOCOMMON __directory_list;

/****************************************************************************/

static struct SignalSemaphore * dirent_lock;

/****************************************************************************/

void
__dirent_lock(void)
{
	if(dirent_lock != NULL)
		ObtainSemaphore(dirent_lock);
}

/****************************************************************************/

void
__dirent_unlock(void)
{
	if(dirent_lock != NULL)
		ReleaseSemaphore(dirent_lock);
}

/****************************************************************************/

CLIB_CONSTRUCTOR(__dirent_init)
{
	BOOL success = FALSE;

	ENTER();

	NewList((struct List *)&__directory_list);

	dirent_lock = AllocVec(sizeof(*dirent_lock),MEMF_ANY|MEMF_PUBLIC);
	if(dirent_lock == NULL)
		goto out;

	InitSemaphore(dirent_lock);

	success = TRUE;

 out:

	RETURN(success);

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}

/****************************************************************************/

CLIB_DESTRUCTOR(__dirent_exit)
{
	ENTER();

	if(__directory_list.mlh_Head != NULL)
	{
		while(NOT IsListEmpty((struct List *)&__directory_list))
			closedir((DIR *)__directory_list.mlh_Head);
	}

	FreeVec(dirent_lock);
	dirent_lock = NULL;

	LEAVE();
}
