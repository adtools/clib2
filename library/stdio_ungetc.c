/*
 * $Id: stdio_ungetc.c,v 1.1.1.1 2004-07-26 16:31:43 obarthel Exp $
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

int
ungetc(int c,FILE *stream)
{
	struct iob * file = (struct iob *)stream;
	int result = EOF;

	assert( stream != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			SHOWMSG("null file pointer!");

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

	/* Pushing back characters only works for files that can be read from. */
	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_READ))
	{
		SHOWMSG("can't even read from this file");

		errno = EACCES;

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	/* Pushing back an EOF is forbidden. */
	if(c == EOF)
	{
		SHOWMSG("cannot push back an EOF");

		errno = EINVAL;

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	/* The following should never happen. */
	if(c < 0)
		D(("warning -- pushback of negative number %ld!",c));

	/* Get rid of the write buffer, if it's still around. */
	if(__iob_write_buffer_is_valid(file) > 0 && __flush_iob_write_buffer(file) < 0)
	{
		SHOWMSG("could not flush write buffer");
		goto out;
	}

	/* We need to replace one of the characters in the buffer, which must
	 * have been read before. The ISO standard requires that it must
	 * be possible to push back at least one single character.
	 */
	if(file->iob_BufferPosition == 0)
	{
		SHOWMSG("no room to push back");

		errno = ENOBUFS;

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	CLEAR_FLAG(file->iob_Flags,IOBF_EOF_REACHED);

	/* Replace the character just read. */
	file->iob_Buffer[--file->iob_BufferPosition] = c;

	result = c;

 out:

	return(result);
}
