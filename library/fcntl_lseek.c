/*
 * $Id: fcntl_lseek.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

off_t
__lseek(int file_descriptor, off_t offset, int mode, int * error_ptr)
{
	DECLARE_UTILITYBASE();
	struct file_hook_message message;
	off_t result = -1;
	struct fd * fd;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(offset);
	SHOWVALUE(mode);

	assert( UtilityBase != NULL );

	assert( error_ptr != NULL );
	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		(*error_ptr) = EBADF;
		goto out;
	}

	if(mode < SEEK_SET || mode > SEEK_END)
	{
		SHOWMSG("seek mode is invalid");

		(*error_ptr) = EINVAL;
		goto out;
	}

	SHOWMSG("calling the hook");

	#if defined(UNIX_PATH_SEMANTICS)
	{
		message.action = file_hook_action_seek_and_extend;
	}
	#else
	{
		message.action = file_hook_action_seek;
	}
	#endif /* UNIX_PATH_SEMANTICS */

	message.position	= offset;
	message.mode		= mode;

	assert( fd->fd_Hook != NULL );

	CallHookPkt(fd->fd_Hook,fd,&message);

	(*error_ptr) = message.error;

	result = message.result;

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

off_t
lseek(int file_descriptor, off_t offset, int mode)
{
	off_t result;

	ENTER();

	result = __lseek(file_descriptor,offset,mode,&errno);

	RETURN(result);
	return(result);
}
