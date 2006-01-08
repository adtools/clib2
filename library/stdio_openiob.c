/*
 * $Id: stdio_openiob.c,v 1.14 2006-01-08 12:04:24 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

int
__open_iob(const char *filename, const char *mode, int file_descriptor, int slot_number)
{
	struct SignalSemaphore * lock;
	ULONG file_flags;
	int result = ERROR;
	int open_mode;
	struct fd * fd = NULL;
	STRPTR buffer = NULL;
	STRPTR aligned_buffer;
	struct iob * file;

	ENTER();

	SHOWSTRING(filename);
	SHOWSTRING(mode);
	SHOWVALUE(slot_number);

	if(__check_abort_enabled)
		__check_abort();

	__stdio_lock();

	assert( mode != NULL && 0 <= slot_number && slot_number < __num_iob );

	file = __iob[slot_number];

	assert( FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE) );

	/* Figure out if the file descriptor provided is any use. */
	if(file_descriptor >= 0)
	{
		assert( file_descriptor < __num_fd );
		assert( __fd[file_descriptor] != NULL );
		assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

		fd = __get_file_descriptor(file_descriptor);
		if(fd == NULL)
		{
			__set_errno(EBADF);
			goto out;
		}
	}

	/* The first character selects the access mode: read, write or append. */
	switch(mode[0])
	{
		case 'r':

			SHOWMSG("read mode");

			open_mode = O_RDONLY;
			break;

		case 'w':

			SHOWMSG("write mode");

			open_mode = O_WRONLY | O_CREAT | O_TRUNC;
			break;

		case 'a':

			SHOWMSG("append mode");

			open_mode = O_WRONLY | O_CREAT | O_APPEND;
			break;

		default:

			D(("unsupported file open mode '%lc'",mode[0]));

			__set_errno(EINVAL);
			goto out;
	}

	/* If the second or third character is a '+', switch to read/write mode. */
	if((mode[1] == '+') || (mode[1] != '\0' && mode[2] == '+'))
	{
		SHOWMSG("read/write access");

		CLEAR_FLAG(open_mode,O_RDONLY);
		CLEAR_FLAG(open_mode,O_WRONLY);

		SET_FLAG(open_mode,O_RDWR);
	}

	SHOWMSG("allocating file buffer");

	/* Allocate a little more memory than necessary. */
	buffer = malloc(BUFSIZ + (CACHE_LINE_SIZE-1));
	if(buffer == NULL)
	{
		SHOWMSG("that didn't work");

		__set_errno(ENOBUFS);
		goto out;
	}

	/* Align the buffer start address to a cache line boundary. */
	aligned_buffer = (char *)((ULONG)(buffer + (CACHE_LINE_SIZE-1)) & ~(CACHE_LINE_SIZE-1));

	if(file_descriptor < 0)
	{
		assert( filename != NULL );

		file_descriptor = open(filename,open_mode);
		if(file_descriptor < 0)
		{
			SHOWMSG("couldn't open the file");
			goto out;
		}
	}
	else
	{
		/* Update the append flag. */
		if(FLAG_IS_SET(open_mode,O_APPEND))
			SET_FLAG(fd->fd_Flags,FDF_APPEND);
		else
			CLEAR_FLAG(fd->fd_Flags,FDF_APPEND);
	}

	#if defined(__THREAD_SAFE)
	{
		/* Allocate memory for an arbitration mechanism, then
		   initialize it. */
		lock = __create_semaphore();
		if(lock == NULL)
			goto out;
	}
	#else
	{
		lock = NULL;
	}
	#endif /* __THREAD_SAFE */

	/* Figure out the buffered file access mode by looking at the open mode. */
	file_flags = IOBF_IN_USE | IOBF_NO_NUL;

	if(FLAG_IS_SET(open_mode,O_RDONLY) || FLAG_IS_SET(open_mode,O_RDWR))
		SET_FLAG(file_flags,IOBF_READ);

	if(FLAG_IS_SET(open_mode,O_WRONLY) || FLAG_IS_SET(open_mode,O_RDWR))
		SET_FLAG(file_flags,IOBF_WRITE);

	__initialize_iob(file,__iob_hook_entry,
		buffer,
		aligned_buffer,BUFSIZ,
		file_descriptor,
		slot_number,
		file_flags,
		lock);

	buffer = NULL;

	result = OK;

 out:

	if(buffer != NULL)
		free(buffer);

	__stdio_unlock();

	RETURN(result);
	return(result);
}
