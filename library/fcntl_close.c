/*
 * $Id: fcntl_close.c,v 1.8 2005-02-04 08:49:10 obarthel Exp $
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
__close(int file_descriptor,int * error_ptr)
{
	DECLARE_UTILITYBASE();

	struct file_hook_message message;
	struct fd * fd;
	int result = -1;
	BOOL no_close;
	BOOL is_alias;

	ENTER();

	SHOWVALUE(file_descriptor);

	assert( UtilityBase != NULL );

	assert( error_ptr != NULL );
	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		(*error_ptr) = EBADF;
		goto out;
	}

	result = 0;

	SHOWMSG("last customer; cleaning up");

	if(fd->fd_Original != NULL) /* this is an alias */
	{
		struct fd * list_fd;

		SHOWMSG("taking out the alias");

		assert( fd->fd_Original != fd );
		assert( fd->fd_Original->fd_Original == NULL );

		/* Remove this alias from the list. */
		for(list_fd = fd->fd_Original ;
		    list_fd != NULL ;
		    list_fd = list_fd->fd_NextLink)
		{
			if(list_fd->fd_NextLink == fd)
			{
				list_fd->fd_NextLink = fd->fd_NextLink;
				break;
			}
		}

		no_close = TRUE;
		is_alias = TRUE;
	}
	else if (fd->fd_NextLink != NULL) /* this one has aliases attached; it is the 'original' resource */
	{
		struct fd * first_alias;
		struct fd * list_fd;

		SHOWMSG("closing original descriptor; migrating it to first alias");

		/* The first link now becomes the original resource */
		first_alias = fd->fd_NextLink;
		first_alias->fd_Original = NULL;

		/* The resources are migrated to the first link. */
		for(list_fd = first_alias->fd_NextLink ;
		    list_fd != NULL ;
		    list_fd = list_fd->fd_NextLink)
		{
			list_fd->fd_Original = first_alias;
		}

		no_close = TRUE;
		is_alias = TRUE;
	}
	else
	{
		no_close = FLAG_IS_SET(fd->fd_Flags,FDF_NO_CLOSE);
		is_alias = FALSE;
	}

	/* Reset the console to regular buffered/unbuffered input. We don't do this
	   for aliases and their like since the original stream is still in use. */
	if(NOT is_alias)
	{
		if(FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING))
		{
			SHOWMSG("resetting non-blocking access mode");

			message.action	= file_hook_action_set_blocking;
			message.arg		= TRUE;

			assert( fd->fd_Hook != NULL );

			CallHookPkt(fd->fd_Hook,fd,&message);
		}
	}

	(*error_ptr) = OK;

	if(NOT no_close && NOT is_alias)
	{
		SHOWMSG("shutting down");

		message.action = file_hook_action_close;

		assert( fd->fd_Hook != NULL );

		CallHookPkt(fd->fd_Hook,fd,&message);

		result = message.result;

		(*error_ptr) = message.error;
	}

	memset(fd,0,sizeof(*fd));

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

int
close(int file_descriptor)
{
	int result;
	int error;

	ENTER();

	result = __close(file_descriptor,&error);
	__set_errno(error);

	RETURN(result);
	return(result);
}
