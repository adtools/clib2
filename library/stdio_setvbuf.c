/*
 * $Id: stdio_setvbuf.c,v 1.6 2005-02-27 18:09:11 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
setvbuf(FILE *stream,char *buf,int bufmode,size_t size)
{
	struct iob * file = (struct iob *)stream;
	char *new_buffer = NULL;
	int result = EOF;

	ENTER();

	SHOWPOINTER(stream);
	SHOWPOINTER(buf);
	SHOWVALUE(bufmode);
	SHOWVALUE(size);

	assert( stream != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			SHOWMSG("invalid stream parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(bufmode < IOBF_BUFFER_MODE_FULL ||
	   bufmode > IOBF_BUFFER_MODE_NONE)
	{
		SHOWMSG("invalid buffer mode");

		__set_errno(EINVAL);
		goto out;
	}

	if((int)size < 0)
	{
		SHOWMSG("invalid buffer size");

		__set_errno(EINVAL);
		goto out;
	}

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE))
	{
		SHOWMSG("this file is not even in use");

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(EBADF);

		goto out;
	}

	/* A buffer size of 0 bytes defaults to unbuffered operation. */
	if(size == 0)
		bufmode = IOBF_BUFFER_MODE_NONE;

	/* If a certain buffer size is requested but no buffer was provided,
	   allocate some memory for it. */
	if(size > 0 && buf == NULL)
	{
		/* Allocate a little more memory than necessary. */
		new_buffer = malloc(size + (CACHE_LINE_SIZE-1));
		if(new_buffer == NULL)
		{
			__set_errno(ENOBUFS);
			goto out;
		}
	}

	/* Get rid of any buffered data. We're going to replace the buffer. */
	if(__iob_write_buffer_is_valid(file) && __flush_iob_write_buffer(file) < 0)
	{
		SHOWMSG("could not flush write buffer");
		goto out;
	}

	if(__iob_read_buffer_is_valid(file) && __drop_iob_read_buffer(file) < 0)
	{
		SHOWMSG("could not drop read buffer");
		goto out;
	}

	/* Get rid of any buffer specially allocated for this stream. */
	if(file->iob_CustomBuffer != NULL)
	{
		free(file->iob_CustomBuffer);
		file->iob_CustomBuffer = NULL;
	}

	if(bufmode == IOBF_BUFFER_MODE_NONE)
	{
		/* Use the default buffer, which is one byte in size. */
		new_buffer	= &file->iob_SingleByte;
		size		= 1;
	}
	else
	{
		/* Replace the old buffer. */
		if(buf != NULL)
		{
			new_buffer = (UBYTE *)buf;
		}
		else
		{
			/* Remember this, so we can release it later. */
			file->iob_CustomBuffer = new_buffer;

			/* Align the buffer start address to a cache line boundary. */
			new_buffer = (char *)((ULONG)(new_buffer + (CACHE_LINE_SIZE-1)) & ~(CACHE_LINE_SIZE-1));
		}
	}

	assert( file->iob_BufferPosition == 0 );
	assert( file->iob_BufferReadBytes == 0 );
	assert( file->iob_BufferWriteBytes == 0 );

	file->iob_Buffer		= new_buffer;
	file->iob_BufferSize	= size;

	file->iob_Flags &= ~IOBF_BUFFER_MODE;
	file->iob_Flags |= bufmode;

	new_buffer = NULL;

	result = 0;

 out:

	funlockfile(stream);

	if(new_buffer != NULL)
		free(new_buffer);

	RETURN(result);
	return(result);
}
