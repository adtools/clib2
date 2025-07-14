/*
 * $Id: unistd_readlink.c,v 1.9 2006-11-13 09:25:28 obarthel Exp $
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

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

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

	D_S(struct bcpl_name, bname);
	const size_t name_size = sizeof(bname->name);

	BPTR lock = ZERO;
	int result = ERROR;
    struct DevProc *dvp = NULL;
	char * new_name = NULL;
	char * path_part;
	size_t name_len;
	size_t new_name_size;
	LONG readlink_result;
	LONG error;

	ENTER();

	SHOWSTRING(path_name);
	SHOWPOINTER(buffer);
	SHOWVALUE(buffer_size);

	assert( path_name != NULL && buffer != NULL );

	if (__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if (path_name == NULL || buffer == NULL)
		{
			SHOWSTRING("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if (__unix_path_semantics)
		{
			if (path_name[0] == '\0')
			{
				SHOWMSG("no name given");

				__set_errno(ENOENT);
				goto out;
			}

			if (__translate_unix_to_amiga_path_name(&path_name, &path_name_nti) != 0)
				goto out;
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	name_len = strlen(path_name);
	if (name_len >= name_size)
	{
		SHOWMSG("name too long");

		SetIoErr(ERROR_LINE_TOO_LONG);
		goto out;
	}

	/* Convert the name into a BCPL string. */
	bname->name[0] = name_len;
	memcpy(&bname->name[1], path_name, name_len);

    /* Get a handle on the device, volume or assignment name in the path. */
	dvp = GetDeviceProc((STRPTR)path_name, dvp);
	if (dvp == NULL)
	{
		SHOWMSG("dvp == NULL");
		goto out;
	}

	/* Try to obtain a lock on the object. It should be a link and not
	 * a file or directory.
	 */
	lock = DoPkt(dvp->dvp_Port, ACTION_LOCATE_OBJECT, dvp->dvp_Lock, MKBADDR(bname), SHARED_LOCK, 0, 0);
	if (lock != ZERO)
	{
		SHOWMSG("lock != ZERO");

		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		goto out;
	}

	error = IoErr();

	if (error != ERROR_IS_SOFT_LINK)
	{
		SHOWMSG("not a soft link");
		goto out;
	}

	/* We only need the leading path name. */
	name_len = ((BYTE *)PathPart(path_name)) - (BYTE *)path_name;

	path_part = malloc(name_len+1);
	if (path_part == NULL)
	{
		SHOWMSG("path_part too long");

		SetIoErr(ERROR_NO_FREE_STORE);
		goto out;
	}

	memcpy(path_part, path_name, name_len);
	path_part[name_len] = '\0';

	PROFILE_OFF();
	lock = Lock((STRPTR)path_part, SHARED_LOCK);
	PROFILE_ON();

	new_name_size = name_size+1;

	/* Provide as much buffer space as possible. */
	if (buffer_size > 0 && (size_t)buffer_size > new_name_size)
		new_name_size = buffer_size;

	/* For soft link resolution we need a temporary buffer to
	   let the file system store the resolved path name in. */
	new_name = malloc(new_name_size);
	if (new_name == NULL)
	{
		SHOWMSG("new_name too long");

		SetIoErr(ERROR_NO_FREE_STORE);
		goto out;
	}

	/* Now ask the file system to resolve the entire path. */
	readlink_result = ReadLink(dvp->dvp_Port, lock, (STRPTR)FilePart(path_name), (STRPTR)new_name, (LONG)new_name_size);
	if (readlink_result < 0)
	{
		SHOWMSG("ReadLink");

		/* This will return either -1 (resolution error) or -2
		   (buffer too small). We regard both as trouble. */
		SetIoErr(ERROR_INVALID_COMPONENT_NAME);
		goto out;
	}

	assert( result > 0 );

	/* If the caller supplied a buffer, copy as much of the name
	   as possible into it. */
	if (buffer != NULL && buffer_size > 0)
	{
		strlcpy(buffer, new_name, buffer_size);

		#if defined(UNIX_PATH_SEMANTICS)
		{
			if (__unix_path_semantics)
			{
				if (__translate_amiga_to_unix_path_name((char const **)&buffer, &buffer_nti) != 0)
					goto out;

				__restore_path_name((char const **)&buffer,&buffer_nti);

				strcpy(buffer, buffer_nti.substitute);
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */

		result = strlen(buffer);

		SHOWSTRING(buffer);
	}
	else
	{
		result = 0;
	}

 out:

	PROFILE_OFF();

	if (dvp != NULL)
		FreeDeviceProc(dvp);

	UnLock(lock);

	PROFILE_ON();

	RETURN(result);

	return result;
}
