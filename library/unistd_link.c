/*
 * $Id: unistd_link.c,v 1.5 2005-02-28 10:07:32 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
link(const char * existing_path,const char * new_path)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info existing_path_name_nti;
	struct name_translation_info new_path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	BPTR existing_path_lock = ZERO;
	int result = -1;
	LONG status;

	ENTER();

	SHOWSTRING(existing_path);
	SHOWSTRING(new_path);

	assert( existing_path != NULL && new_path != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(existing_path == NULL || new_path == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(__translate_unix_to_amiga_path_name(&existing_path,&existing_path_name_nti) != 0)
				goto out;

			if(__translate_unix_to_amiga_path_name(&new_path,&new_path_name_nti) != 0)
				goto out;

			if(existing_path_name_nti.is_root || new_path_name_nti.is_root)
			{
				__set_errno(EACCES);
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	D(("trying to get a lock on '%s'",existing_path));

	PROFILE_OFF();
	existing_path_lock = Lock((STRPTR)existing_path,SHARED_LOCK);
	PROFILE_ON();

	if(existing_path_lock == ZERO)
	{
		SHOWMSG("that didn't work");

		__set_errno(__translate_access_io_error_to_errno(IoErr()));
		goto out;
	}

	D(("trying to make a link named '%s'",new_path));

	PROFILE_OFF();
	status = MakeLink((STRPTR)new_path,existing_path_lock,LINK_HARD);
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		SHOWMSG("that didn't work");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	result = 0;

 out:

	PROFILE_OFF();
	UnLock(existing_path_lock);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
