/*
 * $Id: socket_hook_entry.c,v 1.1.1.1 2004-07-26 16:31:14 obarthel Exp $
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

void
__socket_hook_entry(
	struct Hook * UNUSED		unused_hook,
	struct fd *					fd,
	struct file_hook_message *	message)
{
	struct FileInfoBlock * fib;
	int error = OK;
	int result;

	assert( message != NULL && fd != NULL );

	switch(message->action)
	{
		case file_hook_action_read:

			SHOWMSG("file_hook_action_read");

			assert( message->data != NULL );
			assert( message->size > 0 );

			SHOWVALUE(message->data);
			SHOWVALUE(message->size);

			PROFILE_OFF();

			result	= __recv((LONG)fd->fd_DefaultFile,message->data,message->size,0);
			error	= errno;

			PROFILE_ON();

			break;

		case file_hook_action_write:

			SHOWMSG("file_hook_action_write");

			assert( message->data != NULL );
			assert( message->size > 0 );

			SHOWVALUE(message->data);
			SHOWVALUE(message->size);

			PROFILE_OFF();

			result	= __send((LONG)fd->fd_DefaultFile,message->data,message->size,0);
			error	= errno;

			PROFILE_ON();

			break;

		case file_hook_action_close:

			SHOWMSG("file_hook_action_close");

			PROFILE_OFF();

			__CloseSocket((LONG)fd->fd_DefaultFile);

			PROFILE_ON();

			fd->fd_DefaultFile = -1; /* paranoia! */

			result = 0;

			break;

		case file_hook_action_duplicate_fd:

			SHOWMSG("file_hook_action_duplicate_fd");

			__duplicate_fd(message->duplicate_fd,fd);

			result = 0;

			break;

		case file_hook_action_seek:
		case file_hook_action_seek_and_extend:

			SHOWMSG("file_hook_action_seek");

			result	= -1;
			error	= ESPIPE;

			break;

		case file_hook_action_examine:

			SHOWMSG("file_hook_action_examine");

			fib = message->file_info;

			memset(fib,0,sizeof(*fib));

			fib->fib_DirEntryType = ST_SOCKET;

			PROFILE_OFF();
			DateStamp(&fib->fib_Date);
			PROFILE_ON();

			result = 0;

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

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
