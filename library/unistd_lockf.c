/*
 * $Id: 
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
lockf(int file_descriptor,int function,off_t size)
{
	struct flock l;
	int result = -1;
	int cmd;

	l.l_whence	= SEEK_CUR;
	l.l_start	= 0;
	l.l_len		= size;

	switch(function)
	{
		case F_ULOCK:

			cmd = F_SETLK;
			l.l_type = F_UNLCK;
			break;

		case F_LOCK:

			cmd = F_SETLKW;
			l.l_type = F_WRLCK;
			break;

		case F_TLOCK:

			cmd = F_SETLK;
			l.l_type = F_WRLCK;
			break;

		case F_TEST:

			l.l_type = F_WRLCK;

			if(fcntl(file_descriptor,F_GETLK,&l) == -1)
				goto out;

			if(l.l_pid == getpid() || l.l_type == F_UNLCK)
			{
				result = 0;
				goto out;
			}

			__set_errno(EAGAIN);
			goto out;

		default:

			__set_errno(EINVAL);
			goto out;
	}

	result = fcntl(file_descriptor,cmd,&l);

 out:

	return(result);
}
