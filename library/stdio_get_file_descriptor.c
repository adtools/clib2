/*
 * $Id: stdio_get_file_descriptor.c,v 1.4 2005-04-01 18:46:37 obarthel Exp $
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

enum resolution_mode_t
{
	resolution_mode_exact,	/* Return the descriptor exactly as it is. */
	resolution_mode_alias	/* Resolve the descriptor's alias, if necessary. */
};

/****************************************************************************/

STATIC struct fd *
get_file_descriptor(int file_descriptor,enum resolution_mode_t resolution_mode)
{
	struct fd * result = NULL;
	struct fd * fd;

	__stdio_lock();

	if(file_descriptor < 0 || file_descriptor >= __num_fd)
	{
		SHOWMSG("invalid file descriptor");
		goto out;
	}

	fd = __fd[file_descriptor];
	if(fd == NULL)
	{
		SHOWMSG("invalid file descriptor");
		goto out;
	}

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_IN_USE))
	{
		SHOWMSG("this descriptor is not even in use");
		goto out;
	}

	/* Move up to the original file, if this is an alias. */
	if(resolution_mode == resolution_mode_alias && fd->fd_Original != NULL)
		fd = fd->fd_Original;

	result = fd;

 out:

	__stdio_unlock();

	return(result);
}

/****************************************************************************/

struct fd *
__get_file_descriptor(int file_descriptor)
{
	struct fd * result;

	result = get_file_descriptor(file_descriptor,resolution_mode_alias);

	return(result);
}

/****************************************************************************/

struct fd *
__get_file_descriptor_dont_resolve(int file_descriptor)
{
	struct fd * result;

	result = get_file_descriptor(file_descriptor,resolution_mode_exact);

	return(result);
}
