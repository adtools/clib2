/*
 * $Id: socket_socket.c,v 1.9 2005-04-24 08:46:37 obarthel Exp $
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

int
socket(int domain,int type,int protocol)
{
	struct SignalSemaphore * lock = NULL;
	int result = ERROR;
	struct fd * fd;
	int fd_slot_number;
	LONG socket_fd;

	ENTER();

	SHOWVALUE(domain);
	SHOWVALUE(type);
	SHOWVALUE(protocol);

	__stdio_lock();

	fd_slot_number = __find_vacant_fd_entry();
	if(fd_slot_number < 0)
	{
		if(__grow_fd_table(0) < 0)
		{
			SHOWMSG("couldn't find a vacant fd slot and no memory to create one");
			goto out;
		}

		fd_slot_number = __find_vacant_fd_entry();
		assert( fd_slot_number >= 0 );
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

	PROFILE_OFF();
	socket_fd = __socket(domain,type,protocol);
	PROFILE_ON();

	if(socket_fd < 0)
	{
		SHOWMSG("could not create socket");
		goto out;
	}

	fd = __fd[fd_slot_number];

	__initialize_fd(fd,__socket_hook_entry,(BPTR)socket_fd,FDF_IN_USE | FDF_IS_SOCKET | FDF_READ | FDF_WRITE,lock);

	lock = NULL;

	result = fd_slot_number;

 out:

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
