/*
 * $Id: unistd_ftruncate.c,v 1.6 2005-02-28 13:22:53 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
ftruncate(int file_descriptor, off_t length)
{
	D_S(struct FileInfoBlock,fib);
	int result = -1;
	struct fd * fd = NULL;
	long int position;
	BOOL success;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(length);

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

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

	if(length < 0)
	{
		SHOWMSG("invalid length");

		__set_errno(EINVAL);
		goto out;
	}

	assert( FLAG_IS_SET(fd->fd_Flags,FDF_IN_USE) );

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_WRITE))
	{
		SHOWMSG("file descriptor is not write-enabled");

		__set_errno(EINVAL);
		goto out;
	}

	/* Remember where we started. */
	position = lseek(file_descriptor,0,SEEK_CUR);
	if(position < 0)
		goto out;

	if(CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,fib))
	{
		SHOWMSG("couldn't examine file");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	PROFILE_OFF();

	if(length < fib->fib_Size)
	{
		/* Careful: seek to a position where the file can be safely truncated. */
		success = (Seek(fd->fd_DefaultFile,length,OFFSET_BEGINNING) != -1 && SetFileSize(fd->fd_DefaultFile,length,OFFSET_BEGINNING) != -1);
	}
	else if (length > fib->fib_Size)
	{
		success = (Seek(fd->fd_DefaultFile,fib->fib_Size,OFFSET_BEGINNING) != -1 && __grow_file_size(fd,length - fib->fib_Size) == OK);
	}
	else
	{
		success = TRUE;
	}

	PROFILE_ON();

	if(NO success)
	{
		int error;

		error = __translate_io_error_to_errno(IoErr());

		/* Return to the original file position. */
		lseek(file_descriptor,position,SEEK_SET);

		__set_errno(error);
		goto out;
	}

	/* Return to the original file position. */
	if(lseek(file_descriptor,position,SEEK_SET) < 0)
		goto out;

	result = 0;

 out:

	__fd_unlock(fd);

	RETURN(result);
	return(result);
}
