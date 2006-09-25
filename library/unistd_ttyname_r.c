/*
 * $Id: unistd_ttyname_r.c,v 1.4 2006-09-25 14:05:31 obarthel Exp $
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
	BOOL is_tty = FALSE;
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

	#if defined(__THREAD_SAFE)
	{
		if(FLAG_IS_SET(fd->fd_Flags,FDF_STDIO))
		{
			BPTR file;

			switch(fd->fd_DefaultFile)
			{
				case STDIN_FILENO:

					file = Input();
					break;

				case STDOUT_FILENO:

					file = Output();
					break;

				case STDERR_FILENO:

					#if defined(__amigaos4__)
					{
						file = ErrorOutput();
					}
					#else
					{
						struct Process * this_process = (struct Process *)FindTask(NULL);

						file = this_process->pr_CES;
					}
					#endif /* __amigaos4__ */

					/* The following is rather controversial; if the standard error stream
					   is unavailable, we default to reuse the standard output stream. This
					   is problematic if the standard output stream was redirected and should
					   not be the same as the standard error output stream. */
					if(file == ZERO)
						file = Output();

					break;

				default:

					file = ZERO;
					break;
			}

			__fd_lock(fd);

			if(file != ZERO && IsInteractive(file))
				is_tty = TRUE;

			__fd_unlock(fd);
		}
		else
		{
			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
				is_tty = TRUE;
		}
	}
	#else
	{
		if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
			is_tty = TRUE;
	}
	#endif /* __THREAD_SAFE */

	if(NOT is_tty)
	{
		result = ENOTTY;
		goto out;
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
