/*
 * $Id: stdio_openiob.c,v 1.1.1.1 2004-07-26 16:31:37 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
__open_iob(const char *filename, const char *mode, int file_descriptor, int slot_number)
{
	ULONG file_flags;
	int result = -1;
	char actual_mode[8];
	int open_mode;
	size_t mode_len,len,i;
	struct fd * fd = NULL;
	STRPTR buffer = NULL;
	STRPTR aligned_buffer;
	struct iob * file;

	ENTER();

	SHOWSTRING(filename);
	SHOWSTRING(mode);
	SHOWVALUE(slot_number);

	assert( mode != NULL && 0 <= slot_number && slot_number < __num_iob );

	file = __iob[slot_number];

	assert( FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	/* Figure out if the file descriptor provided is any use. */
	if(file_descriptor >= 0)
	{
		assert( file_descriptor < __num_fd );
		assert( __fd[file_descriptor] != NULL );
		assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

		fd = __get_file_descriptor(file_descriptor);
		if(fd == NULL)
		{
			errno = EBADF;
			goto out;
		}
	}

	/* Options can follow the mode string, separated by a comma.
	 * We don't support any of those.
	 */
	len = mode_len = strlen(mode);
	for(i = 0 ; i < len ; i++)
	{
		if(mode[i] == ',')
		{
			mode_len = i;
			break;
		}
	}

	/* Keep only the first few letters of the mode string. */
	if(mode_len > sizeof(actual_mode)-1)
		mode_len = sizeof(actual_mode)-1;

	memmove(actual_mode,mode,mode_len);
	actual_mode[mode_len] = '\0';

	SHOWSTRING(actual_mode);

	if(strcmp(actual_mode,"r") == SAME || strcmp(actual_mode,"rb") == SAME)
	{
		SHOWMSG("read-only");

		open_mode	= O_RDONLY;
		file_flags	= IOBF_READ;
	}
	else if (strcmp(actual_mode,"w") == SAME || strcmp(actual_mode,"wb") == SAME)
	{
		SHOWMSG("write-only");

		open_mode	= O_WRONLY | O_CREAT | O_TRUNC;
		file_flags	= IOBF_WRITE;
	}
	else if (strcmp(actual_mode,"a") == SAME || strcmp(actual_mode,"ab") == SAME)
	{
		SHOWMSG("write-only; append");

		open_mode	= O_WRONLY | O_CREAT | O_APPEND;
		file_flags	= IOBF_WRITE;
	}
	else if (strcmp(actual_mode,"r+") == SAME || strcmp(actual_mode,"rb+") == SAME || strcmp(actual_mode,"r+b") == SAME)
	{
		SHOWMSG("read/write; don't overwrite");

		open_mode	= O_RDWR;
		file_flags	= IOBF_READ | IOBF_WRITE;
	}
	else if (strcmp(actual_mode,"w+") == SAME || strcmp(actual_mode,"wb+") == SAME || strcmp(actual_mode,"w+b") == SAME)
	{
		SHOWMSG("read/write; overwrite");

		open_mode	= O_RDWR | O_CREAT | O_TRUNC;
		file_flags	= IOBF_READ | IOBF_WRITE;
	}
	else if (strcmp(actual_mode,"a+") == SAME || strcmp(actual_mode,"ab+") == SAME || strcmp(actual_mode,"a+b") == SAME)
	{
		SHOWMSG("read/write; append; don't overwrite");

		open_mode	= O_RDWR | O_CREAT | O_APPEND;
		file_flags	= IOBF_READ | IOBF_WRITE;
	}
	else
	{
		SHOWMSG("unsupported file open mode");

		errno = EINVAL;
		goto out;
	}

	SHOWMSG("allocating file buffer");

	/* Allocate a little more memory than necessary. */
	buffer = malloc(BUFSIZ + 15);
	if(buffer == NULL)
	{
		SHOWMSG("that didn't work");

		errno = ENOBUFS;
		goto out;
	}

	/* Align the buffer start address to a cache line boundary. */
	aligned_buffer = (char *)((ULONG)(buffer + 15) & ~15UL);

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

	__initialize_iob(file,(HOOKFUNC)__iob_hook_entry,
		buffer,
		aligned_buffer,BUFSIZ,
		file_descriptor,
		slot_number,
		IOBF_IN_USE | file_flags | IOBF_NO_NUL);

	buffer = NULL;

	result = 0;

 out:

	if(buffer != NULL)
		free(buffer);

	RETURN(result);
	return(result);
}
