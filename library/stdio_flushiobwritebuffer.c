/*
 * $Id: stdio_flushiobwritebuffer.c,v 1.1.1.1 2004-07-26 16:31:30 obarthel Exp $
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

/* This would otherwise generate far too much (useless) debug output. */
#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/*
int
__iob_write_buffer_is_full(struct iob * file)
{
	int result;

	assert( file != NULL );

	result = (file->iob_BufferSize > 0 && (ULONG)file->iob_BufferWriteBytes == file->iob_BufferSize);

	return(result);
}
*/

/****************************************************************************/

int
__flush_iob_write_buffer(struct iob * file)
{
	DECLARE_UTILITYBASE();
	int result = 0;

	ENTER();

	SHOWPOINTER(file);

	assert( file != NULL );
	assert( UtilityBase != NULL );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(__check_abort_enabled)
		__check_abort();

	if(FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) && file->iob_BufferWriteBytes > 0)
	{
		struct file_hook_message message;

		assert( FLAG_IS_SET(file->iob_Flags,IOBF_WRITE) );
		assert( file->iob_BufferSize > 0 );

		D(("%ld bytes are to be written",file->iob_BufferWriteBytes));

		SHOWMSG("calling the hook");

		message.action	= file_hook_action_write;
		message.data	= file->iob_Buffer;
		message.size	= file->iob_BufferWriteBytes;
		message.result	= 0;

		assert( file->iob_Hook != NULL );

		CallHookPkt(file->iob_Hook,file,&message);

		if(message.result != file->iob_BufferWriteBytes)
		{
			SHOWMSG("that didn't work");

			SET_FLAG(file->iob_Flags,IOBF_ERROR);

			result	= -1;
			errno	= message.error;

			goto out;
		}

		file->iob_BufferWriteBytes = 0;
	}

 out:

	RETURN(result);
	return(result);
}
