/*
 * $Id: stdio_dropiobreadbuffer.c,v 1.9 2006-01-08 12:04:24 obarthel Exp $
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
__drop_iob_read_buffer(struct iob * file)
{
	int result = OK;

	ENTER();

	SHOWPOINTER(file);

	assert( file != NULL );

	if(__check_abort_enabled)
		__check_abort();

	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE))
	{
		CLEAR_FLAG(file->iob_Flags,IOBF_EOF_REACHED);

		if(__iob_read_buffer_is_valid(file))
		{
			LONG num_unread_bytes;

			num_unread_bytes = __iob_num_unread_bytes(file);

			D(("%ld bytes are to be dropped",num_unread_bytes));

			if(num_unread_bytes > 0)
			{
				struct file_action_message fam;
				LONG position;

				SHOWMSG("calling the action function");

				fam.fam_Action	= file_action_seek;
				fam.fam_Offset	= -num_unread_bytes;
				fam.fam_Mode	= SEEK_CUR;

				assert( file->iob_Action != NULL );

				/* Note that a return value of -1 (= SEEK_ERROR) may be a
				   valid file position in files larger than 2 GBytes. Just
				   to be sure, we therefore also check the secondary error
				   to verify that what could be a file position is really
				   an error indication. */
				position = (*file->iob_Action)(file,&fam);
				if(position == SEEK_ERROR && fam.fam_Error != OK)
				{
					SHOWMSG("that didn't work");

					result = ERROR;

					SET_FLAG(file->iob_Flags,IOBF_ERROR);

					__set_errno(fam.fam_Error);

					goto out;
				}

			 	/* If this is a valid file position, clear 'errno' so that
				   it cannot be mistaken for an error. */
				if(position < 0)
					__set_errno(OK);
			}

			file->iob_BufferReadBytes	= 0;
			file->iob_BufferPosition	= 0;
		}
	}

 out:

	RETURN(result);
	return(result);
}
