/*
 * $Id: stdio_growfdtable.c,v 1.1.1.1 2004-07-26 16:31:34 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
__grow_fd_table(void)
{
	const int granularity = 10;
	struct fd ** new_fd;
	int new_num_fd;
	int result = -1;
	int i;

	new_num_fd = __num_fd + granularity;

	new_fd = malloc(sizeof(*new_fd) * new_num_fd);
	if(new_fd == NULL)
	{
		SHOWMSG("not enough memory for new file descriptor table");

		errno = ENOMEM;
		goto out;
	}

	for(i = __num_fd ; i < new_num_fd ; i++)
	{
		new_fd[i] = malloc(sizeof(*new_fd[i]));
		if(new_fd[i] == NULL)
		{
			int j;

			SHOWMSG("not enough memory for new file descriptor table entry");

			for(j = __num_fd ; j < i ; j++)
				free(new_fd[j]);

			free(new_fd);

			errno = ENOMEM;
			goto out;
		}

		memset(new_fd[i],0,sizeof(*new_fd[i]));
	}

	for(i = 0 ; i < __num_fd ; i++)
		new_fd[i] = __fd[i];

	free(__fd);

	__fd		= new_fd;
	__num_fd	= new_num_fd;

	result = 0;

 out:

	return(result);
}