/*
 * $Id: unistd_ftruncate.c,v 1.3 2005-01-02 09:07:19 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
ftruncate(int file_descriptor, off_t length)
{
	DECLARE_UTILITYBASE();
	struct file_hook_message message;
	int result = -1;
	struct fd * fd;
	long int position;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(length);

	assert( UtilityBase != NULL );

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		errno = EBADF;
		goto out;
	}

	if(length < 0)
	{
		SHOWMSG("invalid length");

		errno = EINVAL;
		goto out;
	}

	assert( FLAG_IS_SET(fd->fd_Flags,FDF_IN_USE) );

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_WRITE))
	{
		SHOWMSG("file descriptor is not write-enabled");

		errno = EBADF;
		goto out;
	}

	/* Remember where we started. */
	position = lseek(file_descriptor,0,SEEK_CUR);
	if(position < 0)
		goto out;

	SHOWMSG("calling the hook");

	message.action	= file_hook_action_truncate;
	message.size	= length;

	assert( fd->fd_Hook != NULL );

	CallHookPkt(fd->fd_Hook,fd,&message);

	if(message.result < 0)
	{
		errno = message.error;
		goto out;
	}

	/* Return to the original file position. */
	if(lseek(file_descriptor,position,SEEK_SET) < 0)
		goto out;

	result = 0;

 out:

	RETURN(result);
	return(result);
}
