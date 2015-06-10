/*
 * $Id: mount_statfs.c,v 1.7 2006-01-08 12:04:24 obarthel Exp $
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

#ifndef _MOUNT_HEADERS_H
#include "mount_headers.h"
#endif /* _MOUNT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
statfs(const char *path, struct statfs *buf)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	D_S(struct InfoData,id);
	LONG status;
	BPTR lock = ZERO;
	int result = ERROR;

	ENTER();

	SHOWSTRING(path);
	SHOWPOINTER(buf);

	if(__check_abort_enabled)
		__check_abort();

	assert( path != NULL && buf != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path == NULL || buf == NULL)
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
			if(path[0] == '\0')
			{
				SHOWMSG("no name given");

				__set_errno(ENOENT);
				goto out;
			}

			if(__translate_unix_to_amiga_path_name(&path,&path_nti) != 0)
				goto out;

			SHOWSTRING(path);

			/* The pseudo root directory is a very special case indeed.
			 * We make up some pseudo data for it.
			 */
			if(path_nti.is_root)
			{
				SHOWMSG("returning data for / directory");

				memset(buf,0,sizeof(*buf));

				buf->f_bsize	= 512;
				buf->f_blocks	= 1;
				buf->f_bfree	= 1;
				buf->f_iosize	= buf->f_bsize;
				buf->f_bavail	= buf->f_bfree;
				buf->f_flags	= MNT_NOATIME|MNT_SYMPERM|MNT_LOCAL|MNT_RDONLY;

				result = OK;

				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	D(("trying to get a lock on '%s'",path));

	PROFILE_OFF();
	lock = Lock((STRPTR)path,SHARED_LOCK);
	PROFILE_ON();

	if(lock == ZERO)
	{
		SHOWMSG("that didn't work");

		__set_errno(__translate_access_io_error_to_errno(IoErr()));
		goto out;
	}

	PROFILE_OFF();
	status = Info(lock,id);
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		SHOWMSG("didn't get the info");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	__convert_info_to_statfs(id,buf);

	result = OK;

 out:

	PROFILE_OFF();

	UnLock(lock);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
