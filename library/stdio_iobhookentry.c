/*
 * $Id: stdio_iobhookentry.c,v 1.7 2006-01-08 12:04:24 obarthel Exp $
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
__iob_hook_entry(
	struct iob *					file_iob,
	struct file_action_message *	fam)
{
	struct fd * fd;
	int result;

	assert( fam != NULL && file_iob != NULL );

	switch(fam->fam_Action)
	{
		case file_action_read:
		case file_action_write:
		case file_action_seek:
		case file_action_close:

			assert( file_iob->iob_Descriptor >= 0 && file_iob->iob_Descriptor < __num_fd );
			assert( __fd[file_iob->iob_Descriptor] != NULL );
			assert( FLAG_IS_SET(__fd[file_iob->iob_Descriptor]->fd_Flags,FDF_IN_USE) );

			/* When closing, we want to affect this very file descriptor
			   and not the original one associated with an alias of it. */
			if(fam->fam_Action == file_action_close)
				fd = __get_file_descriptor_dont_resolve(file_iob->iob_Descriptor);
			else
				fd = __get_file_descriptor(file_iob->iob_Descriptor);

			if(fd == NULL)
			{
				fam->fam_Error = EBADF;

				result = EOF;

				break;
			}

			assert( fd->fd_Action != NULL );

			result = (*fd->fd_Action)(fd,fam);

			break;

		default:

			SHOWVALUE(fam->fam_Action);

			fam->fam_Error = EBADF;

			result = EOF;

			break;
	}

	RETURN(result);
	return(result);
}
