/*
 * $Id: stat_lstat.c,v 1.9 2005-04-03 10:22:47 obarthel Exp $
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

/*
 * lstat_lock(): An implementation of Lock() which remembers whether or
 *               not it resolved soft links.
 *
 * Unfortunately is is limited to 255 character names.
 */

static BPTR
lstat_lock(const char *name,const int mode,int * link_length)
{
	D_S(struct bcpl_name,bname);
	const size_t name_size = sizeof(bname->name);
	char * new_name = NULL;
	struct DevProc * dvp = NULL;
	BPTR lock = ZERO;
	size_t name_len;
	LONG error;

	assert( name != NULL && link_length != NULL );

	name_len = strlen(name);
	if(name_len >= name_size)
	{
		SetIoErr(ERROR_LINE_TOO_LONG);
		goto out;
	}

	/* Convert the name into a BCPL string. */
	bname->name[0] = name_len;
	memmove(&bname->name[1],name,name_len);

	while(TRUE)
	{
		/* Get a handle on the device, volume or assignment name in the path. */
		dvp = GetDeviceProc((STRPTR)name,dvp);
		if(dvp == NULL)
			goto out;

		/* Try to obtain a lock on the object. */
		lock = DoPkt(dvp->dvp_Port,ACTION_LOCATE_OBJECT,dvp->dvp_Lock,MKBADDR(bname),mode,0,0);
		if(lock != ZERO)
			break;

		error = IoErr();

		if(error == ERROR_OBJECT_NOT_FOUND)
		{
			/* If this is part of a multi-volume assignment, try the next part. */
			if(FLAG_IS_SET(dvp->dvp_Flags,DVPF_ASSIGN))
				continue;

			/* Not much we can do here... */
			break;
		}
		else if (error == ERROR_IS_SOFT_LINK)
		{
			LONG result;

			/* For soft link resolution we need a temporary buffer to
			   let the file system store the resolved path name in. */
			new_name = malloc(name_size);
			if(new_name == NULL)
			{
				SetIoErr(ERROR_NO_FREE_STORE);
				goto out;
			}

			/* Now ask the file system to resolve the entire path. */
			result = ReadLink(dvp->dvp_Port,dvp->dvp_Lock,(STRPTR)name,(STRPTR)name,name_size);
			if(result < 0)
			{
				/* This will return either -1 (resolution error) or -2
				   (buffer too small). We regard both as trouble. */
				SetIoErr(ERROR_INVALID_COMPONENT_NAME);
				goto out;
			}

			assert( result > 0 );

			/* Remember the length of the link name. */
			(*link_length) = result;

			/* Finished for now. */
			break;
		}
		else
		{
			/* Some other error; ask if the user wants to have another go at it. */
			if(ErrorReport(error,REPORT_LOCK,dvp->dvp_Lock,dvp->dvp_Port) != 0)
				break;
		}

		/* Retry the lookup. */
		FreeDeviceProc(dvp);
		dvp = NULL;
	}

 out:

	error = IoErr();

	if(new_name != NULL)
		free(new_name);

	FreeDeviceProc(dvp);

	SetIoErr(error);

	return(lock);
}

int
lstat(const char * path_name, struct stat * st)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	D_S(struct FileInfoBlock,fib);
	struct FileLock * fl;
	int result = -1;
	LONG status;
	BPTR file_lock = ZERO;
	int link_length = -1;

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
			   We make up some pseudo data for it. */
			if(path_name_nti.is_root)
			{
				time_t mtime;

				SHOWMSG("setting up the root directory info");

				memset(st,0,sizeof(*st));

				time(&mtime);

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
	file_lock = lstat_lock(path_name,SHARED_LOCK,&link_length);
	PROFILE_ON();

	if(file_lock == ZERO && link_length < 0)
	{
		SHOWMSG("that didn't work");

		__set_errno(__translate_access_io_error_to_errno(IoErr()));
		goto out;
	}

	if(link_length > 0)
	{
		time_t mtime;

		/* Build a dummy stat for the link. */

		SHOWMSG("Creating stat info for link.");

		memset(st,0,sizeof(*st));

		time(&mtime);

		st->st_mode		= S_IFLNK | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
		st->st_dev		= (ULONG)DeviceProc((STRPTR)path_name);
		st->st_size		= link_length;
		st->st_mtime	= mtime;
		st->st_atime	= mtime;
		st->st_ctime	= mtime;
		st->st_nlink	= 1;
	}
	else
	{
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
	}

	result = 0;

 out:

	PROFILE_OFF();
	UnLock(file_lock);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
