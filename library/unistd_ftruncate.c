/*
 * $Id: unistd_ftruncate.c,v 1.11 2005-04-24 08:46:37 obarthel Exp $
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
	int result = ERROR;
	struct fd * fd = NULL;
	BOOL restore_initial_position = FALSE;
	off_t current_file_size;
	off_t initial_position = -1;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(length);

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	PROFILE_OFF();

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

	/* Figure out how large the file is right now. */
	if(CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,fib))
	{
		SHOWMSG("couldn't examine file");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	current_file_size = fib->fib_Size;

	/* Is the file to be made shorter than it is right now? */
	if(length < current_file_size)
	{
		/* Remember where we started. */
		if(initial_position < 0)
		{
			initial_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
			if(initial_position == SEEK_ERROR)
				goto out;
		}

		/* Careful: seek to a position where the file can be safely truncated. */
		if(Seek(fd->fd_DefaultFile,length,OFFSET_BEGINNING) == SEEK_ERROR)
		{
			D(("could not move to file offset %ld",length));

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		if(SetFileSize(fd->fd_DefaultFile,length,OFFSET_BEGINNING) == SEEK_ERROR)
		{
			D(("could not reduce file to size %ld",length));

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		/* If the file is now shorter than the file position, which must
		   not be changed by a call to ftruncate(), extend the file again,
		   filling the extension with 0 bytes. */
		if(initial_position > length)
		{
			current_file_size = length;

			length = initial_position;
		}

		restore_initial_position = TRUE;
	}

	/* Is the size of the file to grow? */
	if(length > current_file_size)
	{
		/* Remember where we started. */
		if(initial_position < 0)
		{
			initial_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
			if(initial_position == SEEK_ERROR)
				goto out;
		}

		/* Move to what should be the end of the file. */
		if(Seek(fd->fd_DefaultFile,current_file_size,OFFSET_BEGINNING) == SEEK_ERROR)
		{
			D(("could not move to file offset %ld",current_file_size));

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		/* Add as many bytes to the end of the file as are required
		   to make it as large as requested. */
		if(__grow_file_size(fd,length - current_file_size) != OK)
		{
			D(("could not extend file to size %ld",length));

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		restore_initial_position = TRUE;
	}

	result = OK;

 out:

	if(restore_initial_position)
		Seek(fd->fd_DefaultFile,initial_position,OFFSET_CURRENT);

	__fd_unlock(fd);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
