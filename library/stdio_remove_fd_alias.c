/*
 * $Id: stdio_remove_fd_alias.c,v 1.2 2005-02-28 13:22:53 obarthel Exp $
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
__remove_fd_alias(struct fd * fd)
{
	assert( fd != NULL );

	__stdio_lock();

	if(fd->fd_Original != NULL) /* this is an alias */
	{
		struct fd * list_fd;

		assert( fd->fd_Original != fd );
		assert( fd->fd_Original->fd_Original == NULL );

		/* Remove this alias from the list. */
		for(list_fd = fd->fd_Original ;
		    list_fd != NULL ;
		    list_fd = list_fd->fd_NextLink)
		{
			if(list_fd->fd_NextLink == fd)
			{
				list_fd->fd_NextLink = fd->fd_NextLink;
				break;
			}
		}
	}
	else if (fd->fd_NextLink != NULL) /* this one has aliases attached; it is the 'original' resource */
	{
		struct fd * first_alias;
		struct fd * list_fd;

		/* The first link now becomes the original resource */
		first_alias = fd->fd_NextLink;
		first_alias->fd_Original = NULL;

		/* The resources are migrated to the first link. */
		for(list_fd = first_alias->fd_NextLink ;
		    list_fd != NULL ;
		    list_fd = list_fd->fd_NextLink)
		{
			list_fd->fd_Original = first_alias;
		}
	}

	__stdio_unlock();
}
