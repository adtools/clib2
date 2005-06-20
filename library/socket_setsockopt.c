/*
 * $Id: socket_setsockopt.c,v 1.8 2005-06-20 07:43:59 obarthel Exp $
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
setsockopt(int sockfd,int level,int optname,const void *optval,socklen_t optlen)
{
	struct fd * fd;
	int result = ERROR;

	ENTER();

	SHOWVALUE(sockfd);
	SHOWVALUE(level);
	SHOWVALUE(optname);
	SHOWPOINTER(optval);
	SHOWVALUE(optlen);

	assert( optval != NULL );
	assert(__SocketBase != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(optval == NULL)
		{
			SHOWMSG("invalid optval parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( sockfd >= 0 && sockfd < __num_fd );
	assert( __fd[sockfd] != NULL );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IN_USE) );
	assert( FLAG_IS_SET(__fd[sockfd]->fd_Flags,FDF_IS_SOCKET) );

	fd = __get_file_descriptor_socket(sockfd);
	if(fd == NULL)
		goto out;

	PROFILE_OFF();
	result = __setsockopt((LONG)fd->fd_DefaultFile,level,optname,(void *)optval,optlen);
	PROFILE_ON();

 out:

	if(__check_abort_enabled)
		__check_abort();

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
