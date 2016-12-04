/*
 * $Id: unistd_getcwd.c,v 1.10 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#undef getcwd

/****************************************************************************/

__static char *
__getcwd(char * buffer,size_t buffer_size,const char *file,int line)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info buffer_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	char * buffer_allocated = NULL;
	char * result = NULL;
	BPTR dir_lock = ZERO;

	ENTER();

	SHOWPOINTER(buffer);
	SHOWVALUE(buffer_size);

	assert( buffer != NULL );
	assert( (int)buffer_size > 0 );

	if(__check_abort_enabled)
		__check_abort();

	if(buffer_size == 0)
	{
		SHOWMSG("invalid buffer size");

		__set_errno(EINVAL);
		goto out;
	}

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(buffer == NULL)
		{
			SHOWMSG("invalid buffer parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	PROFILE_OFF();
	dir_lock = Lock("",SHARED_LOCK);
	PROFILE_ON();

	if(dir_lock == ZERO)
	{
		SHOWMSG("could not get a lock on the current directory");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	/* If no buffer is provided, the behaviour of this function is
	   actually undefined. But we try to play nice and allocate
	   a custom buffer for the result to be returned. */
	if(buffer == NULL)
	{
		buffer_allocated = __malloc(buffer_size,file,line);
		if(buffer_allocated == NULL)
		{
			SHOWMSG("not enough memory for result buffer");

			__set_errno(ENOMEM);
			goto out;
		}

		buffer = buffer_allocated;
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__current_path_name[0] != '\0')
		{
			if(buffer_size < strlen(__current_path_name) + 1)
			{
				SHOWMSG("buffer is too small");

				__set_errno(ERANGE);
				goto out;
			}

			strcpy(buffer,__current_path_name);

			D(("returning absolute path name '%s'",buffer));

			result = buffer;
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if(result == NULL)
	{
		LONG status;

		PROFILE_OFF();
		status = NameFromLock(dir_lock,buffer,(LONG)buffer_size);
		PROFILE_ON();

		if(status == DOSFALSE)
		{
			int errno_code;
			LONG io_error;

			SHOWMSG("could not get name from lock");

			io_error = IoErr();

			/* Was the buffer too small? */
			if(io_error == ERROR_LINE_TOO_LONG)
				errno_code = ERANGE;
			else
				errno_code = __translate_io_error_to_errno(io_error);

			__set_errno(errno_code);
			goto out;
		}

		#if defined(UNIX_PATH_SEMANTICS)
		{
			if(__unix_path_semantics)
			{
				const char * path_name = buffer;

				if(__translate_amiga_to_unix_path_name(&path_name,&buffer_nti) != 0)
					goto out;

				if(buffer_size < strlen(path_name) + 1)
				{
					SHOWMSG("buffer is too small");

					__set_errno(ERANGE);
					goto out;
				}

				strcpy(buffer,path_name);
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */
	}

	SHOWSTRING(buffer);

	result = buffer;

 out:

	if(result == NULL && buffer_allocated != NULL)
		free(buffer_allocated);

	PROFILE_OFF();
	UnLock(dir_lock);
	PROFILE_ON();

	RETURN(result);
	return(result);
}

/****************************************************************************/

char *
getcwd(char * buffer, size_t buffer_size)
{
	char * result;

	result = __getcwd(buffer,buffer_size,NULL,0);

	return(result);
}
