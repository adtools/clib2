/*
 * $Id: usergroup_init_exit.c,v 1.7 2005-03-11 13:23:18 obarthel Exp $
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

#if defined(USERGROUP_SUPPORT)

/****************************************************************************/

#ifndef _USERGROUP_HEADERS_H
#include "usergroup_headers.h"
#endif /* _USERGROUP_HEADERS_H */

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

/* Pointer to errno, length == sizeof(long) */
#define UGT_ERRNOLPTR 0x80000004

/****************************************************************************/

struct Library * __UserGroupBase;

/****************************************************************************/

#if defined(__amigaos4__)
struct UserGroupIFace * __IUserGroup;
#endif /* __amigaos4__ */

/****************************************************************************/

CLIB_DESTRUCTOR(__usergroup_exit)
{
	ENTER();

	#if defined(__amigaos4__)
	{
		if(__IUserGroup != NULL)
		{
			DropInterface((struct Interface *)__IUserGroup);
			__IUserGroup = NULL;
		}
	}
	#endif /* __amigaos4__ */

	if(__UserGroupBase != NULL)
	{
		CloseLibrary(__UserGroupBase);
		__UserGroupBase = NULL;
	}

	LEAVE();
}

/****************************************************************************/

CLIB_CONSTRUCTOR(__usergroup_init)
{
	struct TagItem tags[2];
	BOOL success = FALSE;

	ENTER();

	PROFILE_OFF();

	__UserGroupBase = OpenLibrary("usergroup.library",0);

	#if defined(__amigaos4__)
	{
		if(__UserGroupBase != NULL)
		{
			__IUserGroup = (struct UserGroupIFace *)GetInterface(__UserGroupBase, "main", 1, 0);
			if (__IUserGroup == NULL)
			{
				CloseLibrary(__UserGroupBase);
				__UserGroupBase = NULL;
			}
		}
	}
	#endif /* __amigaos4__ */

	if(__UserGroupBase == NULL)
	{
		SHOWMSG("usergroup.library did not open");

		__show_error("\"usergroup.library\" could not be opened.");
		goto out;
	}

	/* Wire the library's errno variable to our local errno. */
	tags[0].ti_Tag	= UGT_ERRNOLPTR;
	tags[0].ti_Data	= (ULONG)&errno;

	tags[1].ti_Tag = TAG_END;

	if(__ug_SetupContextTagList(__program_name,tags) != 0)
	{
		SHOWMSG("could not initialize usergroup.library");

		__show_error("\"usergroup.library\" could not be initialized.");
		goto out;
	}

	success = TRUE;

 out:

	PROFILE_ON();

	RETURN(success);

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}

/****************************************************************************/

#endif /* USERGROUP_SUPPORT */
