/*
 * $Id: unistd_ttyname_r.c,v 1.6 2006-11-16 14:39:23 obarthel Exp $
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

#ifndef	_UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/*
 * Just a quick kludge, really.
 */

int
ttyname_r(int file_descriptor,char *name,size_t buflen)
{
	const char *tty_file_name;
	struct fd *fd;
	int result;

	ENTER();

	SHOWVALUE(file_descriptor);

	__stdio_lock();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		result = EBADF;
		goto out;
	}

	__fd_lock(fd);

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_INTERACTIVE))
	{
		BPTR file;

		file = __resolve_fd_file(fd);
		if(file == ZERO || NOT IsInteractive(file))
		{
			result = ENOTTY;
			goto out;
		}
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		tty_file_name = "/CONSOLE";
	}
	#else
	{
		tty_file_name = "CONSOLE:";
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if(buflen < strlen(tty_file_name)+1) /* XXX Should this be _POSIX_PATH_MAX? */
	{
		result = ERANGE;
		goto out;
	}

	strcpy(name,tty_file_name);

	result = OK;

 out:

	__fd_unlock(fd);

	__stdio_unlock();

	RETURN(result);
	return(result);
}
