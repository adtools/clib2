/*
 * $Id: socket_accept.c,v 1.1.1.1 2004-07-26 16:31:08 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

int
accept(int sockfd,struct sockaddr *cliaddr,int *addrlen)
{
	struct fd * fd;
	struct fd * new_fd;
	int new_fd_slot_number;
	int result = -1;
	LONG new_socket_fd;

	ENTER();

	SHOWVALUE(sockfd);
	SHOWPOINTER(cliaddr);
	SHOWPOINTER(addrlen);

	assert( cliaddr != NULL && addrlen != NULL );
	assert(__SocketBase != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(cliaddr == NULL || addrlen == NULL)
		{
			SHOWMSG("invalid parameters");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( sockfd >= 0 && sockfd < __num_fd );
	assert( __fd[sockfd] != NULL );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IN_USE) );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IS_SOCKET) );

	fd = __get_socket_descriptor(sockfd);
	if(fd == NULL)
		goto out;

	new_fd_slot_number = __find_vacant_fd_entry();
	if(new_fd_slot_number < 0)
	{
		if(__grow_fd_table() < 0)
		{
			SHOWMSG("couldn't find a vacant fd slot and no memory to create one");
			goto out;
		}

		new_fd_slot_number = __find_vacant_fd_entry();
		assert( new_fd_slot_number >= 0 );
	}

	PROFILE_OFF();
	new_socket_fd = __accept((LONG)fd->fd_DefaultFile,cliaddr,(LONG *)addrlen);
	PROFILE_ON();

	if(new_socket_fd < 0)
	{
		SHOWMSG("could not accept connection");
		goto out;
	}

	new_fd = __fd[new_fd_slot_number];

	__initialize_fd(new_fd,(HOOKFUNC)__socket_hook_entry,(BPTR)new_socket_fd,FDF_IN_USE | FDF_IS_SOCKET | FDF_READ | FDF_WRITE);

	result = new_fd_slot_number;

 out:

	if(__check_abort_enabled)
		__check_abort();

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */