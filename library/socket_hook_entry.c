/*
 * $Id: socket_hook_entry.c,v 1.18 2010-10-20 13:12:58 obarthel Exp $
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

#include <sys/ioctl.h>

/****************************************************************************/

int
__socket_hook_entry(
	struct fd *						fd,
	struct file_action_message *	fam)
{
	struct FileInfoBlock * fib;
	BOOL is_aliased;
	int result;
	int param;

	assert( fam != NULL && fd != NULL );

	/* Careful: file_action_close has to monkey with the file descriptor
	            table and therefore needs to obtain the stdio lock before
	            it locks this particular descriptor entry. */
	if(fam->fam_Action == file_action_close)
		__stdio_lock();

	__fd_lock(fd);

	switch(fam->fam_Action)
	{
		case file_action_read:

			SHOWMSG("file_action_read");

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			SHOWPOINTER(fam->fam_Data);
			SHOWVALUE(fam->fam_Size);

			PROFILE_OFF();

			result = __recv(fd->fd_Socket,fam->fam_Data,fam->fam_Size,0);
			if(result < 0)
				fam->fam_Error = __get_errno();

			PROFILE_ON();

			break;

		case file_action_write:

			SHOWMSG("file_action_write");

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			SHOWPOINTER(fam->fam_Data);
			SHOWVALUE(fam->fam_Size);

			PROFILE_OFF();

			result = __send(fd->fd_Socket,fam->fam_Data,fam->fam_Size,0);
			if(result < 0)
				fam->fam_Error = __get_errno();

			PROFILE_ON();

			break;

		case file_action_close:

			SHOWMSG("file_action_close");

			result = OK;

			/* If this is an alias, just remove it. */
			is_aliased = __fd_is_aliased(fd);
			if(is_aliased)
			{
				__remove_fd_alias(fd);
			}
			else
			{
				/* Are we permitted to close this file? */
				if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_NO_CLOSE))
				{
					PROFILE_OFF();

					result = __CloseSocket(fd->fd_Socket);

					PROFILE_ON();
				}
			}

			__fd_unlock(fd);

			#if defined(__THREAD_SAFE)
			{
				/* Free the lock semaphore now. */
				if(NOT is_aliased)
					__delete_semaphore(fd->fd_Lock);
			}
			#endif /* __THREAD_SAFE */

			/* And that's the last for this file descriptor. */
			memset(fd,0,sizeof(*fd));

			fd = NULL;

			break;

		case file_action_seek:

			SHOWMSG("file_action_seek");

			result = ERROR;

			fam->fam_Error = ESPIPE;

			break;

		case file_action_set_blocking:

			SHOWMSG("file_action_set_blocking");

			param = (int)(fam->fam_Arg == 0);

			result = __IoctlSocket(fd->fd_Socket,FIONBIO,&param);
			if(result < 0)
				fam->fam_Error = __get_errno();

			break;

		case file_action_set_async:

			SHOWMSG("file_action_set_async");

			param = (int)(fam->fam_Arg != 0);

			result = __IoctlSocket(fd->fd_Socket,FIOASYNC,&param);
			if(result < 0)
				fam->fam_Error = __get_errno();

			break;

		case file_action_examine:

			SHOWMSG("file_action_examine");

			fib = fam->fam_FileInfo;

			memset(fib,0,sizeof(*fib));

			fib->fib_DirEntryType = ST_SOCKET;

			PROFILE_OFF();
			DateStamp(&fib->fib_Date);
			PROFILE_ON();

			result = OK;

			break;

		default:

			SHOWVALUE(fam->fam_Action);

			result = ERROR;

			fam->fam_Error = EBADF;

			break;
	}

	__fd_unlock(fd);

	if(fam->fam_Action == file_action_close)
		__stdio_unlock();

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
