/*
 * $Id: stdio_init_exit.c,v 1.26 2005-03-11 09:37:29 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The file handle table. */
struct iob **	__iob;
int				__num_iob;

/****************************************************************************/

/* The file descriptor table. */
struct fd **	__fd;
int				__num_fd;

/****************************************************************************/

void
__close_all_files(void)
{
	int i;

	ENTER();

	__check_abort_enabled = FALSE;

	__stdio_lock();

	if(__num_iob > 0)
	{
		for(i = 0 ; i < __num_iob ; i++)
		{
			if(FLAG_IS_SET(__iob[i]->iob_Flags,IOBF_IN_USE))
				fclose((FILE *)__iob[i]);
		}

		__num_iob = 0;
	}

	if(__num_fd > 0)
	{
		for(i = 0 ; i < __num_fd ; i++)
		{
			if(FLAG_IS_SET(__fd[i]->fd_Flags,FDF_IN_USE))
				close(i);
		}

		__num_fd = 0;
	}

	__stdio_unlock();

	LEAVE();
}

/****************************************************************************/

CLIB_DESTRUCTOR(__stdio_exit)
{
	ENTER();

	__close_all_files();

	__stdio_lock_exit();

	LEAVE();
}

/****************************************************************************/

int
__stdio_init(void)
{
	const int num_standard_files = (STDERR_FILENO - STDIN_FILENO + 1);

	int result = ERROR;

	ENTER();

	if(__stdio_lock_init() < 0)
		goto out;

	if(__grow_iob_table(num_standard_files) < 0)
		goto out;

	if(__grow_fd_table(num_standard_files) < 0)
		goto out;

	result = OK;

 out:

	RETURN(result);
	return(result);
}
