/*
 * $Id: stat_stat.c,v 1.7 2005-02-28 10:07:30 obarthel Exp $
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

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
stat(const char * path_name, struct stat * st)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	D_S(struct FileInfoBlock,fib);
	struct FileLock * fl;
	int result = -1;
	LONG status;
	BPTR file_lock = ZERO;

	ENTER();

	SHOWSTRING(path_name);
	SHOWPOINTER(st);

	assert( path_name != NULL && st != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL || st == NULL)
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
			if(__translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
				goto out;

			/* The pseudo root directory is a very special case indeed.
			 * We make up some pseudo data for it.
			 */
			if(path_name_nti.is_root)
			{
				struct DateStamp ds;
				time_t mtime;

				SHOWMSG("seting up the root directory info");

				memset(st,0,sizeof(*st));

				PROFILE_OFF();
				DateStamp(&ds);
				PROFILE_ON();

				mtime = __convert_datestamp_to_time(&ds);

				st->st_mode		= S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
				st->st_mtime	= mtime;
				st->st_atime	= mtime;
				st->st_ctime	= mtime;
				st->st_nlink	= 2;
				st->st_blksize	= 512;

				result = 0;
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	D(("trying to get a lock on '%s'",path_name));

	PROFILE_OFF();
	file_lock = Lock((STRPTR)path_name,SHARED_LOCK);
	PROFILE_ON();

	if(file_lock == ZERO)
	{
		SHOWMSG("that didn't work");

		__set_errno(__translate_access_io_error_to_errno(IoErr()));
		goto out;
	}

	PROFILE_OFF();
	status = Examine(file_lock,fib);
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		SHOWMSG("couldn't examine it");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	fl = BADDR(file_lock);

	__convert_file_info_to_stat(fl->fl_Task,fib,st);

	result = 0;

 out:

	PROFILE_OFF();
	UnLock(file_lock);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
