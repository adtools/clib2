/*
 * $Id: stdio_duplicate_fd.c,v 1.2 2005-01-02 09:07:08 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

void
__duplicate_fd(struct fd * duplicate_fd,struct fd * original_fd)
{
	HOOKFUNC hook_function;

	assert( duplicate_fd != NULL && original_fd != NULL );

	/* Obtain the hook function associated with the file; note that
	   this is different depending upon which operating system
	   release is being used (compare "stdio_initializefd.c"). */
	#if defined(__amigaos4__)
	{
		hook_function = original_fd->fd_Hook->h_Entry;
	}
	#else
	{
		hook_function = original_fd->fd_Hook->h_SubEntry;
	}
	#endif /* __amigaos4__ */

	/* Initialize the duplicate to match the original. */
	__initialize_fd(duplicate_fd,hook_function,original_fd->fd_DefaultFile,original_fd->fd_Flags);

	/* Figure out where the linked list of file descriptors associated
	   with this one starts. */
	if(original_fd->fd_Original != NULL)
		duplicate_fd->fd_Original = original_fd->fd_Original;
	else
		duplicate_fd->fd_Original = original_fd;

	/* Add the duplicate at the beginning of the list. */
	duplicate_fd->fd_NextLink				= duplicate_fd->fd_Original->fd_NextLink;
	duplicate_fd->fd_Original->fd_NextLink	= duplicate_fd;
}
