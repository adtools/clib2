/*
 * $Id: stdlib_bsearch.c,v 1.4 2006-01-08 12:04:25 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

void *
bsearch(const void *key, const void *base, size_t count, size_t size, int (*compare)(const void * key,const void * value))
{
	void * result = NULL;

	ENTER();

	SHOWPOINTER(key);
	SHOWPOINTER(base);
	SHOWVALUE(count);
	SHOWVALUE(size);
	SHOWPOINTER(compare);

	if(count > 0 && size > 0)
	{
		void * current;
		size_t lower = 0;
		size_t upper = count;
		size_t position;
		int delta;

		assert( key != NULL && base != NULL && compare != NULL );

		#if defined(CHECK_FOR_NULL_POINTERS)
		{
			if(key == NULL || base == NULL || compare == NULL)
			{
				SHOWMSG("invalid parameters");

				__set_errno(EFAULT);
				goto out;
			}
		}
		#endif /* CHECK_FOR_NULL_POINTERS */

		while(lower < upper)
		{
			position = (lower + upper) / 2;

			current = (void *)(((ULONG)base) + (position * size));

			delta = (*compare)(key, current);
			if(delta == 0)
			{
				result = current;
				break;
			}

			if(delta < 0)
				upper = position;
			else
				lower = position + 1;
		}
	}

 out:

	RETURN(result);
	return(result);
}
