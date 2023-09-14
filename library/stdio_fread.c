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
fread(void *ptr,size_t element_size,size_t count,FILE *stream)
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

	if (__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if (ptr == NULL || stream == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if (FLAG_IS_CLEAR(file->iob_Flags, IOBF_IN_USE))
	{
		SHOWMSG("this file is not even in use");

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(EBADF);

		goto out;
	}

	if (FLAG_IS_CLEAR(file->iob_Flags, IOBF_READ))
	{
		SHOWMSG("this file is not read-enabled");

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(EBADF);

		goto out;
	}

	/* So that we can tell error and 'end of file' conditions apart. */
	clearerr(stream);

	if (element_size > 0 && count > 0)
	{
		size_t total_bytes_read = 0;
		size_t total_size;
		unsigned char * data = ptr;
		ssize_t num_bytes_read;
		int c;

		if (__fgetc_check((FILE *)file) < 0)
			goto out;

		/* Check for overflow. */
		total_size = element_size * count;
		if (element_size != (total_size / count))
			goto out;

		SHOWVALUE(total_size);

		/* No buffering enabled? */
		if ((file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
		{
			/* We bypass the buffer entirely. */
			num_bytes_read = read(file->iob_Descriptor, data, total_size);
			if (num_bytes_read == -1)
			{
				SET_FLAG(file->iob_Flags,IOBF_ERROR);
				goto out;
			}

			if (num_bytes_read == 0)
				SET_FLAG(file->iob_Flags, IOBF_EOF_REACHED);

			total_bytes_read = num_bytes_read;
		}
		else
		{
			while (total_size > 0)
			{
				/* If the read buffer is empty and there is more data to be
				 * read, we'll bypass the buffer entirely. Note that we try
				 * to read a complete full buffer worth's of data, not just
				 * a few bytes.
				 */
				if (file->iob_BufferReadBytes == 0 && total_size >= (size_t)file->iob_BufferSize)
				{
					/* We bypass the buffer entirely. */
					num_bytes_read = read(file->iob_Descriptor, data, total_size);
					if (num_bytes_read == -1)
					{
						SET_FLAG(file->iob_Flags, IOBF_ERROR);
						goto out;
					}

					if (num_bytes_read == 0)
						SET_FLAG(file->iob_Flags, IOBF_EOF_REACHED);

					total_bytes_read += num_bytes_read;
					break;
				}

				/* If there is still data in the read buffer, try to copy it
				 * directly into the output buffer.
				 */
				if (file->iob_BufferPosition < file->iob_BufferReadBytes)
				{
					const unsigned char * buffer = &file->iob_Buffer[file->iob_BufferPosition];
					size_t num_bytes_in_buffer;

					/* Copy as much data as will fit. */
					assert( file->iob_BufferReadBytes >= file->iob_BufferPosition );

					num_bytes_in_buffer = file->iob_BufferReadBytes - file->iob_BufferPosition;
					if (total_size < num_bytes_in_buffer)
						num_bytes_in_buffer = total_size;

					assert( num_bytes_in_buffer > 0 );

					memmove(data, buffer, num_bytes_in_buffer);
					data += num_bytes_in_buffer;

					assert( file->iob_BufferPosition + num_bytes_in_buffer <= file->iob_BufferSize );

					file->iob_BufferPosition += num_bytes_in_buffer;

					total_bytes_read += num_bytes_in_buffer;

					/* Stop if the string buffer has been filled. */
					assert( total_size >= num_bytes_in_buffer );

					/* Are we finished yet? */
					total_size -= num_bytes_in_buffer;
					if (total_size == 0)
						break;
				}

				/* Read the next byte and/or refill the read buffer. */
				c = __getc(file);
				if (c == EOF)
					break;

				(*data++) = c;

				total_size--;
				total_bytes_read++;
			}
		}

		SHOWVALUE(total_bytes_read);

		result = total_bytes_read / element_size;
	}
	else
	{
		SHOWVALUE(element_size);
		SHOWVALUE(count);

		SHOWMSG("either element size or count is zero");
	}

	D(("total number of elements read = %ld",result));

 out:

	funlockfile(stream);

	RETURN(result);
	return(result);
}
