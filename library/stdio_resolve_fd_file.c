/*
 * $Id: stdio_resolve_fd_file.c,v 1.1 2006-11-16 14:39:23 obarthel Exp $
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

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

#ifdef __resolve_fd_file
#undef __resolve_fd_file
#endif /* __resolve_fd_file */

/****************************************************************************/

BPTR
__resolve_fd_file(struct fd * fd)
{
	BPTR file;

	/* Is this one the standard I/O streams for which the associated file
	   handle should be determined dynamically? */
	if(FLAG_IS_SET(fd->fd_Flags,FDF_STDIO))
	{
		switch(fd->fd_File)
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
	}
	else
	{
		/* Just return what's there. */
		file = fd->fd_File;
	}

	return(file);
}

/****************************************************************************/

#endif /* __THREAD_SAFE */
