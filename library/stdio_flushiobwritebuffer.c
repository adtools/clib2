/*
 * $Id: stdio_flushiobwritebuffer.c,v 1.9 2006-09-22 09:02:51 obarthel Exp $
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

/* This would otherwise generate far too much (useless) debug output. */
#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
__flush_iob_write_buffer(struct iob * file)
{
	int result = OK;

	ENTER();

	SHOWPOINTER(file);

	assert( file != NULL );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(__check_abort_enabled)
		__check_abort();

	if(FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) && file->iob_BufferWriteBytes > 0)
	{
		struct file_action_message fam;

		assert( FLAG_IS_SET(file->iob_Flags,IOBF_WRITE) );
		assert( file->iob_BufferSize > 0 );

		D(("%ld bytes are to be written",file->iob_BufferWriteBytes));

		SHOWMSG("calling the hook");

		fam.fam_Action	= file_action_write;
		fam.fam_Data	= (char *)file->iob_Buffer;
		fam.fam_Size	= file->iob_BufferWriteBytes;

		assert( file->iob_Action != NULL );

		if((*file->iob_Action)(file,&fam) == EOF)
		{
			SHOWMSG("that didn't work");

			result = ERROR;

			SET_FLAG(file->iob_Flags,IOBF_ERROR);

			__set_errno(fam.fam_Error);

			goto out;
		}

		file->iob_BufferWriteBytes = 0;
	}

 out:

	RETURN(result);
	return(result);
}
