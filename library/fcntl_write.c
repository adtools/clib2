/*
 * $Id: fcntl_write.c,v 1.1.1.1 2004-07-26 16:30:34 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

ssize_t
__write(int file_descriptor, const void * buffer, size_t num_bytes, int * error_ptr)
{
	DECLARE_UTILITYBASE();
	struct fd * fd;
	off_t result = -1;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWPOINTER(buffer);
	SHOWVALUE(num_bytes);

	assert( error_ptr != NULL );
	assert( buffer != NULL );
	assert( (int)num_bytes >= 0 );
	assert( UtilityBase != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(buffer == NULL)
		{
			SHOWMSG("invalid buffer address");

			(*error_ptr) = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

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

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_WRITE))
	{
		SHOWMSG("file descriptor is not write-enabled");

		(*error_ptr) = EBADF;
		goto out;
	}

	if(num_bytes > 0)
	{
		struct file_hook_message message;

		SHOWMSG("calling the hook");

		message.action	= file_hook_action_write;
		message.data	= (void *)buffer;
		message.size	= num_bytes;

		assert( fd->fd_Hook != NULL );

		CallHookPkt(fd->fd_Hook,fd,&message);

		(*error_ptr) = message.error;

		result = message.result;
	}
	else
	{
		(*error_ptr) = 0;

		result = 0;
	}

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

ssize_t
write(int file_descriptor, const void * buffer, size_t num_bytes)
{
	ssize_t result;

	ENTER();

	result = __write(file_descriptor,buffer,num_bytes,&errno);

	RETURN(result);
	return(result);
}
