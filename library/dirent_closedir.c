/*
 * $Id: dirent_closedir.c,v 1.7 2005-02-27 21:58:21 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _DIRENT_HEADERS_H
#include "dirent_headers.h"
#endif /* _DIRENT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
closedir(DIR * directory_pointer)
{
	struct DirectoryHandle * dh;
	int result = -1;

	ENTER();

	SHOWPOINTER(directory_pointer);

	if(__check_abort_enabled)
		__check_abort();

	__dirent_lock();

	if(directory_pointer == NULL)
	{
		__set_errno(EBADF);
		goto out;
	}

	#ifndef NDEBUG
	{
		BOOL directory_pointer_is_valid = FALSE;

		for(dh = (struct DirectoryHandle *)__directory_list.mlh_Head ;
		    dh->dh_MinNode.mln_Succ != NULL ;
		    dh = (struct DirectoryHandle *)dh->dh_MinNode.mln_Succ)
		{
			if(dh == (struct DirectoryHandle *)directory_pointer)
			{
				directory_pointer_is_valid = TRUE;
				break;
			}
		}

		if(NOT directory_pointer_is_valid)
		{
			__set_errno(EBADF);
			goto out;
		}
	}
	#endif /* NDEBUG */

	dh = (struct DirectoryHandle *)directory_pointer;

	Remove((struct Node *)dh);

	#if defined(UNIX_PATH_SEMANTICS)
	{
		struct Node * node;

		while((node = RemHead((struct List *)&dh->dh_VolumeList)) != NULL)
			free(node);
	}
	#endif /* UNIX_PATH_SEMANTICS */

	PROFILE_OFF();
	UnLock(dh->dh_DirLock);
	PROFILE_ON();

	free(dh);

	result = 0;

 out:

	__dirent_unlock();

	RETURN(result);
	return(result);
}
