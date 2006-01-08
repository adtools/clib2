/*
 * $Id: stdio_puts.c,v 1.8 2006-01-08 12:04:25 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
puts(const char *s)
{
	struct iob * file = (struct iob *)stdout;
	int result = EOF;
	int buffer_mode;
	int c;

	ENTER();

	SHOWSTRING(s);

	assert( s != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stdout);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(s == NULL)
		{
			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	buffer_mode = (file->iob_Flags & IOBF_BUFFER_MODE);
	if(buffer_mode == IOBF_BUFFER_MODE_NONE)
		buffer_mode = IOBF_BUFFER_MODE_LINE;

	if(__fputc_check(stdout) < 0)
		goto out;

	while((c = (*s++)) != '\0')
	{
		if(__putc(c,stdout,buffer_mode) == EOF)
			goto out;
	}

	if(__putc('\n',stdout,buffer_mode) == EOF)
		goto out;

	result = OK;

 out:

	/* Note: if buffering is disabled for this stream, then we still
	   may have buffered data around, queued to be printed right now.
	   This is intended to improve performance as it takes more effort
	   to write a single character to a file than to write a bunch. */
	if(result == 0 && (file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
	{
		if(__iob_write_buffer_is_valid(file) && __flush_iob_write_buffer(file) < 0)
		{
			SHOWMSG("couldn't flush the write buffer");
			result = EOF;
		}
	}

	funlockfile(stdout);

	RETURN(result);
	return(result);
}
