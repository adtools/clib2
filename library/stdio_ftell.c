/*
 * $Id: stdio_ftell.c,v 1.7 2005-02-27 18:09:10 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

long int
ftell(FILE *stream)
{
	struct iob * file = (struct iob *)stream;
	struct file_action_message fam;
	long int result = -1;
	int position;

	assert( stream != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

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

	SHOWMSG("calling the hook");

	SHOWPOINTER(&fam);

	fam.fam_Action	= file_action_seek;
	fam.fam_Offset	= 0;
	fam.fam_Mode	= SEEK_CUR;

	SHOWVALUE(fam.fam_Offset);
	SHOWVALUE(fam.fam_Mode);

	assert( file->iob_Action != NULL );

	position = (*file->iob_Action)(file,&fam);
	if(position < 0)
	{
		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(fam.fam_Error);

		goto out;
	}

	if(__iob_read_buffer_is_valid(file))
	{
		/* Subtract the number of bytes still in the buffer which have
		 * not been read before.
		 */
		position -= __iob_num_unread_bytes(file);
	}
	else if (__iob_write_buffer_is_valid(file))
	{
		/* Add the number of bytes still stored in the buffer which have
		 * not been written to disk yet.
		 */
		position += __iob_num_unwritten_bytes(file);
	}

	result = position;

 out:

	funlockfile(stream);

	return(result);
}
