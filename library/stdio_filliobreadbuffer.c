/*
 * $Id: stdio_filliobreadbuffer.c,v 1.4 2005-02-03 16:56:16 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
__fill_iob_read_buffer(struct iob * file)
{
	DECLARE_UTILITYBASE();
	struct file_hook_message message;
	int result = -1;

	ENTER();

	SHOWPOINTER(file);

	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file != NULL && (file->iob_BufferReadBytes == 0 || file->iob_BufferPosition == file->iob_BufferReadBytes) && file->iob_BufferWriteBytes == 0 );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_READ) );
	assert( file->iob_BufferSize > 0 );
	assert( UtilityBase != NULL );

	if(__check_abort_enabled)
		__check_abort();

	/* Flush all line buffered streams before we proceed to fill this buffer. */
	if((file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_LINE)
	{
		int i;

		for(i = 0 ; i < __num_iob ; i++)
		{
			if(__iob[i] != NULL &&
			   FLAG_IS_SET(__iob[i]->iob_Flags,IOBF_IN_USE) &&
			   FLAG_IS_SET(__iob[i]->iob_Flags,IOBF_WRITE) &&
			   (__iob[i]->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_LINE && 
			   __iob_write_buffer_is_valid(__iob[i]))
			{
				if(__flush_iob_write_buffer(__iob[i]) < 0)
					goto out;
			}
		}
	}

	SHOWMSG("calling the hook");

	SHOWPOINTER(file->iob_Buffer);
	SHOWVALUE(file->iob_BufferSize);

	message.action	= file_hook_action_read;
	message.data	= file->iob_Buffer;
	message.size	= file->iob_BufferSize;
	message.result	= 0;

	assert( file->iob_Hook != NULL );

	CallHookPkt(file->iob_Hook,file,&message);

	if(message.result < 0)
	{
		__set_errno(message.error);

		D(("got error %ld",message.error));

		SET_FLAG(file->iob_Flags,IOBF_ERROR);
		goto out;
	}

	file->iob_BufferReadBytes	= message.result;
	file->iob_BufferPosition	= 0;

	SHOWVALUE(file->iob_BufferReadBytes);

	result = 0;

 out:

	RETURN(result);
	return(result);
}
