/*
 * $Id: unistd_dup2.c,v 1.3 2005-01-02 09:07:19 obarthel Exp $
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
dup2(int file_descriptor1, int file_descriptor2)
{
	DECLARE_UTILITYBASE();
	struct fd * fd1;
	int result = -1;

	ENTER();

	SHOWVALUE(file_descriptor1);
	SHOWVALUE(file_descriptor2);

	assert( UtilityBase != NULL );

	if(__check_abort_enabled)
		__check_abort();

	assert( file_descriptor1 >= 0 && file_descriptor1 < __num_fd );
	assert( __fd[file_descriptor1] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor1]->fd_Flags,FDF_IN_USE) );

	fd1 = __get_file_descriptor(file_descriptor1);
	if(fd1 == NULL)
	{
		errno = EBADF;
		goto out;
	}

	if(file_descriptor2 < 0)
	{
		/* Try to find a place to put the duplicate into. */
		file_descriptor2 = __find_vacant_fd_entry();
		if(file_descriptor2 < 0)
		{
			/* No free space, so let's grow the table. */
			if(__grow_fd_table() < 0)
			{
				SHOWMSG("not enough memory for new file descriptor");
				goto out;
			}

			file_descriptor2 = __find_vacant_fd_entry();
			assert( file_descriptor2 >= 0 );
		}
	}
	else if (file_descriptor1 != file_descriptor2)
	{
		/* Make sure the requested duplicate exists. */
		while(file_descriptor2 >= __num_fd)
		{
			if(__check_abort_enabled)
				__check_abort();

			if(__grow_fd_table() < 0)
				goto out;
		}

		assert( file_descriptor2 >= 0 && file_descriptor2 < __num_fd );
		assert( __fd[file_descriptor2] != NULL );
	}

	if(file_descriptor1 != file_descriptor2)
	{
		struct file_hook_message message;
		struct fd * fd2;

		/* Have a look at the requested file descriptor. */
		assert( 0 <= file_descriptor2 && file_descriptor2 < __num_fd );

		fd2 = __fd[file_descriptor2];

		assert( fd2 != NULL );

		/* Make sure that the entry is cleaned up before we used it. */
		if(FLAG_IS_SET(fd2->fd_Flags,FDF_IN_USE))
		{
			SHOWMSG("closing file descriptor #2");

			if(close(file_descriptor2) < 0)
				goto out;
		}

		SHOWMSG("calling the hook");

		message.action			= file_hook_action_duplicate_fd;
		message.duplicate_fd	= fd2;

		assert( fd1->fd_Hook != NULL );

		CallHookPkt(fd1->fd_Hook,fd1,&message);

		result = message.result;
		if(result != 0)
		{
			errno = message.error;
			goto out;
		}
	}

	result = file_descriptor2;

 out:

	RETURN(result);
	return(result);
}
