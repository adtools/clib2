/*
 * $Id: fcntl_open.c,v 1.6 2005-01-09 15:58:02 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* This is used in place of ExamineFH() in order to work around a bug in
   dos.library V40 and below: a "NIL:" file handle will crash the
   ExamineFH() function. */
static LONG
safe_examine_file_handle(BPTR file_handle,struct FileInfoBlock *fib)
{
	LONG result = DOSFALSE;

	assert( fib != NULL );

	#ifndef __amigaos4__
	{
		struct FileHandle * fh = (struct FileHandle *)BADDR(file_handle);

		if(fh == NULL || fh->fh_Type == NULL)
		{
			SetIoErr(ERROR_OBJECT_WRONG_TYPE);
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	PROFILE_OFF();
	result = ExamineFH(file_handle,fib);
	PROFILE_ON();

 out:

	return(result);
}

/* Same thing as above, but for ChangeMode(), which suffers from
   the same problem. */
static LONG
safe_change_mode(LONG type,BPTR file_handle,LONG mode)
{
	LONG result = DOSFALSE;

	#ifndef __amigaos4__
	{
		struct FileHandle * fh = (struct FileHandle *)BADDR(file_handle);

		assert( type == CHANGE_FH );

		if(fh == NULL || fh->fh_Type == NULL)
		{
			SetIoErr(ERROR_OBJECT_WRONG_TYPE);
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	PROFILE_OFF();
	result = ChangeMode(type,file_handle,mode);
	PROFILE_ON();

 out:

	return(result);
}

/****************************************************************************/

int
open(const char *path_name, int open_flag, ... /* mode_t mode */ )
{
	DECLARE_UTILITYBASE();
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	D_S(struct FileInfoBlock,fib);
	LONG is_file_system = FALSE;
	LONG open_mode;
	BPTR lock = ZERO;
	BPTR handle = ZERO;
	BOOL create_new_file = FALSE;
	LONG is_interactive;
	int fd_slot_number;
	struct fd * fd;
	int access_mode;
	int result = -1;
	int i;

	ENTER();

	SHOWSTRING(path_name);
	SHOWVALUE(open_flag);

	assert(path_name != NULL);
	assert( UtilityBase != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("path name is invalid");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	access_mode = (open_flag & 3);
	if(access_mode < O_RDONLY && access_mode > O_RDWR)
	{
		SHOWMSG("access mode is invalid");

		errno = EINVAL;
		goto out;
	}

	fd_slot_number = __find_vacant_fd_entry();
	if(fd_slot_number < 0)
	{
		if(__grow_fd_table() < 0)
		{
			SHOWMSG("couldn't find a vacant file descriptor, and couldn't allocate one either");
			goto out;
		}

		fd_slot_number = __find_vacant_fd_entry();
		assert( fd_slot_number >= 0 );
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(__translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
				goto out;

			if(path_name_nti.is_root)
			{
				errno = EACCES;
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if (Strnicmp(path_name, "PIPE:",5) == SAME && FLAG_IS_SET(open_flag, O_CREAT))
	{
		open_mode = MODE_NEWFILE;
	}
	else if (Strnicmp(path_name,"NIL:",4) != SAME && FLAG_IS_SET(open_flag,O_CREAT))
	{
		if(FLAG_IS_SET(open_flag,O_EXCL))
		{
			LONG error;

			SHOWMSG("checking if the file to create already exists");

			PROFILE_OFF();
			lock = Lock((STRPTR)path_name,SHARED_LOCK);
			PROFILE_ON();

			if(lock != ZERO)
			{
				SHOWMSG("the file already exists");

				errno = EEXIST;
				goto out;
			}

			error = IoErr();

			if(error == ERROR_OBJECT_WRONG_TYPE)
			{
				SHOWMSG("there's something not a directory on the path");

				errno = ENOTDIR;
				goto out;
			}
			else if (error != ERROR_OBJECT_NOT_FOUND && error != ERROR_ACTION_NOT_KNOWN)
			{
				SHOWMSG("error accessing the object");

				__translate_io_error_to_errno(IoErr(),&errno);
				goto out;
			}

			SHOWMSG("the object does not already exist");
		}

		open_mode = MODE_READWRITE;

		if(FLAG_IS_SET(open_flag,O_TRUNC))
		{
			SHOWMSG("checking if the file to create already exists");

			PROFILE_OFF();
			lock = Lock((STRPTR)path_name,SHARED_LOCK);
			PROFILE_ON();

			if(lock != ZERO)
			{
				LONG status;

				PROFILE_OFF();
				status = Examine(lock,fib);
				PROFILE_ON();

				if(status == DOSFALSE)
				{
					SHOWMSG("could not examine the object");

					__translate_io_error_to_errno(IoErr(),&errno);
					goto out;
				}

				/* We can open only files, but never directories. */
				if(fib->fib_DirEntryType >= 0)
				{
					SHOWMSG("can't open a directory");

					errno = EISDIR;
					goto out;
				}

				if(FLAG_IS_SET(fib->fib_Protection,FIBF_WRITE) ||
				   FLAG_IS_SET(fib->fib_Protection,FIBF_DELETE))
				{
					SHOWMSG("this object is not write enabled");

					errno = EACCES;
					goto out;
				}

				open_mode = MODE_NEWFILE;

				PROFILE_OFF();

				UnLock(lock);
				lock = ZERO;

				PROFILE_ON();
			}
			else
			{
				LONG error;

				error = IoErr();

				if(error == ERROR_OBJECT_WRONG_TYPE)
				{
					SHOWMSG("there's something not a directory on the path");

					errno = ENOTDIR;
					goto out;
				}
				else if (error != ERROR_OBJECT_NOT_FOUND && error != ERROR_ACTION_NOT_KNOWN)
				{
					SHOWMSG("error accessing the object");

					__translate_io_error_to_errno(IoErr(),&errno);
					goto out;
				}
			}
		}

		create_new_file = TRUE;
	}
	else
	{
		open_mode = MODE_OLDFILE;
	}

	SHOWSTRING(path_name);

	PROFILE_OFF();
	handle = Open((STRPTR)path_name,open_mode);
	PROFILE_ON();

	if(handle == ZERO)
	{
		LONG io_err = IoErr();

		D(("the file '%s' didn't open in mode %ld",path_name,open_mode));
		__translate_access_io_error_to_errno(IoErr(),&errno);

		/* Check if ended up trying to open a directory as if
		   it were a plain file. */
		if(io_err == ERROR_OBJECT_WRONG_TYPE)
		{
			PROFILE_OFF();

			lock = Lock((STRPTR)path_name,SHARED_LOCK);
			if(lock != ZERO)
			{
				if(Examine(lock,fib) && fib->fib_DirEntryType >= 0)
					errno = EISDIR;
			}

			PROFILE_ON();
		}

		goto out;
	}

	if(safe_examine_file_handle(handle,fib) != DOSFALSE)
	{
		BOOL operation_permitted = TRUE;

		/* Check if the file is readable. */
		if(FLAG_IS_SET(fib->fib_Protection,FIBF_READ))
		{
			if(access_mode == O_RDONLY ||
			   access_mode == O_RDWR)
			{
				operation_permitted = FALSE;
			}
		}

		/* Check if the file can be written to. */
		if(FLAG_IS_SET(fib->fib_Protection,FIBF_WRITE))
		{
			if(access_mode == O_WRONLY)
				operation_permitted = FALSE;
		}

		if(NOT operation_permitted)
		{
			SHOWMSG("this object must not be opened");

			errno = EACCES;
			goto out;
		}
	}

	fd = __fd[fd_slot_number];

	__initialize_fd(fd,(HOOKFUNC)__fd_hook_entry,handle,0);

	/* Figure out if this stream is attached to a console. */
	PROFILE_OFF();
	is_interactive = IsInteractive(handle);
	PROFILE_ON();

	if(is_interactive)
	{
		SET_FLAG(fd->fd_Flags,FDF_IS_INTERACTIVE);

		if(FLAG_IS_SET(open_flag,O_NONBLOCK))
		{
			SHOWMSG("enabling non-blocking mode");

			if(SetMode(handle,1)) /* single character mode */
				SET_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
		}
	}
	else
	{
		size_t len;

		len = 0;

		for(i = 0 ; path_name[i] != '\0' ; i++)
		{
			if(path_name[i] == ':')
			{
				len = i+1;
				break;
			}
		}

		if(len > 0)
		{
			char * path_name_copy;

			path_name_copy = malloc(len+1);
			if(path_name_copy != NULL)
			{
				memmove(path_name_copy,path_name,len);
				path_name_copy[len] = '\0';

				PROFILE_OFF();
				is_file_system = IsFileSystem(path_name_copy);
				PROFILE_ON();

				free(path_name_copy);
			}
		}
		else
		{
			PROFILE_OFF();
			is_file_system = IsFileSystem("");
			PROFILE_ON();
		}

		if(is_file_system)
		{
			/* We opened the file in exclusive access mode. Switch it back
			   into shared access mode so that its contents can be read
			   while it's still open. */
			if(open_mode == MODE_NEWFILE)
				safe_change_mode(CHANGE_FH,handle,SHARED_LOCK);

			/* We should be able to seek in this file. */
			SET_FLAG(fd->fd_Flags,FDF_CACHE_POSITION);
		}
	}

	if(FLAG_IS_SET(open_flag,O_APPEND))
	{
		SHOWMSG("appending; seeking to end of file");

		PROFILE_OFF();
		Seek(handle,0,OFFSET_END);
		PROFILE_ON();

		SET_FLAG(fd->fd_Flags,FDF_APPEND);
	}

	switch(access_mode)
	{
		case O_RDONLY:

			SET_FLAG(fd->fd_Flags,FDF_READ);
			break;

		case O_WRONLY:

			SET_FLAG(fd->fd_Flags,FDF_WRITE);
			break;

		case O_RDWR:

			SET_FLAG(fd->fd_Flags,FDF_READ);
			SET_FLAG(fd->fd_Flags,FDF_WRITE);
			break;
	}

	if(create_new_file && is_file_system)
		SET_FLAG(fd->fd_Flags,FDF_CREATED);

	SET_FLAG(fd->fd_Flags,FDF_IN_USE);

	result = fd_slot_number;

	handle = ZERO;

	assert( result != -1 );

 out:

	PROFILE_OFF();

	if(handle != ZERO)
		Close(handle);

	UnLock(lock);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
