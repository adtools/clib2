/*
 * $Id: socket_ioctl.c,v 1.13 2006-09-25 15:38:21 obarthel Exp $
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

#ifndef _STDARG_H
#include <stdarg.h>
#endif /* _STDARG_H */

/****************************************************************************/

#include <sys/ioctl.h>

/****************************************************************************/

int
ioctl(int sockfd,int request, ... /* char *arg */)
{
	va_list arg;
	char * param;
	struct fd * fd = NULL;
	int result = ERROR;

	ENTER();

	SHOWVALUE(sockfd);
	SHOWVALUE(request);

	assert(__SocketBase != NULL);

	assert( sockfd >= 0 && sockfd < __num_fd );
	assert( __fd[sockfd] != NULL );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IN_USE) );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IS_SOCKET) );

	fd = __get_file_descriptor_socket(sockfd);
	if(fd == NULL)
		goto out;

	__fd_lock(fd);

	va_start(arg,request);
	param = va_arg(arg,char *);
	va_end(arg);

	SHOWPOINTER(param);

	PROFILE_OFF();
	result = __IoctlSocket((LONG)fd->fd_DefaultFile,request,param);
	PROFILE_ON();

	if(result == 0)
	{
		const int * option = (const int *)param;

		if(request == (int)FIONBIO)
		{
			if((*option) != 0)
				SET_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
			else
				CLEAR_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
		}
		else if (request == (int)FIOASYNC)
		{
			if((*option) != 0)
				SET_FLAG(fd->fd_Flags,FDF_ASYNC_IO);
			else
				CLEAR_FLAG(fd->fd_Flags,FDF_ASYNC_IO);
		}
	}

 out:

	__fd_unlock(fd);

	if(__check_abort_enabled)
		__check_abort();

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
