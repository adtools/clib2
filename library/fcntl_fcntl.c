/*
 * $Id: fcntl_fcntl.c,v 1.9 2005-02-20 13:19:40 obarthel Exp $
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

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
fcntl(int file_descriptor, int cmd, ... /* int arg */ )
{
	struct file_action_message fam;
	struct flock * l;
	int vacant_slot;
	int result = -1;
	struct fd * fd;
	va_list arg;
	int error;
	int flags;
	int fdbase;
	int i;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(cmd);

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		__set_errno(EBADF);
		goto out;
	}

	switch(cmd)
	{
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:

			SHOWMSG("cmd=F_GETLK/F_SETLK/F_SETLKW");

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
			{
				__set_errno(EINVAL);
				goto out;
			}

			va_start(arg,cmd);
			l = va_arg(arg,struct flock *);
			va_end(arg);

			assert( l != NULL );

			if(l->l_type < F_RDLCK || l->l_type > F_WRLCK)
			{
				SHOWMSG("invalid flock type");

				__set_errno(EINVAL);
				break;
			}

			if(l->l_whence < SEEK_SET || l->l_whence > SEEK_END)
			{
				SHOWMSG("invalid flock offset");

				__set_errno(EINVAL);
				break;
			}

			if(__handle_record_locking(cmd,l,fd,&error) < 0)
			{
				__set_errno(error);
				goto out;
			}

			result = 0;

			break;

		case F_GETFL:

			SHOWMSG("cmd=F_GETFL");

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
			{
				__set_errno(EINVAL);
				goto out;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING))
				SET_FLAG(result,O_NONBLOCK);

			if(FLAG_IS_SET(fd->fd_Flags,FDF_ASYNC_IO))
				SET_FLAG(result,O_ASYNC);

			result = 0;

			break;

		case F_SETFL:

			SHOWMSG("cmd=F_SETFL");

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
			{
				__set_errno(EINVAL);
				goto out;
			}

			va_start(arg,cmd);
			flags = va_arg(arg,int);
			va_end(arg);

			if((FLAG_IS_SET(flags,O_NONBLOCK) && FLAG_IS_CLEAR(fd->fd_Flags,FDF_NON_BLOCKING)) ||
			   (FLAG_IS_CLEAR(flags,O_NONBLOCK) && FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING)))
			{
				fam.fam_Action	= file_action_set_blocking;
				fam.fam_Arg		= FLAG_IS_CLEAR(flags,O_NONBLOCK);

				assert( fd->fd_Action != NULL );

				if((*fd->fd_Action)(fd,&fam) < 0)
				{
					__set_errno(fam.fam_Error);

					goto out;
				}

				if(FLAG_IS_SET(flags,O_NONBLOCK))
					SET_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
				else
					CLEAR_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
			}

			if((FLAG_IS_SET(flags,O_ASYNC) && FLAG_IS_CLEAR(fd->fd_Flags,FDF_ASYNC_IO)) ||
			   (FLAG_IS_CLEAR(flags,O_ASYNC) && FLAG_IS_SET(fd->fd_Flags,FDF_ASYNC_IO)))
			{
				fam.fam_Action	= file_action_set_async;
				fam.fam_Arg		= FLAG_IS_SET(flags,O_ASYNC);

				assert( fd->fd_Action != NULL );

				if((*fd->fd_Action)(fd,&fam) < 0)
				{
					__set_errno(fam.fam_Error);

					goto out;
				}

				if(FLAG_IS_SET(flags,O_ASYNC))
					SET_FLAG(fd->fd_Flags,FDF_ASYNC_IO);
				else
					CLEAR_FLAG(fd->fd_Flags,FDF_ASYNC_IO);
			}

			result = 0;

			break;

		case F_DUPFD:

			SHOWMSG("cmd=F_DUPFD");

			va_start(arg, cmd);
			fdbase = va_arg(arg, int);
			va_end(arg);

			if(fdbase < 0)
			{
				__set_errno(EINVAL);
				goto out;
			}

			/* Check if we have that many fd's already */
			while(fdbase >= __num_fd)
			{
				if(__check_abort_enabled)
					__check_abort();

				/* No; enlarge it */
				if(__grow_fd_table() < 0)
					goto out;
			}

			vacant_slot = -1;

			/* Guaranteed to have enough here */
			do
			{
				if(__check_abort_enabled)
					__check_abort();

				for(i = fdbase ; i < __num_fd ; i++)
				{
					if(FLAG_IS_CLEAR(__fd[i]->fd_Flags,FDF_IN_USE))
					{
						vacant_slot = i;
						break;
					}
				}

				/* Didn't really find any, grow the table further */
				if(vacant_slot < 0 && __grow_fd_table() < 0)
					goto out;
			}
			while(vacant_slot < 0);

			/* Got a file descriptor, duplicate it */
			__duplicate_fd(__fd[vacant_slot],fd);

			result = vacant_slot;

			break;

		default:

			SHOWMSG("something else");

			__set_errno(ENOSYS);
			break;
	}

 out:

	RETURN(result);
	return(result);
}
