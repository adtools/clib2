/*
 * $Id: stdio_growiobtable.c,v 1.4 2005-03-04 09:07:09 obarthel Exp $
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

/****************************************************************************/

int
__grow_iob_table(int max_iob)
{
	const int granularity = 10;
	int new_num_iob;
	int result = -1;

	if(max_iob == 0)
		new_num_iob = __num_iob + granularity;
	else
		new_num_iob = max_iob;

	if(new_num_iob > __num_iob)
	{
		struct iob ** new_iob;
		int i;

		new_iob = malloc(sizeof(*new_iob) * new_num_iob);
		if(new_iob == NULL)
		{
			SHOWMSG("not enough memory for file table");

			__set_errno(ENOMEM);
			goto out;
		}

		for(i = __num_iob ; i < new_num_iob ; i++)
		{
			new_iob[i] = malloc(sizeof(*new_iob[i]));
			if(new_iob[i] == NULL)
			{
				int j;

				SHOWMSG("not enough memory for file table entry");

				for(j = __num_iob ; j < i ; j++)
					free(new_iob[j]);

				free(new_iob);

				__set_errno(ENOMEM);
				goto out;
			}

			memset(new_iob[i],0,sizeof(*new_iob[i]));
		}

		if(__iob != NULL)
		{
			for(i = 0 ; i < __num_iob ; i++)
				new_iob[i] = __iob[i];

			free(__iob);
		}

		__iob		= new_iob;
		__num_iob	= new_num_iob;
	}

	result = 0;

 out:

	return(result);
}
