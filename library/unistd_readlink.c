/*
 * $Id: unistd_readlink.c,v 1.4 2005-02-03 16:56:17 obarthel Exp $
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
readlink(const char * path_name, char * buffer, int buffer_size)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	struct name_translation_info buffer_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	struct DevProc * dvp = NULL;
	BPTR lock = ZERO;
	int result = -1;

	ENTER();

	SHOWSTRING(path_name);
	SHOWPOINTER(buffer);
	SHOWVALUE(buffer_size);

	assert( path_name != NULL && buffer != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL || buffer == NULL)
		{
			SHOWSTRING("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics && __translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
			goto out;
	}
	#endif /* UNIX_PATH_SEMANTICS */

	D(("trying to get a lock on '%s'",path_name));

	PROFILE_OFF();
	lock = Lock((STRPTR)path_name,SHARED_LOCK);
	PROFILE_ON();

	if(lock != ZERO)
	{
		LONG status;

		SHOWMSG("trying to obtain the absolute path");

		PROFILE_OFF();	
		status = NameFromLock(lock,buffer,buffer_size);
		PROFILE_ON();

		if(status == DOSFALSE)
		{
			SHOWMSG("that didn't work");

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}
	}
	else
	{
		LONG read_link_result;

		PROFILE_OFF();
		dvp = GetDeviceProc((STRPTR)path_name,NULL);
		PROFILE_ON();

		if(dvp == NULL)
		{
			SHOWMSG("didn't get deviceproc");

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		PROFILE_OFF();
		read_link_result = ReadLink(dvp->dvp_Port,dvp->dvp_Lock,(STRPTR)path_name,buffer,(ULONG)buffer_size);
		PROFILE_ON();

		if(read_link_result == -1)
		{
			SHOWMSG("couldn't read the link");

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}
		else if (read_link_result == -2)
		{
			SHOWMSG("buffer was too short");

			__set_errno(ENOBUFS);
			goto out;
		}
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(__translate_amiga_to_unix_path_name((char const **)&buffer,&buffer_nti) != 0)
				goto out;

			__restore_path_name((char const **)&buffer,&buffer_nti);

			strcpy(buffer,buffer_nti.substitute);
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	result = strlen(buffer);

	SHOWSTRING(buffer);

 out:

	PROFILE_OFF();

	FreeDeviceProc(dvp);
	UnLock(lock);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
