/*
 * $Id: mount_fstatfs.c,v 1.14 2006-11-16 14:39:23 obarthel Exp $
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
fstatfs(int file_descriptor, struct statfs *buf)
{
	D_S(struct InfoData,id);
	BPTR parent_dir = ZERO;
	int result = ERROR;
	struct fd * fd = NULL;
	LONG success;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWPOINTER(buf);

	assert( buf != NULL );

	if(__check_abort_enabled)
		__check_abort();

	__stdio_lock();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(buf == NULL)
		{
			SHOWMSG("invalid buffer parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		__set_errno(EBADF);
		goto out;
	}

	__fd_lock(fd);

	if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
	{
		__set_errno(EINVAL);
		goto out;
	}

	if(FLAG_IS_SET(fd->fd_Flags,FDF_STDIO))
	{
		__set_errno(EBADF);
		goto out;
	}

	PROFILE_OFF();
	parent_dir = __safe_parent_of_file_handle(fd->fd_File);
	PROFILE_ON();

	if(parent_dir == ZERO)
	{
		SHOWMSG("couldn't find parent directory");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	PROFILE_OFF();
	success = Info(parent_dir,id);
	PROFILE_ON();

	if(NO success)
	{
		SHOWMSG("couldn't get info on drive");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	__convert_info_to_statfs(id,buf);

	result = OK;

 out:

	__fd_unlock(fd);

	__stdio_unlock();

	UnLock(parent_dir);

	RETURN(result);
	return(result);
}
