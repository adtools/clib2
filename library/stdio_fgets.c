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

char *
fgets(char *s,int n,FILE *stream)
{
	struct iob * file = (struct iob *)stream;
	char * result = s;
	int c;

	ENTER();

	SHOWPOINTER(s);
	SHOWVALUE(n);
	SHOWPOINTER(stream);

	assert( s != NULL && stream != NULL );

	if (__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if (s == NULL || stream == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			result = NULL;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if (n <= 0)
	{
		SHOWMSG("no work to be done");

		result = NULL;
		goto out;
	}

	/* Take care of the checks and data structure changes that
	 * need to be handled only once for this stream.
	 */
	if (__fgetc_check(stream) < 0)
	{
		result = NULL;
		goto out;
	}

	/* So that we can tell error and 'end of file' conditions apart. */
	clearerr(stream);

	/* One off for the terminating '\0'. */
	n--;

	assert( 0 <= file->iob_BufferReadBytes );
	assert( file->iob_BufferReadBytes <= file->iob_BufferSize );
	assert( file->iob_BufferPosition <= file->iob_BufferSize );

	while (n > 0)
	{
		/* If there is data in the buffer, try to copy it directly
		 * into the string buffer. If there is a line feed in the
		 * buffer, too, try to conclude the read operation.
		 */
		if (file->iob_BufferPosition < file->iob_BufferReadBytes)
		{
			const unsigned char * buffer = &file->iob_Buffer[file->iob_BufferPosition];
			size_t num_bytes_in_buffer;
			const unsigned char * lf;

			/* Copy only as much data as will fit into the string buffer. */
			assert( file->iob_BufferReadBytes >= file->iob_BufferPosition );

			num_bytes_in_buffer = file->iob_BufferReadBytes - file->iob_BufferPosition;
			if (num_bytes_in_buffer > (size_t)n)
				num_bytes_in_buffer = n;

			/* Try to find a line feed character which could conclude
			 * the read operation if the remaining buffer data, including
			 * the line feed character, fit into the string buffer.
			 */
			lf = (unsigned char *)memchr(buffer, '\n', num_bytes_in_buffer);
			if(lf != NULL)
			{
				size_t num_characters_in_line = lf + 1 - buffer;

				/* Give the user a chance to abort what could otherwise
				 * become an uninterrupted series of copying operations.
				 */
				if (__check_abort_enabled)
					__check_abort();

				assert( num_characters_in_line <= num_bytes_in_buffer );

				/* Copy the remainder of the read buffer into the
				 * string buffer, including the terminating line
				 * feed character.
				 */
				memmove(s, buffer, num_characters_in_line);
				s += num_characters_in_line;

				assert( file->iob_BufferPosition + num_characters_in_line <= file->iob_BufferSize );

				file->iob_BufferPosition += num_characters_in_line;

				/* And that concludes the line read operation. */
				break;
			}

			memmove(s, buffer, num_bytes_in_buffer);
			s += num_bytes_in_buffer;

			assert( file->iob_BufferPosition + num_bytes_in_buffer <= file->iob_BufferSize );

			file->iob_BufferPosition += num_bytes_in_buffer;

			/* Stop if the string buffer has been filled. */
			assert( n >= num_bytes_in_buffer );

			n -= num_bytes_in_buffer;
			if (n == 0)
				break;
		}

		/* Read the next buffered character; this will refill the read
		 * buffer, if necessary.
		 */
		c = __getc(stream);
		if (c == EOF)
		{
			if (ferror(stream))
			{
				/* Just to be on the safe side. */
				(*s) = '\0';

				result = NULL;
				goto out;
			}

			/* Make sure that we return NULL if we really
			 * didn't read anything at all.
			 */
			if (s == result)
				result = NULL;

			break;
		}

		(*s++) = c;

		if (c == '\n')
			break;

		assert( n > 0 );
		n--;
	}

	(*s) = '\0';

	SHOWSTRING(result);

 out:

	funlockfile(stream);

	RETURN(result);
	return(result);
}
