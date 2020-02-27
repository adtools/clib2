/*
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2019 by Olaf Barthel <obarthel (at) gmx.net>
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

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(ptr == NULL || stream == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE))
	{
		SHOWMSG("this file is not even in use");

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(EBADF);

		goto out;
	}

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_WRITE))
	{
		SHOWMSG("this stream is not write-enabled");

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(EBADF);

		goto out;
	}

	clearerr((FILE *)file);

	if(element_size > 0 && count > 0)
	{
		const unsigned char * s = (unsigned char *)ptr;
		unsigned char c;
		int buffer_mode;
		size_t total_bytes_written = 0;
		size_t total_size;

		total_size = element_size * count;

		if(__fputc_check((FILE *)file) < 0)
			goto out;

		/* If this is an unbuffered interactive stream, we will switch
		   to line buffered mode in order to improve readability of
		   the output. */
		buffer_mode = (file->iob_Flags & IOBF_BUFFER_MODE);
		if(buffer_mode == IOBF_BUFFER_MODE_NONE)
		{
			struct fd * fd = __fd[file->iob_Descriptor];

			__fd_lock(fd);

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
				buffer_mode = IOBF_BUFFER_MODE_LINE;

			__fd_unlock(fd);
		}

		if (buffer_mode == IOBF_BUFFER_MODE_LINE)
		{
			assert( file->iob_BufferSize > 0 );

			while(total_size > 0)
			{
				/* Is there still room in the write buffer to store
				   more of the data? */
				if(file->iob_BufferWriteBytes < file->iob_BufferSize)
				{
					unsigned char * buffer = &file->iob_Buffer[file->iob_BufferWriteBytes];
					size_t num_buffer_bytes;
					const unsigned char * lf;

					/* Store only as many characters as will fit into the write buffer. */
					num_buffer_bytes = file->iob_BufferSize - file->iob_BufferWriteBytes;
					if(total_size < num_buffer_bytes)
						num_buffer_bytes = total_size;

					/* Try to find a line feed in the string. If there is one,
					   reduce the number of characters to write to the sequence
					   which ends with the line feed character. */
					lf = (unsigned char *)memchr(s, '\n', num_buffer_bytes);
					if(lf != NULL)
						num_buffer_bytes = lf + 1 - s;

					memmove(buffer, s, num_buffer_bytes);
					s += num_buffer_bytes;

					file->iob_BufferWriteBytes += num_buffer_bytes;

					/* Write the buffer to disk if it's full or contains a line feed. */
					if((lf != NULL || __iob_write_buffer_is_full(file)) && __flush_iob_write_buffer(file) < 0)
					{
						/* Abort with error. */
						break;
					}

					total_bytes_written += num_buffer_bytes;

					/* Stop as soon as no further data needs to be written. */
					total_size -= num_buffer_bytes;
					if(total_size == 0)
						break;

					/* If there is again room in the output buffer,
					   repeat this optimization. */
					if(file->iob_BufferWriteBytes < file->iob_BufferSize)
						continue;
				}

				c = (*s++);

				if(__putc_line_buffered(c,(FILE *)file) == EOF)
					break;

				total_size--;
				total_bytes_written++;
			}
		}
		else if (buffer_mode == IOBF_BUFFER_MODE_NONE)
		{
			ssize_t num_bytes_written;

			/* We bypass the buffer entirely. */
			num_bytes_written = write(file->iob_Descriptor, s, total_size);
			if(num_bytes_written == -1)
			{
				SET_FLAG(file->iob_Flags,IOBF_ERROR);
				goto out;
			}

			total_bytes_written = (size_t)num_bytes_written;
		}
		else
		{
			assert( file->iob_BufferSize > 0 );

			while(total_size > 0)
			{
				/* If there is more data to be written than the write buffer will hold
				   and the write buffer is empty anyway, then we'll bypass the write
				   buffer entirely. */
				if(file->iob_BufferWriteBytes == 0 && total_size >= (size_t)file->iob_BufferSize)
				{
					ssize_t num_bytes_written;

					/* We bypass the buffer entirely. */
					num_bytes_written = write(file->iob_Descriptor, s, total_size);
					if(num_bytes_written == -1)
					{
						SET_FLAG(file->iob_Flags,IOBF_ERROR);
						goto out;
					}

					total_bytes_written += num_bytes_written;
					break;
				}

				/* Is there still room in the write buffer to store
				   more of the data? */
				if(file->iob_BufferWriteBytes < file->iob_BufferSize)
				{
					unsigned char * buffer = &file->iob_Buffer[file->iob_BufferWriteBytes];
					size_t num_buffer_bytes;

					/* Store only as many bytes as will fit into the write buffer. */
					num_buffer_bytes = file->iob_BufferSize - file->iob_BufferWriteBytes;
					if(total_size < num_buffer_bytes)
						num_buffer_bytes = total_size;

					memmove(buffer, s, num_buffer_bytes);
					s += num_buffer_bytes;

					file->iob_BufferWriteBytes += num_buffer_bytes;

					/* Write a full buffer to disk. */
					if(__iob_write_buffer_is_full(file) && __flush_iob_write_buffer(file) < 0)
					{
						/* Abort with error. */
						break;
					}

					total_bytes_written += num_buffer_bytes;

					/* Stop as soon as no further data needs to be written. */
					total_size -= num_buffer_bytes;
					if(total_size == 0)
						break;

					/* If there is again room in the output buffer,
					   try this optimization again. */
					if(file->iob_BufferWriteBytes < file->iob_BufferSize)
						continue;
				}

				c = (*s++);

				if(__putc_fully_buffered(c,(FILE *)file) == EOF)
					break;

				total_size--;
				total_bytes_written++;
			}
		}
		
		if((file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
		{
			if(__iob_write_buffer_is_valid(file))
				__flush_iob_write_buffer(file);
		}

		result = total_bytes_written / element_size;
	}
	else
	{
		SHOWVALUE(element_size);
		SHOWVALUE(count);

		SHOWMSG("either element size or count is zero");
	}

 out:

	funlockfile(stream);

	RETURN(result);
	return(result);
}
