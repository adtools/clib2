/*
 * $Id: stdio_fwrite.c,v 1.2 2004-12-27 09:15:55 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

size_t
fwrite(const void *ptr,size_t element_size,size_t count,FILE *stream)
{
	struct iob * file = (struct iob *)stream;
	size_t result = 0;

	ENTER();

	SHOWPOINTER(ptr);
	SHOWVALUE(element_size);
	SHOWVALUE(count);
	SHOWPOINTER(stream);

	assert( ptr != NULL && stream != NULL );
	assert( (int)element_size >= 0 && (int)count >= 0 );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(ptr == NULL || stream == NULL)
		{
			SHOWMSG("invalid parameters");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE))
	{
		SHOWMSG("this file is not even in use");

		errno = EBADF;

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_WRITE))
	{
		SHOWMSG("this stream is not write-enabled");

		errno = EBADF;

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	if(element_size > 0 && count > 0)
	{
		unsigned char * data = (unsigned char *)ptr;
		unsigned char c;
		int buffer_mode;
		size_t total_bytes_written = 0;
		size_t total_size;

		total_size = element_size * count;

		if(__fputc_check((FILE *)file) < 0)
			goto out;

		buffer_mode = (file->iob_Flags & IOBF_BUFFER_MODE);
		if(buffer_mode == IOBF_BUFFER_MODE_NONE)
		{
			if(FLAG_IS_SET(__fd[file->iob_Descriptor]->fd_Flags,FDF_IS_INTERACTIVE))
				buffer_mode = IOBF_BUFFER_MODE_LINE;
		}

		if(buffer_mode == IOBF_BUFFER_MODE_LINE)
		{
			while(total_size-- > 0)
			{
				c = (*data++);

				if(__putc_line_buffered(c,(FILE *)file) < 0)
					goto out;

				total_bytes_written++;
			}
		}
		else
		{
			while(total_size-- > 0)
			{
				c = (*data++);

				if(__putc_fully_buffered(c,(FILE *)file) < 0)
					goto out;

				total_bytes_written++;
			}
		}

		if((file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
		{
			if(__iob_write_buffer_is_valid(file) && __flush_iob_write_buffer(file) < 0)
				goto out;
		}

		result = total_bytes_written / element_size;
	}

 out:

	RETURN(result);
	return(result);
}
