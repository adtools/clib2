/*
 * $Id: socket_accept.c,v 1.16 2006-01-08 12:04:24 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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
accept(int sockfd,struct sockaddr *cliaddr,socklen_t *addrlen)
{
	struct SignalSemaphore * lock = NULL;
	struct fd * fd = NULL;
	struct fd * new_fd;
	int new_fd_slot_number;
	int result = ERROR;
	LONG socket_fd;
	LONG new_socket_fd = -1;
	BOOL stdio_locked = FALSE;

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

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( sockfd >= 0 && sockfd < __num_fd );
	assert( __fd[sockfd] != NULL );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IN_USE) );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IS_SOCKET) );

	/* We need to know which parameter to submit to the accept()
	   call first. */
	__stdio_lock();
	stdio_locked = TRUE;

	fd = __get_file_descriptor_socket(sockfd);
	if(fd == NULL)
		goto out;

	/* Remember the socket number for later. */
	socket_fd = (LONG)fd->fd_DefaultFile;

	/* Now let go of the stdio lock, so that the only locking performed
	   will be done inside the accept() call. */
	__stdio_unlock();
	stdio_locked = FALSE;

	/* Wait for the accept() to complete, then hook up the socket
	   with a file descriptor. */
	PROFILE_OFF();
	new_socket_fd = __accept(socket_fd,cliaddr,(LONG *)addrlen);
	PROFILE_ON();

	if(new_socket_fd < 0)
	{
		SHOWMSG("could not accept connection");
		goto out;
	}

	/* OK, back to work: we'll need to manipulate the file
	   descriptor tables. */
	__stdio_lock();
	stdio_locked = TRUE;

	new_fd_slot_number = __find_vacant_fd_entry();
	if(new_fd_slot_number < 0)
	{
		if(__grow_fd_table(0) < 0)
		{
			SHOWMSG("couldn't find a vacant fd slot and no memory to create one");
			goto out;
		}

		new_fd_slot_number = __find_vacant_fd_entry();
		assert( new_fd_slot_number >= 0 );
	}

	#if defined(__THREAD_SAFE)
	{
		lock = __create_semaphore();
		if(lock == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}
	}
	#endif /* __THREAD_SAFE */

	new_fd = __fd[new_fd_slot_number];

	__initialize_fd(new_fd,__socket_hook_entry,(BPTR)new_socket_fd,FDF_IN_USE | FDF_IS_SOCKET | FDF_READ | FDF_WRITE,lock);

	lock = NULL;

	result = new_fd_slot_number;

	new_socket_fd = -1;

 out:

	if(new_socket_fd != -1)
	{
		PROFILE_OFF();

		__CloseSocket(new_socket_fd);

		PROFILE_ON();
	}

	if(stdio_locked)
		__stdio_unlock();

	#if defined(__THREAD_SAFE)
	{
		__delete_semaphore(lock);
	}
	#endif /* __THREAD_SAFE */

	if(__check_abort_enabled)
		__check_abort();

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
