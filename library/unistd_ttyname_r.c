/*
 * $Id: unistd_ttyname_r.c,v 1.1 2005-06-04 10:46:21 obarthel Exp $
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
	struct fd *fd;
	int result;

	ENTER();

	SHOWVALUE(file_descriptor);

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		result = EBADF;
		goto out;
	}

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_INTERACTIVE))
	{
		result = ENOTTY;
		goto out;
	}

	if(buflen < _POSIX_TTY_NAME_MAX) /* XXX Should this be _POSIX_PATH_MAX? */
	{
		result = ERANGE;
		goto out;
	}

	strcpy(name,"CONSOLE:"); /* The buffer is at least 9 bytes, so this is ok. */

	result = OK;

 out:

	RETURN(result);
	return(result);
}
