/*
 * $Id: fcntl_lseek.c,v 1.5 2005-02-18 18:53:16 obarthel Exp $
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

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

off_t
__lseek(int file_descriptor, off_t offset, int mode, int * error_ptr)
{
	D_S(struct FileInfoBlock,fib);
	off_t result = -1;
	struct fd * fd;
	BOOL fib_is_valid = FALSE;
	LONG current_position;
	LONG new_position;
	LONG new_mode;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(offset);
	SHOWVALUE(mode);

	assert( error_ptr != NULL );
	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		(*error_ptr) = EBADF;
		goto out;
	}

	if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
	{
		SHOWMSG("can't seek on a socket");

		(*error_ptr) = ESPIPE;
		goto out;
	}

	if(mode < SEEK_SET || mode > SEEK_END)
	{
		SHOWMSG("seek mode is invalid");

		(*error_ptr) = EINVAL;
		goto out;
	}

	if(mode == SEEK_CUR)
		new_mode = OFFSET_CURRENT;
	else if (mode == SEEK_SET)
		new_mode = OFFSET_BEGINNING;
	else
		new_mode = OFFSET_END;

	D(("seek&extended to offset %ld, mode %ld; current position = %ld",offset,new_mode,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT)));

	if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
	{
		current_position = fd->fd_Position;
	}
	else
	{
		PROFILE_OFF();
		current_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
		PROFILE_ON();

		if(current_position < 0)
		{
			(*error_ptr) = EBADF;
			goto out;
		}
	}

	new_position = current_position;

	switch(new_mode)
	{
		case OFFSET_CURRENT:

			new_position += offset;
			break;

		case OFFSET_BEGINNING:

			new_position = offset;
			break;

		case OFFSET_END:

			if(__safe_examine_file_handle(fd->fd_DefaultFile,fib))
			{
				new_position = fib->fib_Size + offset;

				fib_is_valid = TRUE;
			}

			break;
	}

	if(new_position != current_position)
	{
		LONG position;

		PROFILE_OFF();
		position = Seek(fd->fd_DefaultFile,offset,new_mode);
		PROFILE_ON();

		if(position < 0)
		{
			D(("seek failed, mode=%ld (%ld), offset=%ld, ioerr=%ld",new_mode,message->mode,offset,IoErr()));

			(*error_ptr) = __translate_io_error_to_errno(IoErr());

			#if defined(UNIX_PATH_SEMANTICS)
			{
				if(NOT fib_is_valid && CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,fib))
				{
					(*error_ptr) = __translate_io_error_to_errno(IoErr());
					goto out;
				}

				if(new_position <= fib->fib_Size)
				{
					(*error_ptr) = __translate_io_error_to_errno(IoErr());
					goto out;
				}

				if(__grow_file_size(fd,new_position - fib->fib_Size) != OK)
				{
					(*error_ptr) = __translate_io_error_to_errno(IoErr());
					goto out;
				}
			}
			#else
			{
				(*error_ptr) = __translate_io_error_to_errno(IoErr());
				goto out;
			}
			#endif /* UNIX_PATH_SEMANTICS */
		}
	}

	if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
		fd->fd_Position = new_position;

	result = new_position;

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

off_t
lseek(int file_descriptor, off_t offset, int mode)
{
	off_t result;
	int error;

	ENTER();

	result = __lseek(file_descriptor,offset,mode,&error);
	__set_errno(error);

	RETURN(result);
	return(result);
}
