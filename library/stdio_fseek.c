/*
 * $Id: stdio_fseek.c,v 1.3 2005-02-03 16:56:16 obarthel Exp $
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

int
fseek(FILE *stream, long int offset, int wherefrom)
{
	DECLARE_UTILITYBASE();
	struct iob * file = (struct iob *)stream;
	int result = -1;

	ENTER();

	SHOWPOINTER(stream);
	SHOWVALUE(offset);
	SHOWVALUE(wherefrom);

	assert(stream != NULL);
	assert( UtilityBase != NULL );

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

	if(__check_abort_enabled)
		__check_abort();

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

 	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE))
 	{
 		SHOWMSG("this file is not even in use");

		__set_errno(EBADF);
 
		SET_FLAG(file->iob_Flags,IOBF_ERROR);
 		goto out;
 	}

	if(wherefrom < SEEK_SET || wherefrom > SEEK_END)
	{
		SHOWMSG("invalid wherefrom parameter");

		__set_errno(EBADF);

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	/* Try to turn the absolute position into a relative seek
	 * within the buffer, if possible.
	 */
	if(wherefrom == SEEK_SET && __iob_read_buffer_is_valid(file))
	{
		long int current_position;

		current_position = ftell(stream);
		if(current_position >= 0)
		{
			offset -= current_position;

			wherefrom = SEEK_CUR;
		}
	}

	/* We have to clear the EOF condition */
	CLEAR_FLAG(file->iob_Flags,IOBF_EOF_REACHED);

	if(wherefrom != SEEK_CUR || offset != 0)
	{
		BOOL buffer_position_adjusted = FALSE;

		/* Try to adjust the buffer position rather than adjusting
		 * the file position itself, which is very costly.
		 */
		if(wherefrom == SEEK_CUR && __iob_read_buffer_is_valid(file))
		{
			/* Try to adjust the buffer read position. This also
			 * affects the number of bytes that can still be read.
			 */
			if ((offset < 0 && (-offset) <= file->iob_BufferPosition) ||
			    (offset > 0 && offset <= __iob_num_unread_bytes(file)))
			{
				file->iob_BufferPosition += offset;

				buffer_position_adjusted = TRUE;
			}
		}

		if(NOT buffer_position_adjusted)
		{
			struct file_hook_message message;

			/* Oh dear, no luck. So we have to get rid of the
			 * current buffer contents and start with a clean
			 * slate.
			 */
			if(__iob_write_buffer_is_valid(file) && __flush_iob_write_buffer(file) < 0)
			{
				SHOWMSG("couldn't flush write buffer");

				goto out;
			}

			if(__iob_read_buffer_is_valid(file) && __drop_iob_read_buffer(file) < 0)
			{
				SHOWMSG("couldn't drop read buffer");

				goto out;
			}

			SHOWMSG("calling the hook");

			SHOWPOINTER(&message);

			message.action		= file_hook_action_seek;
			message.position	= offset;
			message.mode		= wherefrom;
			message.result		= 0;

			SHOWVALUE(message.position);
			SHOWVALUE(message.mode);

			assert( file->iob_Hook != NULL );

			CallHookPkt(file->iob_Hook,file,&message);

			if(message.result < 0)
			{
				__set_errno(message.error);

				SET_FLAG(file->iob_Flags,IOBF_ERROR);
				goto out;
			}
		}
	}

	result = 0;

 out:

	RETURN(result);
	return(result);
}
