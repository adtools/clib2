/*
 * $Id: stdio_iobhookentry.c,v 1.1.1.1 2004-07-26 16:31:36 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

void
__iob_hook_entry(
	struct Hook *				UNUSED	unused_hook,
	struct iob *						file_iob,
	struct file_hook_message *			message)
{
	int result;
	int error;

	assert( message != NULL && file_iob != NULL );

	switch(message->action)
	{
		case file_hook_action_read:

			SHOWMSG("file_hook_action_read");

			SHOWVALUE(file_iob->iob_Descriptor);
			SHOWPOINTER(message->data);
			SHOWVALUE(message->size);

			assert( message->data != NULL );
			assert( message->size > 0 );
			assert( FLAG_IS_SET(file_iob->iob_Flags,IOBF_IN_USE) );

			result = __read(file_iob->iob_Descriptor,message->data,(size_t)message->size,&error);
			break;

		case file_hook_action_write:

			SHOWMSG("file_hook_action_write");

			SHOWVALUE(file_iob->iob_Descriptor);
			SHOWPOINTER(message->data);
			SHOWVALUE(message->size);

			assert( message->data != NULL );
			assert( message->size > 0 );
			assert( FLAG_IS_SET(file_iob->iob_Flags,IOBF_IN_USE) );

			result = __write(file_iob->iob_Descriptor,message->data,(size_t)message->size,&error);

			break;

		case file_hook_action_seek:

			SHOWMSG("file_hook_action_seek");

			SHOWVALUE(file_iob->iob_Descriptor);
			SHOWVALUE(message->position);
			SHOWVALUE(message->mode);

			assert( FLAG_IS_SET(file_iob->iob_Flags,IOBF_IN_USE) );

			result = __lseek(file_iob->iob_Descriptor,message->position,message->mode,&error);
			break;

		case file_hook_action_close:

			SHOWMSG("file_hook_action_close");

			SHOWVALUE(file_iob->iob_Descriptor);

			assert( FLAG_IS_SET(file_iob->iob_Flags,IOBF_IN_USE) );

			result = __close(file_iob->iob_Descriptor,&error);
			break;

		default:

			SHOWVALUE(message->action);

			result	= -1;
			error	= EBADF;

			break;
	}

	SHOWVALUE(result);

	message->result	= result;
	message->error	= error;
}
