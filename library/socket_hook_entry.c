/*
 * $Id: socket_hook_entry.c,v 1.11 2005-03-03 14:20:55 obarthel Exp $
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
	int result;
	int param;

	assert( fam != NULL && fd != NULL );

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

			result = __recv((LONG)fd->fd_DefaultFile,fam->fam_Data,fam->fam_Size,0);
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

			result = __send((LONG)fd->fd_DefaultFile,fam->fam_Data,fam->fam_Size,0);
			if(result < 0)
				fam->fam_Error = __get_errno();

			PROFILE_ON();

			break;

		case file_action_close:

			SHOWMSG("file_action_close");

			/* If this is an alias, just remove it. */
			if(__fd_is_aliased(fd))
			{
				__remove_fd_alias(fd);
			}
			else
			{
				/* Are we permitted to close this file? */
				if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_NO_CLOSE))
				{
					PROFILE_OFF();

					__CloseSocket((LONG)fd->fd_DefaultFile);

					PROFILE_ON();
				}
			}

			__fd_unlock(fd);

			/* Free the lock semaphore now. */
			__delete_semaphore(fd->fd_Lock);

			/* And that's the last for this file descriptor. */
			memset(fd,0,sizeof(*fd));

			result = 0;

			break;

		case file_action_seek:

			SHOWMSG("file_action_seek");

			result = -1;

			fam->fam_Error = ESPIPE;

			break;

		case file_action_set_blocking:

			SHOWMSG("file_action_set_blocking");

			param = (int)(fam->fam_Arg == 0);

			result = __IoctlSocket(fd->fd_DefaultFile,FIONBIO,&param);
			if(result < 0)
				fam->fam_Error = __get_errno();

			break;

		case file_action_set_async:

			SHOWMSG("file_action_set_async");

			param = (int)(fam->fam_Arg != 0);

			result = __IoctlSocket(fd->fd_DefaultFile,FIOASYNC,&param);
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

			result = 0;

			break;

		default:

			SHOWVALUE(fam->fam_Action);

			result = -1;

			fam->fam_Error = EBADF;

			break;
	}

	__fd_unlock(fd);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
