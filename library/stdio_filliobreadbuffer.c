/*
 * $Id: stdio_filliobreadbuffer.c,v 1.12 2006-09-22 09:02:51 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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
	struct file_action_message fam;
	int num_bytes_read;
	int result = ERROR;

	ENTER();

	SHOWPOINTER(file);

	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file != NULL && (file->iob_BufferReadBytes == 0 || file->iob_BufferPosition == file->iob_BufferReadBytes) && file->iob_BufferWriteBytes == 0 );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_READ) );
	assert( file->iob_BufferSize > 0 );

	/* Flush all line buffered streams before we proceed to fill this buffer. */
	if((file->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_LINE)
	{
		if(__flush_all_files(IOBF_BUFFER_MODE_LINE) < 0)
			goto out;
	}

	SHOWMSG("calling the hook");

	SHOWPOINTER(file->iob_Buffer);
	SHOWVALUE(file->iob_BufferSize);

	fam.fam_Action	= file_action_read;
	fam.fam_Data	= (char *)file->iob_Buffer;
	fam.fam_Size	= file->iob_BufferSize;

	assert( file->iob_Action != NULL );

	num_bytes_read = (*file->iob_Action)(file,&fam);
	if(num_bytes_read == EOF)
	{
		D(("got error %ld",fam.fam_Error));

		SET_FLAG(file->iob_Flags,IOBF_ERROR);

		__set_errno(fam.fam_Error);

		goto out;
	}

	file->iob_BufferReadBytes	= num_bytes_read;
	file->iob_BufferPosition	= 0;

	SHOWVALUE(file->iob_BufferReadBytes);

	result = OK;

 out:

	RETURN(result);
	return(result);
}
