/*
 * $Id: unistd_access.c,v 1.8 2006-01-08 12:04:27 obarthel Exp $
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
access(const char * path_name, int mode)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	int result = ERROR;
	BPTR lock = ZERO;

	ENTER();

	SHOWSTRING(path_name);
	SHOWVALUE(mode);

	assert( path_name != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("invalid path name");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(mode < 0 || mode > (R_OK|W_OK|X_OK|F_OK))
	{
		SHOWMSG("invalid mode");

		__set_errno(EINVAL);
		goto out;
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		STRPTR actual_path_name = NULL;

		if(__unix_path_semantics)
		{
			if(path_name[0] == '\0')
			{
				SHOWMSG("no name given");

				__set_errno(ENOENT);
				goto out;
			}

			if(__translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
				goto out;

			if(NOT path_name_nti.is_root)
				actual_path_name = (STRPTR)path_name;
		}

		if(actual_path_name != NULL)
		{
			D(("trying to get a lock on '%s'",actual_path_name));

			lock = Lock(actual_path_name,SHARED_LOCK);
			if(lock == ZERO)
			{
				__set_errno(__translate_access_io_error_to_errno(IoErr()));
				goto out;
			}
		}
	}
	#else
	{
		D(("trying to get a lock on '%s'",path_name));

		PROFILE_OFF();
		lock = Lock((STRPTR)path_name,SHARED_LOCK);
		PROFILE_ON();

		if(lock == ZERO)
		{
			__set_errno(__translate_access_io_error_to_errno(IoErr()));
			goto out;
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if((mode != F_OK) && (mode & (R_OK|W_OK|X_OK)) != 0)
	{
		D_S(struct FileInfoBlock,fib);

		#if defined(UNIX_PATH_SEMANTICS)
		{
			if(lock == ZERO)
			{
				memset(fib,0,sizeof(*fib));

				/* This is a simulated directory which cannot be
				 * modified under program control.
				 */
				fib->fib_Protection		= FIBF_WRITE;
				fib->fib_DirEntryType	= ST_ROOT;
			}
			else
			{
				LONG status;

				PROFILE_OFF();
				status = Examine(lock,fib);
				PROFILE_ON();

				if(status == DOSFALSE)
				{
					SHOWMSG("couldn't examine");

					__set_errno(__translate_io_error_to_errno(IoErr()));
					goto out;
				}
			}
		}
		#else
		{
			LONG status;

			PROFILE_OFF();
			status = Examine(lock,fib);
			PROFILE_ON();

			if(status == DOSFALSE)
			{
				SHOWMSG("couldn't examine");

				__set_errno(__translate_io_error_to_errno(IoErr()));
				goto out;
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */

		fib->fib_Protection ^= FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE;

		if(FLAG_IS_SET(mode,R_OK))
		{
			if(FLAG_IS_CLEAR(fib->fib_Protection,FIBF_READ))
			{
				SHOWMSG("not readable");

				__set_errno(EACCES);
				goto out;
			}
		}

		if(FLAG_IS_SET(mode,W_OK))
		{
			if(FLAG_IS_CLEAR(fib->fib_Protection,FIBF_WRITE) ||
			   FLAG_IS_CLEAR(fib->fib_Protection,FIBF_DELETE))
			{
				SHOWMSG("not writable");

				__set_errno(EACCES);
				goto out;
			}	
		}

		if(FLAG_IS_SET(mode,X_OK))
		{
			/* Note: 'X' means 'search' for directories, which is always
			 *       permitted on the Amiga.
			 */
			if((fib->fib_DirEntryType < 0) && FLAG_IS_CLEAR(fib->fib_Protection,FIBF_EXECUTE))
			{
				SHOWMSG("not executable");

				__set_errno(EACCES);
				goto out;
			}	
		}
	}

	result = OK;

 out:

	PROFILE_OFF();
	UnLock(lock);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
